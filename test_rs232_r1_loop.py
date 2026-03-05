#!/usr/bin/env python3
# python .\test_rs232_r1_loop.py --port COM5 --baud=38400
import argparse
import sys
import time
from datetime import datetime

try:
    import serial
except ImportError:
    print("ERROR: falta pyserial. Instala con: pip install pyserial", file=sys.stderr)
    sys.exit(1)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Envía ':1|R1\\r' en bucle y muestra respuestas del equipo."
    )
    parser.add_argument("--port", required=True, help="Puerto serie (ej: COM3)")
    parser.add_argument("--baud", type=int, default=38400, help="Baudios (default: 38400)")
    parser.add_argument("--interval", type=float, default=0.1, help="Segundos entre envíos (default: 0.1)")
    parser.add_argument("--timeout", type=float, default=0.3, help="Timeout lectura en segundos (default: 0.3)")
    parser.add_argument("--count", type=int, default=0, help="Número de ciclos (0 = infinito)")
    parser.add_argument("--boot-wait", type=float, default=8.0, help="Espera máxima tras abrir puerto para recibir BOOT|READY (default: 8.0)")
    parser.add_argument("--ignore-prefix", default="RXC|,BOOT,EEPROM:", help="Prefijos de líneas a ignorar, separados por coma")
    parser.add_argument("--verbose", action="store_true", help="Muestra TX y RX en cada ciclo")
    args = parser.parse_args()

    cmd = b":0|R1\r"
    cmdx = b"R1\r"
    ignore_prefixes = tuple(p.strip() for p in args.ignore_prefix.split(",") if p.strip())

    try:
        with serial.Serial(args.port, args.baud, timeout=args.timeout, write_timeout=1) as ser:
            print(f"Conectado a {args.port} @ {args.baud} bps")

            # Esperar a que la placa envíe BOOT|READY (o timeout)
            if args.boot_wait > 0:
                print(f"Esperando BOOT|READY (máx {args.boot_wait:.1f}s) ...")
                boot_deadline = time.time() + args.boot_wait
                boot_ok = False
                while time.time() < boot_deadline:
                    raw = ser.read_until(b"\r")
                    if not raw:
                        continue
                    line = raw.decode("ascii", errors="replace").strip("\r\n")
                    if line:
                        print(f"  [boot] {line}")
                    if line.startswith("BOOT|READY"):
                        boot_ok = True
                        break
                if boot_ok:
                    print("Placa lista.")
                else:
                    print("AVISO: no se recibió BOOT|READY, continuando de todas formas.")
                    time.sleep(0.5)

            ser.reset_input_buffer()
            print("Enviando comando en bucle. Pulsa Ctrl+C para salir.")

            i = 0
            ok = 0
            timeout_count = 0
            t0 = time.time()

            while True:
                if args.count > 0 and i >= args.count:
                    break

                i += 1
                ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]

                ser.write(cmd)
                ser.flush()

                # Leer múltiples líneas dentro de la ventana de timeout
                deadline = time.time() + args.timeout
                got_response = False
                last_ignored = None

                while time.time() < deadline:
                    raw = ser.read_until(b"\r")
                    if not raw:
                        continue

                    rx = raw.decode("ascii", errors="replace").strip("\r\n")
                    if not rx:
                        continue

                    if any(rx.startswith(pref) for pref in ignore_prefixes):
                        last_ignored = rx
                        continue

                    ok += 1
                    got_response = True
                    if args.verbose:
                        print(f"[{ts}] TX: {cmd!r} | RX: '{rx}'")
                    else:
                        print(f"[{ts}] {rx}")
                    break

                if not got_response:
                    timeout_count += 1
                    if args.verbose and last_ignored is not None:
                        print(f"[{ts}] TIMEOUT (solo diag: '{last_ignored}')")
                    else:
                        print(f"[{ts}] TIMEOUT (sin respuesta)")

                #time.sleep(args.interval)

            dt = time.time() - t0
            rate = (i / dt) if dt > 0 else 0.0
            print("--- Resumen ---")
            print(f"Ciclos: {i}")
            print(f"Respuestas OK: {ok}")
            print(f"Timeouts: {timeout_count}")
            print(f"Frecuencia media: {rate:.2f} cmd/s")

    except serial.SerialException as exc:
        print(f"ERROR serial: {exc}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("\nInterrumpido por usuario")
        return 0

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
