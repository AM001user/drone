import socket
import time
import subprocess

UDP_PORT = 12345
TCP_PORT = 80
DISCOVERY_REQUEST = "ESP32_DISCOVER"
DISCOVERY_REPLY_PREFIX = "ESP32_IP:"
message = "test message from PC"


def local_ip():
    try:
        out = subprocess.check_output("ipconfig", shell=True).decode('latin-1')
        if '192.168.137.' in out:
            for line in out.splitlines():
                if 'IPv4' in line and '192.168.137.' in line:
                    return line.split(':')[-1].strip()
    except Exception:
        pass
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('1.1.1.1', 80))
        return s.getsockname()[0]
    except Exception:
        return None
    finally:
        s.close()


def broadcast_for(ip):
    if not ip:
        return '255.255.255.255'
    p = ip.split('.')
    return '.'.join(p[:3] + ['255'])


def discover(timeout=5):
    ip = local_ip()
    bcast = broadcast_for(ip)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    s.settimeout(timeout)
    s.bind(('', UDP_PORT))
    targets = {bcast, '255.255.255.255'}
    for _ in range(3):
        for t in targets:
            s.sendto(DISCOVERY_REQUEST.encode(), (t, UDP_PORT))
        time.sleep(0.05)

    end = time.time() + timeout
    while time.time() < end:
        try:
            data, addr = s.recvfrom(1024)
        except socket.timeout:
            break
        p = data.decode().strip()
        if p == DISCOVERY_REQUEST:
            continue
        if p.startswith(DISCOVERY_REPLY_PREFIX):
            s.close()
            return p.split(':', 1)[1].strip()
    s.close()
    return None


def recv_line(sock):
    buf = b''
    while True:
        ch = sock.recv(1)
        if not ch:
            break
        buf += ch
        if ch == b'\n':
            break
    return buf.decode().strip()


def main():
    esp = discover(timeout=6)
    if not esp:
        print("Impossible de trouver l'ESP32. Vérifiez le réseau et le moniteur série.")
        raise SystemExit(1)
    print(f'ESP32 trouvé : {esp}. Connexion TCP...')

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.settimeout(10)
        s.connect((esp, TCP_PORT))

        print('Attente du handshake...')
        line = recv_line(s)
        print('Reçu:', line)
        if line != 'ESP32:READY':
            raise RuntimeError('Handshake inattendu')

        s.sendall(b'PC:HELLO\n')
        print('Envoyé: PC:HELLO')
        print('Reçu:', recv_line(s))

        for i in range(5):
            m = f'PC:MSG:{i+1}:{message}\n'
            print('Envoi:', m.strip())
            s.sendall(m.encode())
            print('Reçu:', recv_line(s))

        s.sendall(b'PC:BYE\n')
        print('Session terminée:', recv_line(s))

    finally:
        s.close()
if __name__ == '__main__':
    main()