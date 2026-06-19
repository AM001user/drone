import socket
import time

UDP_PORT = 12345
TCP_PORT = 80
DISCOVERY_REQUEST = "ESP32_DISCOVER"
DISCOVERY_REPLY_PREFIX = "ESP32_IP:"
message = "Bonjour ESP32, ici le PC en mode automatique !"

print("En attente du signal de l'ESP32 (recherche de l'IP)...")

udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
udp_socket.bind(("", UDP_PORT))

esp32_ip = None
start_time = time.time()
timeout = 15

while time.time() - start_time < timeout:
    udp_socket.settimeout(timeout - (time.time() - start_time))
    try:
        data, addr = udp_socket.recvfrom(1024)
        payload = data.decode("utf-8").strip()
        if payload.startswith(DISCOVERY_REPLY_PREFIX):
            esp32_ip = payload.split(":", 1)[1].strip()
            print(f"ESP32 détecté à l'adresse IP : {esp32_ip}")
            break
        else:
            print(f"Message UDP ignoré de {addr}: {payload}")
    except socket.timeout:
        break

udp_socket.close()

if not esp32_ip:
    print("Aucune réponse de découverte reçue. Envoi d'une requête de découverte en broadcast...")
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    udp_socket.settimeout(5)
    udp_socket.sendto(DISCOVERY_REQUEST.encode("utf-8"), ("255.255.255.255", UDP_PORT))

    try:
        data, addr = udp_socket.recvfrom(1024)
        payload = data.decode("utf-8").strip()
        if payload.startswith(DISCOVERY_REPLY_PREFIX):
            esp32_ip = payload.split(":", 1)[1].strip()
            print(f"ESP32 détecté à l'adresse IP : {esp32_ip}")
        else:
            print(f"Réponse UDP non reconnue : {payload}")
    except socket.timeout:
        print("Aucune réponse UDP après la demande de découverte.")
    finally:
        udp_socket.close()

if not esp32_ip:
    print("Impossible de trouver l'ESP32. Vérifiez que le PC et l'ESP32 sont sur le même réseau.")
    raise SystemExit(1)

print(f"Connexion TCP automatique à {esp32_ip}:{TCP_PORT}...")

try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.settimeout(10)
    client_socket.connect((esp32_ip, TCP_PORT))

    def recv_line(sock):
        data = b""
        while True:
            chunk = sock.recv(1)
            if not chunk:
                break
            data += chunk
            if chunk == b"\n":
                break
        return data.decode("utf-8").strip()

    # Handshake
    print("Attente du handshake de l'ESP32...")
    line = recv_line(client_socket)
    print(f"Reçu : {line}")
    if line != "ESP32:READY":
        raise RuntimeError(f"Handshake inattendu : {line}")

    client_socket.sendall(b"PC:HELLO\n")
    print("Envoyé : PC:HELLO")

    line = recv_line(client_socket)
    print(f"Reçu : {line}")
    if line != "ESP32:HELLO":
        raise RuntimeError(f"Handshake non confirmé : {line}")

    # Échange de messages multiples
    for i in range(5):
        msg = f"PC:MSG:{i + 1}:{message}\n"
        print(f"Envoi : {msg.strip()}")
        client_socket.sendall(msg.encode("utf-8"))

        line = recv_line(client_socket)
        print(f"Reçu : {line}")
        if not line.startswith(f"ESP32:ACK:{i + 1}"):
            raise RuntimeError(f"ACK inattendu : {line}")

    client_socket.sendall(b"PC:BYE\n")
    print("Envoyé : PC:BYE")

    line = recv_line(client_socket)
    print(f"Reçu : {line}")
    if line != "ESP32:BYE":
        raise RuntimeError(f"Fin de session inattendue : {line}")

    print("Session terminée avec succès.")

except Exception as e:
    print(f"Une erreur est survenue : {e}")
finally:
    client_socket.close()
    print("Connexion terminée.")