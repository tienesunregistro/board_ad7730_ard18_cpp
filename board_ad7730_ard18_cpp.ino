#include <SPI.h>
#include <SoftwareSerial.h>

// Ficheros de configuración y tipos
#include "config.h"
#include "tipos.h"

// Drivers de Hardware
#include "AD7730Driver.h"
#include "LS7366Driver.h"

// Controladores de Lógica
#include "AlarmaController.h"
#include "BufferController.h"
#include "EepromController.h"
#include "CelulaController.h"
#include "ComandoController.h"

// --- Variables Globales Principales ---
TVarGlobal vg;
TVarEEprom vgEEprom;
SoftwareSerial SerialAux(6, 7); // RX, TX para debug

// --- Instancias de Drivers y Controladores ---
AD7730Driver ad7730(PIN_CS_AD7730, PIN_RDY_AD730, PIN_RESET_AD730);
LS7366Driver ls7366(PIN_CS_LS7366);
BufferController bufferController;
EepromController eepromController(&vgEEprom);
AlarmaController alarmaController(PIN_ALARMA_FUERZA_POST, PIN_ALARMA_FUERZA_NEG);
CelulaController celulaController(&vg, &vgEEprom, &ad7730);
ComandoController comandoController(&vg, &vgEEprom, &celulaController, &ls7366, &eepromController);

// --- Variables para la Interrupción ---
static unsigned long isr_secuencia = 0;

// --- Rutina de Servicio de Interrupción (ISR) ---
// Lee el ADC directamente y almacena en el buffer (idéntico al original ISR_RDY_ADC7730)
void ISR_ADC_RDY()
{
    digitalWrite(PIN_TEST_DEBUG, HIGH);

    if (!ad7730.configurado)
    {
        digitalWrite(PIN_TEST_DEBUG, LOW);
        return;
    }

    TDatoCanal datoCanalCelula;
    datoCanalCelula.dato = ad7730.leerDatoConFiltroISR();
    datoCanalCelula.t_ms = millis();
    datoCanalCelula.secuencia = isr_secuencia++;

    bufferController.store(&datoCanalCelula);

    digitalWrite(PIN_TEST_DEBUG, LOW);
}

// ============================================================================
//  SETUP
// ============================================================================
void setup()
{
    noInterrupts();

    Serial.begin(BAUDRATE_RS232);
    SerialAux.begin(38400);
    SPI.begin();

    pinMode(PIN_TEST_DEBUG, OUTPUT);

    // Inicializar todos los controladores
    eepromController.init();
    alarmaController.init();
    celulaController.init();  // Esto ya inicializa el AD7730 con la célula por defecto
    comandoController.init(); // << NUEVO: Inicializa los coeficientes de cálculo
    ls7366.init(vgEEprom.count_mode);
    bufferController.init();

    // Configurar y activar la interrupción del ADC
    attachInterrupt(digitalPinToInterrupt(PIN_RDY_AD730), ISR_ADC_RDY, FALLING);

    ad7730.configurado = true;

    interrupts();

    delay(500);               // Dar tiempo a que se llene el buffer con lecturas válidas
    bufferController.flush(); // Descartar las primeras lecturas (transitorio)

    celulaController.fuerzaCero();

    delay(1000); // Pausa para dar tiempo a leer los mensajes de debug

    vg.salida_datos_continua_start = 0;
    vg.modo_salida_datos_binario = 0;

    // Vaciar cualquier basura acumulada en el buffer de recepción durante el setup
    while (Serial.available())
        Serial.read();

    // Enviar marca de arranque para que el host sepa que estamos listos
    Serial.print(F("BOOT|READY"));
    Serial.write(CR);
}

// ============================================================================
//  LOOP
// ============================================================================
void loop()
{
    // 1. Consumir muestras del ADC desde el buffer circular
    //    (la ISR ya leyó el ADC y almacenó en el buffer)
    if (bufferController.isDataAvailable())
    {
        TDatoCanal datoLeido;
        bufferController.read(&datoLeido);
        long dataconvert = datoLeido.dato;

        // Aplicar polaridad de la célula (idéntico al original leer_adlc)
        if (vg.pol_celula < 0)
            dataconvert = -dataconvert;

        vg.dac_CH1 = dataconvert;

        // Aplicar filtro digital
        if (vgEEprom.filtro_on_off)
        {
            vg.dac_filtrado_CH1 = (vg.dac_filtrado_CH1 * 3 + vg.dac_CH1) / 4;
        }
        else
        {
            vg.dac_filtrado_CH1 = vg.dac_CH1;
        }

        // Rastrear picos máximos (idéntico al original leer_adlc)
        if (vg.dac_CH1 > vg.max_dac_tramo_CH1)
        {
            vg.max_dac_tramo_CH1 = vg.dac_CH1;
        }
        if (vg.dac_filtrado_CH1 > vg.max_dac_tramo_filtrado_CH1)
        {
            vg.max_dac_tramo_filtrado_CH1 = vg.dac_filtrado_CH1;
        }
        if (vg.dac_CH1 > vg.max_dac_CH1)
        {
            vg.max_dac_CH1 = vg.dac_CH1;
        }
    }

    // 3. Procesar comandos serie
    check_rs232();

    // 4. Comprobar alarmas
    static unsigned long last_check_alarmas = 0;
    if (millis() - last_check_alarmas > TIMEOUT_CHECK_ALARMAS)
    {
        alarmaController.check(vg.pasos_limite_carga_celPos, vg.pasos_limite_carga_celNeg, vg.dac_filtrado_CH1);
        last_check_alarmas = millis();
    }

    // 5. Transmisión continua (si está activa)
    if (vg.salida_datos_continua_start)
    {
        comandoController.procesar("R4");
    }
}

// ============================================================================
//  Manejo de Comunicación RS232
// ============================================================================
void check_rs232()
{
    static char inputBuffer[LONG_COMANDO];
    static int bufferIndex = 0;

    while (Serial.available() > 0)
    {
        char inChar = (char)Serial.read();

        if (inChar == CHAR_INICIO_CMD)
        {
            bufferIndex = 0;
            continue;
        }

        if (inChar == CHAR_FIN_CMD || inChar == LF)
        {
            if (bufferIndex > 0)
            {
                inputBuffer[bufferIndex] = '\0';

                char *cmdToProcess = inputBuffer;
                char *separator = strchr(inputBuffer, '|');

                if (separator != NULL)
                {
                    *separator = '\0';
                    int idRecibido = atoi(inputBuffer);
                    cmdToProcess = separator + 1;

                    if (idRecibido != vgEEprom.idEstacion && idRecibido != 0)
                    {
                        bufferIndex = 0;
                        continue; // ID no coincide, ignorar y seguir
                    }
                }

                SerialAux.print(F("CMD>"));
                SerialAux.println(cmdToProcess);
                comandoController.procesar(cmdToProcess);
            }
            bufferIndex = 0;
        }
        else if (bufferIndex < (LONG_COMANDO - 1))
        {
            if (inChar >= 32)
            {
                inputBuffer[bufferIndex++] = inChar;
            }
        }
    }
}
