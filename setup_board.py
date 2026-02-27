#!/usr/bin/env python3
# python .\setup_board.py --port COM5 --baud=38400

import argparse
import sys
import time
from datetime import datetime

try:
    import serial
except ImportError:
    print("ERROR: falta pyserial. Instala con: pip install pyserial", file=sys.stderr)
    sys.exit(1)

# ID global de la placa con la que queremos hablar desde el PC
TARGET_ID = 1

def send_command(ser, cmd_str, timeout=0.6):
    """Envía un comando formateado :ID|CMD y espera respuesta."""
    full_cmd = f":{TARGET_ID}|{cmd_str}\r".encode('ascii')
    ser.reset_input_buffer()
    ser.write(full_cmd)
    ser.flush()
    
    # Esperar respuesta terminada en \r
    deadline = time.time() + timeout
    while time.time() < deadline:
        if ser.in_waiting > 0:
            line = ser.readline().decode('ascii', errors='replace').strip()
            if line:
                return line
    return None

def menu_lectura_continua(ser):
    print(f"\n--- Lectura Continua (R1) - Placa {TARGET_ID} ---")
    print("Presiona Ctrl+C para volver al menú principal")
    try:
        while True:
            resp = send_command(ser, "R1", timeout=0.2)
            ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            if resp:
                print(f"[{ts}] {resp}")
            else:
                print(f"[{ts}] ERROR: Sin respuesta")
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nLectura detenida.")

def modificar_id_estacion(ser):
    global TARGET_ID
    print(f"\n--- Modificar ID de la Estación (WP|10) ---")
    nuevo = input(f"Introduce el nuevo ID para la placa {TARGET_ID} (0-99): ")
    if not nuevo.isdigit():
        print("Error: El ID debe ser un número.")
        return

    # Comando: :ID_ACTUAL|WP|10|NUEVO_ID
    resp = send_command(ser, f"WP|10|{nuevo}")
    print(f"Respuesta de la placa: {resp if resp else 'Comando enviado (sin respuesta)'}")
    
    confirm = input(f"¿Deseas actualizar el ID de destino en este script a {nuevo}? (s/n): ")
    if confirm.lower() == 's':
        TARGET_ID = int(nuevo)

def modificar_config_celula(ser):
    print(f"\n--- Configuración Parámetros Célula (WP|19) ---")
    print("Formato: id_cel|cap|pol|res|limP|limN|gainP|gainN|rate")
    val = input("Introduce la cadena completa: ")
    if not val: return

    resp = send_command(ser, f"WP|19|{val}")
    print(f"Respuesta placa: {resp if resp else 'Comando enviado'}")

def main() -> int:
    global TARGET_ID
    parser = argparse.ArgumentParser(description="Terminal de Control ARD18-V1")
    parser.add_argument("--port", required=True, help="Puerto serie (ej: COM3 o /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=38400, help="Baudios (default 38400)")
    args = parser.parse_args()

    try:
        with serial.Serial(args.port, args.baud, timeout=0.5) as ser:
            print(f"Conectado a {args.port}. Esperando inicialización...")
            time.sleep(2) # Tiempo para el bootloader del Arduino

            while True:
                print(f"\n==========================================")
                print(f" PLACA OBJETIVO ACTUAL: {TARGET_ID}")
                print(f"==========================================")
                print("1. Lectura de Fuerza Continua (R1)")
                print("2. CAMBIAR ID DE LA ESTACIÓN (WP|10)")
                print("3. Configurar Parámetros Célula (WP|19)")
                print("4. Salvar en EEPROM (WP|0)")
                print("5. Restaurar valores de fábrica (WP|999)")
                print("6. Cambiar ID de destino en este SCRIPT")
                print("q. Salir")
                
                op = input("\nSelecciona opción: ").lower()

                if op == '1': menu_lectura_continua(ser)
                elif op == '2': modificar_id_estacion(ser)
                elif op == '3': modificar_config_celula(ser)
                elif op == '4':
                    print("Salvando configuración...")
                    print(f"Respuesta: {send_command(ser, 'WP|0')}")
                elif op == '5':
                    if input("¿Restaurar TODO a fábrica? (s/n): ").lower() == 's':
                        print(f"Respuesta: {send_command(ser, 'WP|999')}")
                elif op == '6':
                    TARGET_ID = int(input("Nuevo ID de destino para el script: "))
                elif op == 'q':
                    break

    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())