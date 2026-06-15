#!/usr/bin/env python3
"""
serial_map.py
Lis les valeurs renvoyées par le capteur HC-SR04 sur le port série et construit
une carte 2D simple (nuage de points) représentant la distance aux obstacles.

Fonctionnalités:
- Se connecte à un port série (ex: COM10) à 115200 bauds
- Lit des lignes envoyées par l'Arduino. Format attendu:
  - "<dist> cm" (ex: "123.4 cm")
  - ou "<angle>,<dist>" (ex: "45,123.4") si l'Arduino envoie l'angle
- Mode manuel: appuyer sur 'r' (record) puis saisir l'angle pour enregistrer
  la dernière mesure lue au port série.
- Commandes: r = record, p = plot et sauvegarde PNG, q = quit, h = help

Usage:
  python scripts/serial_map.py --port COM10 --baud 115200 --out map.png

Dépendances:
  pip install pyserial matplotlib numpy

"""
import argparse
import threading
import time
import re
import sys
from collections import deque

try:
    import serial  # type: ignore
except ImportError:
    print("Module pyserial requis. Installez avec: pip install pyserial")
    raise

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt  # type: ignore
import numpy as np  # type: ignore


def parse_distance_line(line):
    # Try formats: "123.4 cm" or "45,123.4" or just a number
    line = line.strip()
    if not line:
        return None, None
    # comma separated angle,dist
    if "," in line:
        parts = [p.strip() for p in line.split(",") if p.strip()]
        if len(parts) >= 2:
            try:
                angle = float(parts[0])
                dist = float(re.findall(r"[-+]?[0-9]*\.?[0-9]+", parts[1])[0])
                return angle, dist
            except Exception:
                return None, None
    # look for number in line
    m = re.findall(r"[-+]?[0-9]*\.?[0-9]+", line)
    if m:
        try:
            dist = float(m[0])
            # check if line contains 'cm' then no angle
            return None, dist
        except Exception:
            return None, None
    return None, None


class SerialReader(threading.Thread):
    def __init__(self, port, baud, buffer_size=10):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self._stop = threading.Event()
        self.last_distance = None
        self.last_angle = None
        self.last_update = 0.0
        self.lock = threading.Lock()
        self.lines = deque(maxlen=200)
        self.ser = None

    def run(self):
        try:
            with serial.Serial(self.port, self.baud, timeout=1) as ser:
                self.ser = ser
                print(f"Connected to {self.port} @ {self.baud} baud\n")
                while not self._stop.is_set():
                    try:
                        raw = ser.readline()
                        if not raw:
                            continue
                        try:
                            line = raw.decode('utf-8', errors='replace').strip()
                        except Exception:
                            line = str(raw)
                        if not line:
                            continue
                        ang, dist = parse_distance_line(line)
                        with self.lock:
                            if dist is not None:
                                self.last_distance = dist
                                self.last_angle = ang
                                self.last_update = time.time()
                            self.lines.append((line, ang, dist))
                        # Silently record, don't print to avoid output confusion
                    except Exception:
                        time.sleep(0.1)
        except serial.SerialException as e:
            print("Could not open serial port:", e)
            self._stop.set()

    def send_command(self, command: str) -> bool:
        if self.ser is None or not self.ser.is_open:
            return False
        try:
            with self.lock:
                self.ser.write(command.encode('utf-8'))
                self.ser.flush()
            return True
        except Exception:
            return False

    def wait_for_reading(self, expected_angle=None, timeout=2.0):
        deadline = time.time() + timeout
        with self.lock:
            initial_update = self.last_update
        while time.time() < deadline:
            with self.lock:
                if self.last_update > initial_update:
                    if expected_angle is None or self.last_angle == expected_angle:
                        return self.last_angle, self.last_distance
            time.sleep(0.05)
        return None, None

    def stop(self):
        self._stop.set()


