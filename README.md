Aquí tienes un fichero **README.md** profesional y exhaustivo para tu repositorio de Git. Está diseñado específicamente para el **Kernel 3.0** y la placa **ARD18-V1**, estructurado para que cualquier desarrollador o ingeniero pueda entender e integrar el sistema.

---

# ARD18-V1 Force & Extension Kernel v3.0

Firmware industrial basado en Arduino para el control de celdas de carga y medición de extensión. Este Kernel está optimizado para la placa electrónica **ARD18-V1**, proporcionando alta precisión mediante el uso de conversiones de 24 bits y procesamiento por interrupciones.

## 🚀 Características Técnicas

- **Adquisición de Fuerza:** Conversor **AD7730** de 18 bits. Lecturas gestionadas por interrupción de hardware para máxima estabilidad.
- **Medición de Extensión:** Chip **LS7366R** para lectura de encoders en cuadratura (X1, X2, X4) con registros de 32 bits.
- **Procesamiento de Datos:**
    - Buffer circular de datos optimizado para evitar pérdida de muestras.
    - Filtro de Media Móvil Simple (SMA) y Exponencial (EMA/RC) seleccionables.
    - Detección de picos máximos (Real-time y por tramos) con protección atómica.
- **Gestión de Configuración:** Persistencia en EEPROM con verificación de integridad mediante **Checksum**.
- **Seguridad:** Alarmas de sobrecarga programables con salidas digitales físicas y lógica de histéresis.

---

## 📡 Configuración de Comunicación (RS232)

El sistema utiliza un protocolo basado en ASCII con tramas delimitadas.

| Parámetro | Valor |
| :--- | :--- |
| **Velocidad (Baudrate)** | 38400 bps |
| **Data Bits** | 8 |
| **Paridad** | None |
| **Stop Bits** | 1 |
| **Formato de Trama** | `:ID\|CMD\|PARAM1\|PARAM2\r` |

- **Inicio de trama:** `:` (Dos puntos)
- **Separador:** `|` (Pipe)
- **Fin de trama:** `\r` (CR - Carriage Return / 0x0D)

---

## 📑 Diccionario de Comandos

### 1. Comandos de Lectura (Read)

| Comando | Descripción | Respuesta Ejemplo |
| :--- | :--- | :--- |
| `RI` | Información del sistema (Placa, Máquina, Kernel) | `ARD18-V1 / GENERIC / KERNEL 3.0` |
| `R1` | Lectura de fuerza actual (Newtons) | `125.305` |
| `R2` | Lectura de extensión actual (mm) | `10.025` |
| `R3` | Lectura de fuerza bruta (Pasos ADC) | `123350` |
| `R4` | Transmisión continua (Fuerza \| Extensión) | `125.45\|10.025` |
| `RA` | Pico máximo de fuerza (Autoreseteable) | `500.200` |
| `RB` | Pico máximo de fuerza en tramo actual | `450.120` |
| `RC` | Pico máximo de tramo filtrado | `448.900` |
| `RS` | Estado de alarmas (Bitmask) | `0` (OK) o `1, 2, 4...` (Error) |
| `RX` | Identificar código de célula conectada (Hardware) | `0` a `4` |

### 2. Comandos de Acción / Escritura (Write)

| Comando | Descripción | Notas |
| :--- | :--- | :--- |
| `WZ` | Cero de fuerza (Tara) | Captura offset y resetea picos. |
| `WE` | Cero de extensión | Resetea el contador del encoder. |
| `WY` | Hardware Reset ADC | Reinicializa y calibra el AD7730. |
| `WI\|P\|V`| Escritura en pin digital | P=Pin, V=Estado (0 o 1). |

### 3. Gestión de Parámetros (`RP` / `WP`)

| Comando | Descripción | Uso |
| :--- | :--- | :--- |
| `WP\|0` | **Salvar en EEPROM** | Persiste la configuración de la RAM. |
| `WP\|9\|V` | Filtro digital ON/OFF | V=1 (ON), V=0 (OFF). |
| `WP\|14\|V`| Paso de husillo | V=Valor en mm. |
| `WP\|15\|V`| Pasos del encoder | V=Pulsos por vuelta. |
| `WP\|19\|...`| Configurar celda de carga | Ver manual de calibración. |
| `RP\|99` | Test de memoria RAM | Devuelve bytes libres. |
| `WP\|999` | **Factory Reset** | Carga y salva valores por defecto. |

---

## 🛠 Arquitectura de Hardware (Pinout)

La placa **ARD18-V1** utiliza la siguiente asignación de pines:

- **SPI:** D11 (MOSI), D12 (MISO), D13 (SCK).
- **ADC AD7730:** CS: D10, RDY: D2 (INT0), Reset: D8.
- **Encoder LS7366:** CS: D9.
- **Alarmas:** D4 (Sobrecarga Positiva), D5 (Sobrecarga Negativa).
- **Debug:** D3 (Pin de test para análisis de tiempos de ISR).

---

## ⚙️ Instalación y Compilación

1. Requiere el IDE de Arduino 1.8.x o superior.
2. Librerías estándar necesarias: `SPI.h`, `EEPROM.h`.
3. Seleccionar placa: **Arduino Nano** o **Arduino Uno** (ATmega328P).
4. El Kernel utiliza la interrupción `INT0` para el ADC; no utilizar el Pin 2 para otros propósitos.

---

## 📝 Licencia e Integridad
Este firmware ha sido desarrollado para entornos de alta precisión. Las lecturas críticas están protegidas por bloques de exclusión mutua (`noInterrupts`) para garantizar la atomicidad de los datos de 32 bits en arquitecturas de 8 bits.

**Versión de Documentación:** 1.0  
**Fecha de revisión:** 2026-02-22  
**Autor:** [JJ García]