def update_plot_realtime(fig, ax, angles_deg, dists):
    """Update the plot in real-time"""
    ax.clear()
    if angles_deg:
        angles = np.radians(angles_deg)
        xs = np.array(dists) * np.cos(angles)
        ys = np.array(dists) * np.sin(angles)
        ax.scatter(xs, ys, c='red', s=50, alpha=0.7)
    
    ax.axvline(0, color='k', linewidth=0.5)
    ax.axhline(0, color='k', linewidth=0.5)
    ax.set_aspect('equal', 'box')
    
    maxr = max(dists) if dists else 100
    ax.set_xlim(-maxr*1.2, maxr*1.2)
    ax.set_ylim(-maxr*1.2, maxr*1.2)
    ax.set_title(f'Carte 2D (points: {len(angles_deg)})')
    ax.set_xlabel('X (cm)')
    ax.set_ylabel('Y (cm)')
    ax.grid(True, alpha=0.3)
    
    fig.canvas.draw()
    plt.pause(0.001)


def plot_points(angles_deg, dists, out_file="map.png"):
    """Save the final plot to file"""
    angles = np.radians(angles_deg)
    xs = np.array(dists) * np.cos(angles)
    ys = np.array(dists) * np.sin(angles)

    plt.figure(figsize=(6,6))
    plt.scatter(xs, ys, c='red', s=20)
    plt.axvline(0, color='k', linewidth=0.5)
    plt.axhline(0, color='k', linewidth=0.5)
    plt.gca().set_aspect('equal', 'box')
    maxr = max(dists) if dists else 100
    plt.xlim(-maxr*1.1, maxr*1.1)
    plt.ylim(-maxr*1.1, maxr*1.1)
    plt.title('Carte 2D (points depuis HC-SR04)')
    plt.xlabel('X (cm)')
    plt.ylabel('Y (cm)')
    plt.grid(True)
    plt.savefig(out_file, dpi=150)
    plt.close()
    print(f'Saved map to {out_file}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', default='COM10', help='Serial port (ex: COM10)')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate')
    parser.add_argument('--out', default='map.png', help='Output PNG file')
    args = parser.parse_args()

    reader = SerialReader(args.port, args.baud)
    reader.start()

    angles = []
    dists = []
    recording = False

    time.sleep(0.5)  # Let serial connect first
    
    # Setup matplotlib in interactive mode and show the empty window immediately
    plt.ion()
    fig, ax = plt.subplots(figsize=(7, 7))
    fig.suptitle("SONAR Carte 2D - En attente de démarrage...", fontsize=14, fontweight='bold')
    update_plot_realtime(fig, ax, angles, dists)
    plt.show(block=False)
    
    print("=" * 60)
    print("SONAR 2D MAPPER - Mode interactif")
    print("=" * 60)
    print("Commands:")
    print("  start  : Commencer l'enregistrement (préparation...)")
    print("  record : Envoyer un angle au servo et enregistrer la distance")
    print("  stop   : Arrêter et sauvegarder la carte PNG")
    print("  save   : Sauvegarder sans arrêter")
    print("  clear  : Effacer tous les points")
    print("  h      : Aide")
    print("  q      : Quitter")
    print("=" * 60 + "\n")
    
    try:
        while True:
            if not recording:
                print("[STANDBY] Tapez 'start' pour commencer l'enregistrement")
                sys.stdout.write("> ")
                sys.stdout.flush()
                cmd = input().strip().lower()
                
                if cmd == 'q':
                    break
                if cmd == 'h':
                    print('\n--- HELP ---')
                    print('start  : Lance l\'enregistrement')
                    print('q      : Quitter')
                    continue
                if cmd == 'start':
                    recording = True
                    angles = []
                    dists = []
                    print("\n🔴 ENREGISTREMENT ACTIF - Préparez-vous!\n")
                    fig.suptitle("SONAR Carte 2D - EN DIRECT", fontsize=14, fontweight='bold')
                    fig.canvas.draw()
                    continue
                continue
            
            # Recording mode
            sys.stdout.write("> ")
            sys.stdout.flush()
            cmd = input().strip().lower()
            
            if cmd == 'h':
                print('\n--- HELP (Mode Enregistrement) ---')
                print('record   : Enregistrer point courant (demande l\'angle)')
                print('angle,distance : Ajouter directement (ex: 45,123.4)')
                print('stop     : Arrêter et sauvegarder PNG')
                print('save     : Sauvegarder PNG (continue)')
                print('clear    : Effacer tous les points')
                print('q        : Quitter')
                continue
            
            if cmd == 'stop':
                recording = False
                if angles:
                    print(f'\nSauvegarde de la carte avec {len(angles)} points...')
                    plot_points(angles, dists, out_file=args.out)
                    print(f'✓ Carte sauvegardée: {args.out}\n')
                    fig.suptitle("SONAR Carte 2D - Sauvegardée", fontsize=14, fontweight='bold')
                else:
                    print('Aucun point à sauvegarder')
                    fig.suptitle("SONAR Carte 2D - En attente de démarrage...", fontsize=14, fontweight='bold')
                fig.canvas.draw()
                continue
            
            if cmd == 'q':
                break
            
            if cmd == 'save':
                if angles:
                    print(f'Sauvegarde rapide ({len(angles)} points)...')
                    plot_points(angles, dists, out_file=args.out)
                    print(f'✓ Sauvegardé: {args.out}')
                else:
                    print('Aucun point à sauvegarder')
                continue
            
            if cmd == 'clear':
                angles = []
                dists = []
                print('✓ Points effacés')
                update_plot_realtime(fig, ax, angles, dists)
                continue
            
            if cmd == 'record' or cmd == 'r' or cmd == '':
                try:
                    ang_str = input('  Angle (degrés, 0-360): ').strip()
                    ang = float(ang_str)
                    if not (0 <= ang <= 360):
                        print('  ERROR: Angle entre 0 et 360')
                        continue
                except Exception:
                    print('  ERROR: Angle invalide')
                    continue

                if not reader.send_command(f"{ang}\n"):
                    print('  ERROR: Impossible d envoyer la commande au servo')
                    continue

                print(f'  Envoi angle {ang}° au servo...')
                received_angle, dist = reader.wait_for_reading(expected_angle=ang, timeout=2.0)
                if dist is None:
                    with reader.lock:
                        dist = reader.last_distance
                    if dist is None:
                        print('  ERROR: Aucune distance reçue du capteur ultrason')
                        continue
                    print('  Aucune réponse angle dédiée reçue, utilisation de la dernière distance lue')
                    received_angle = ang
                else:
                    print(f'  ✓ Réponse reçue: {received_angle}° / {dist} cm')

                angles.append(received_angle if received_angle is not None else ang)
                dists.append(dist)
                print(f'  ✓ Point enregistré: {received_angle if received_angle is not None else ang}° / {dist} cm ({len(angles)} points)')
                update_plot_realtime(fig, ax, angles, dists)
                continue
            
            # allow direct numeric entry: angle,dist
            if ',' in cmd:
                try:
                    a_s, d_s = cmd.split(',', 1)
                    a = float(a_s)
                    d = float(d_s)
                    angles.append(a)
                    dists.append(d)
                    print(f'  ✓ Point ajouté: {a}° / {d} cm ({len(angles)} points)')
                    update_plot_realtime(fig, ax, angles, dists)
                except Exception:
                    print('  ERROR: Format invalide (utilise: angle,distance)')
                continue
            
            if cmd:
                print('Commande inconnue. Tapez "h" pour l\'aide')
    
    except KeyboardInterrupt:
        print('\n')
    finally:
        reader.stop()
        plt.close('all')
        print('Au revoir!')


if __name__ == '__main__':
    main()
