#include "ComandoController.h"
#include "AD7730Driver.h"
#include "Arduino.h"
#include <SoftwareSerial.h>

extern SoftwareSerial SerialAux;

// Union para convertir float a bytes (idéntico al original)
union conversor
{
    float valorf;
    uint8_t Data[4];
};

// ============================================================================
//  CONSTRUCTOR E INIT
// ============================================================================

ComandoController::ComandoController(TVarGlobal *vg, TVarEEprom *vgEEprom,
                                     CelulaController *celulaController,
                                     LS7366Driver *ls7366,
                                     EepromController *eepromController)
{
    _vg = vg;
    _vgEEprom = vgEEprom;
    _celulaController = celulaController;
    _ls7366 = ls7366;
    _eepromController = eepromController;
}

void ComandoController::init()
{
    RecalcularCoeficientes();
}

// ============================================================================
//  PARSEO DE COMANDOS (fiel al procesarComando original)
// ============================================================================

int ComandoController::get_params(char *cadena, char params[][LONG_MAX_PARAMETRO], int nr_max)
{
    char *p;
    char *token;
    int i = 0;

    token = strtok_r(cadena, "|", &p);
    while (token != NULL)
    {
        if (i < nr_max)
        {
            strncpy(params[i], token, LONG_MAX_PARAMETRO - 1);
            params[i][LONG_MAX_PARAMETRO - 1] = '\0';
            i++;
        }
        else
        {
            break;
        }
        token = strtok_r(NULL, "|", &p);
    }
    return i;
}

void ComandoController::procesar(const char *comando)
{
    static char cmd_copy[LONG_COMANDO + 1];
    static char params[NR_MAX_PARAMETROS][LONG_MAX_PARAMETRO];
    int n_params = 0;

    memset(params, 0, sizeof(params));

    strncpy(cmd_copy, comando, LONG_COMANDO);
    cmd_copy[LONG_COMANDO] = '\0';

    char *sep = strchr(cmd_copy, '|');
    char cmd_name[5];

    if (sep != NULL)
    {
        int cmdLen = sep - cmd_copy;
        strncpy(cmd_name, cmd_copy, (cmdLen > 4) ? 4 : cmdLen);
        cmd_name[(cmdLen > 4) ? 4 : cmdLen] = '\0';
        n_params = get_params(sep + 1, params, NR_MAX_PARAMETROS);
    }
    else
    {
        strncpy(cmd_name, cmd_copy, 4);
        cmd_name[4] = '\0';
        n_params = 0;
    }

    if (strlen(cmd_name) < 2)
    {
        do_cmd_error("E|CMD_SHORT");
        return;
    }

    char prefijo = cmd_name[0];
    char accion = cmd_name[1];

    if (prefijo == 'R')
    {
        switch (accion)
        {
        case '1':
            do_cmd_r1();
            break;
        case '2':
            do_cmd_r2();
            break;
        case '3':
            do_cmd_r3();
            break;
        case '4':
            do_cmd_r4();
            break;
        case 'A':
            do_cmd_ra();
            break;
        case 'B':
            do_cmd_rb();
            break;
        case 'C':
            do_cmd_rc();
            break;
        case 'P':
            do_cmd_rp(n_params, params);
            break;
        case 'S':
            do_cmd_rs();
            break;
        case 'X':
            do_cmd_rx();
            break;
        case 'I':
            do_cmd_ri();
            break;
        default:
            do_cmd_error("E|CMD_UNKNOWN");
            break;
        }
    }
    else if (prefijo == 'W')
    {
        switch (accion)
        {
        case 'P':
            do_cmd_wp(n_params, params);
            break;
        case 'E':
            do_cmd_we();
            break;
        case 'I':
            do_cmd_wi(n_params, params);
            break;
        case 'Y':
            do_cmd_wy();
            break;
        case 'Z':
            do_cmd_wz();
            break;
        default:
            do_cmd_error("E|CMD_UNKNOWN");
            break;
        }
    }
    else
    {
        do_cmd_error("E|CMD_UNKNOWN");
    }
}

// ============================================================================
//  COMANDOS DE LECTURA
// ============================================================================

int ComandoController::do_cmd_ri()
{
    Serial.print(F(IDBOARD));
    Serial.print(F(" / "));
    Serial.print(F(IDMAQ));
    Serial.print(F(" / "));
    Serial.print(F(SKERNEL));
    Serial.write(CR);
    return 1;
}

// Lectura de fuerza en N, con compensación de cero
int ComandoController::do_cmd_r1()
{
    float datoF;

    if (_vgEEprom->filtro_on_off)
    {
        datoF = LecturaFuerzaFiltrada();
    }
    else
    {
        datoF = LecturaFuerzaNoFiltrada();
    }

    if (abs(datoF) > 10000)
    {
        Serial.print(datoF, 1);
    }
    else
    {
        Serial.print(datoF, 3);
    }
    Serial.write(CR);
    return 1;
}

// Lectura encoder
int ComandoController::do_cmd_r2()
{
    float value = LecturaExtension();
    Serial.print(value, 3);
    Serial.write(CR);
    return 1;
}

// Lectura analógico CH1, valor en PASOS
int ComandoController::do_cmd_r3()
{
    long value;

    if (_vgEEprom->filtro_on_off)
    {
        value = LecturaFuerzaPasosFiltrada();
    }
    else
    {
        value = LecturaFuerzaPasosNoFiltrada();
    }

    Serial.print(value);
    Serial.write(CR);
    return 1;
}

// Lectura de fuerza y extensión (binario o texto)
int ComandoController::do_cmd_r4()
{
    float datoF;

    uint8_t fs1 = 0xAA;
    uint8_t fs2 = 0x55;
    uint8_t nrBytes = 8;

    if (_vgEEprom->filtro_on_off)
    {
        datoF = LecturaFuerzaFiltrada();
    }
    else
    {
        datoF = LecturaFuerzaNoFiltrada();
    }

    if (!_vg->modo_salida_datos_binario)
    {
        if (abs(datoF) > 10000)
        {
            Serial.print(datoF, 1);
            Serial.write('|');
        }
        else
        {
            Serial.print(datoF, 3);
            Serial.write('|');
        }
    }
    else
    {
        Serial.write(fs1);
        Serial.write(fs2);
        Serial.write(nrBytes);
        float32_to_serial_binary(datoF);
    }

    datoF = LecturaExtension();

    if (!_vg->modo_salida_datos_binario)
    {
        Serial.print(datoF, 3);
        Serial.write(CR);
        Serial.write(LF);
    }
    else
    {
        float32_to_serial_binary(datoF);
    }

    return 1;
}

// Pico máximo de fuerza acumulado, autoreseteable
int ComandoController::do_cmd_ra()
{
    float datoF = LecturaFuerzaMaxima();
    Serial.print(datoF, 3);
    Serial.write(CR);
    return 1;
}

// Pico máximo de fuerza en tramo, autoreseteable
int ComandoController::do_cmd_rb()
{
    float datoF = LecturaFuerzaMaximaTramo();
    Serial.print(datoF, 3);
    Serial.write(CR);
    return 1;
}

// Pico máximo de fuerza en tramo filtrado, autoreseteable
int ComandoController::do_cmd_rc()
{
    float datoF = LecturaFuerzaMaximaTramoFiltrada();
    Serial.print(datoF, 3);
    Serial.write(CR);
    return 1;
}

// Alarmas
int ComandoController::do_cmd_rs()
{
    // TODO: integrar con AlarmaController
    Serial.print(0);
    Serial.write(CR);
    return 1;
}

// Identificar extensómetro por lectura analógica A0
int ComandoController::do_cmd_rx()
{
    float m_v = analogRead(A0) * (5000.0 / 1023.0);

    int celula = 0;
    if (m_v < 200)
        celula = 0;
    else if (m_v <= 1500)
        celula = 1;
    else if (m_v <= 2500)
        celula = 2;
    else if (m_v <= 3500)
        celula = 3;
    else
        celula = 4;

    Serial.print(celula);
    Serial.write(CR);
    return 1;
}

// Lectura de parámetros (fiel al original do_cmd_rp)
int ComandoController::do_cmd_rp(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 1)
        return do_cmd_error("E|PARAM_COUNT");

    int indice = 0;
    int funcion = atoi(params[0]);

    switch (funcion)
    {
    case 9: // filtro_on_off
        Serial.print(_vgEEprom->filtro_on_off);
        Serial.write(CR);
        break;

    case 10: // idEstacion
        Serial.print(_vgEEprom->idEstacion);
        Serial.write(CR);
        break;

    case 13: // parámetros célula en RAM
        Serial.print(_vg->id_celula);
        Serial.print('|');
        Serial.print(_vg->cap_celula, 3);
        Serial.print('|');
        Serial.print(_vg->pol_celula);
        Serial.print('|');
        Serial.print(_vg->res_celula, 3);
        Serial.print('|');
        Serial.print(_vg->limite_carga_celPos, 3);
        Serial.print('|');
        Serial.print(_vg->limite_carga_celNeg, 3);
        Serial.print('|');
        Serial.print(_vg->gan_celulaPos, 3);
        Serial.print('|');
        Serial.print(_vg->gan_celulaNeg, 3);
        Serial.print('|');
        Serial.print(_vg->datarate_celula);
        Serial.write(CR);
        break;

    case 14: // PasoHusillo
        Serial.print(_vgEEprom->PasoHusillo);
        Serial.write(CR);
        break;

    case 15: // PasosEncoder
        Serial.print(_vgEEprom->PasosEncoder);
        Serial.write(CR);
        break;

    case 16: // polaridad extensión
        Serial.print(_vgEEprom->IPE);
        Serial.write(CR);
        break;

    case 17: // modo quadrature
        Serial.print(_vgEEprom->count_mode);
        Serial.write(CR);
        break;

    case 19: // parámetros célula en EEPROM
        if (n_params < 2)
            return do_cmd_error("E|PARAM_COUNT");
        indice = atoi(params[1]);
        if (indice < 0 || indice >= ID_MAXIMO_CELULA)
            return do_cmd_error("E|CELL_ID_OOR");

        Serial.print(_vgEEprom->celulas[indice].idcel);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].cap, 3);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].pol);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].res, 3);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].limite_carga_celP, 3);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].limite_carga_celN, 3);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].gainpasostoFPos, 5);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].gainpasostoFNeg, 5);
        Serial.print('|');
        Serial.print(_vgEEprom->celulas[indice].datarate);
        Serial.write(CR);
        break;

    case 21: // salida_datos_continua_start
        Serial.print(_vg->salida_datos_continua_start);
        Serial.write(CR);
        break;

    case 22: // modo_salida_datos_binario
        Serial.print(_vg->modo_salida_datos_binario);
        Serial.write(CR);
        break;

    case 90: // ID máquina
        Serial.print(_vgEEprom->id_maquina);
        Serial.write(CR);
        break;

    case 99: // Test memoria RAM
    {
        extern int __heap_start, *__brkval;
        int v;
        Serial.print(F("Memoria RAM:"));
        Serial.print((int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));
        Serial.write(CR);
        break;
    }

    default:
        return do_cmd_error("E|PARAM_UNKNOWN");
    }

    return 1;
}

// ============================================================================
//  COMANDOS DE ESCRITURA
// ============================================================================

// Cero de extensión
int ComandoController::do_cmd_we()
{
    _ls7366->cero();
    _ls7366->leer();
    return 1;
}

// Cero de fuerza
int ComandoController::do_cmd_wz()
{
    _celulaController->fuerzaCero();
    return 1;
}

// Reset hardware del conversor
int ComandoController::do_cmd_wy()
{
    // Detener ISR durante la reinicialización
    _celulaController->getAD7730()->configurado = false;
    _celulaController->seleccionar(_vg->id_celula);
    delay(100);
    _celulaController->getAD7730()->configurado = true;
    _celulaController->fuerzaCero();
    return 1;
}

// Escritura I/O
int ComandoController::do_cmd_wi(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 2)
        return do_cmd_error("E|PARAM_COUNT");

    int pin = atoi(params[0]);
    int estado = atoi(params[1]);

    if (pin != PIN_ALARMA_FUERZA_POST && pin != PIN_ALARMA_FUERZA_NEG && pin != PIN_TEST_DEBUG)
        return do_cmd_error("E|PIN_INVALID");

    digitalWrite(pin, estado);
    return 1;
}

// Escritura de parámetros (fiel al original do_cmd_wp)
int ComandoController::do_cmd_wp(int n_params, char params[][LONG_MAX_PARAMETRO])
{
    if (n_params < 1)
        return do_cmd_error("E|PARAM_COUNT");

    int indice = 0;
    int funcion = atoi(params[0]);

    switch (funcion)
    {
    case 0: // WP|0 - Salvar EEPROM
        if (n_params != 1)
            return do_cmd_error("E|PARAM_COUNT");
        _eepromController->salvar();
        break;

    case 9: // WP|9|xx - filtro_on_off
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->filtro_on_off = atoi(params[1]);
        break;

    case 10: // WP|10|xx - idEstacion
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->idEstacion = atoi(params[1]);
        break;

    case 14: // WP|14|xx - PasoHusillo
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->PasoHusillo = atof(params[1]);
        RecalcularCoeficientes();
        break;

    case 15: // WP|15|xx - PasosEncoder
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->PasosEncoder = atof(params[1]);
        RecalcularCoeficientes();
        break;

    case 16: // WP|16|xx - IPE (polaridad extensión)
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->IPE = atoi(params[1]);
        RecalcularCoeficientes();
        break;

    case 17: // WP|17|xx - count_mode
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vgEEprom->count_mode = atoi(params[1]);
        _ls7366->init(_vgEEprom->count_mode);
        RecalcularCoeficientes();
        break;

    case 19: // WP|19|idcel|cap|pol|res|limP|limN|gainP|gainN|datarate
        if (n_params != 10)
            return do_cmd_error("E|PARAM_COUNT_19");

        indice = atoi(params[1]);
        if (indice < 0 || indice >= ID_MAXIMO_CELULA)
            return do_cmd_error("E|CELL_ID_OOR");

        _vgEEprom->celulas[indice].idcel = atoi(params[1]);
        _vgEEprom->celulas[indice].cap = atof(params[2]);
        _vgEEprom->celulas[indice].pol = atoi(params[3]);
        _vgEEprom->celulas[indice].res = atof(params[4]);
        _vgEEprom->celulas[indice].limite_carga_celP = abs(atof(params[5]));
        _vgEEprom->celulas[indice].limite_carga_celN = abs(atof(params[6]));
        _vgEEprom->celulas[indice].gainpasostoFPos = atof(params[7]);
        _vgEEprom->celulas[indice].gainpasostoFNeg = atof(params[8]);
        _vgEEprom->celulas[indice].datarate = (TDataRate)atoi(params[9]);
        _celulaController->getAD7730()->configurado = false;
        _celulaController->seleccionar(indice);
        _celulaController->getAD7730()->configurado = true;
        break; // WP|20|xx - activar célula
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        indice = atoi(params[1]);
        if (indice < 0 || indice >= ID_MAXIMO_CELULA)
            return do_cmd_error("E|CELL_ID_OOR");
        _celulaController->getAD7730()->configurado = false;
        _celulaController->seleccionar(indice);
        _celulaController->getAD7730()->configurado = true;
        break;

    case 21: // WP|21|xx - salida_datos_continua
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vg->salida_datos_continua_start = atoi(params[1]);
        break;

    case 22: // WP|22|xx - modo_salida_datos_binario
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        _vg->modo_salida_datos_binario = atoi(params[1]);
        break;

    case 90: // WP|90|xx - ID máquina
        if (n_params != 2)
            return do_cmd_error("E|PARAM_COUNT");
        strncpy(_vgEEprom->id_maquina, params[1], 14);
        _vgEEprom->id_maquina[14] = '\0';
        break;

    case 999: // WP|999 - Reset a valores de fábrica
        if (n_params != 1)
            return do_cmd_error("E|PARAM_COUNT");
        _eepromController->setValoresPorDefecto();
        _eepromController->salvar();
        break;

    default:
        return do_cmd_error("E|PARAM_UNKNOWN");
    }

    return 1;
}

int ComandoController::do_cmd_error(const char *msg)
{
    Serial.print(msg);
    Serial.write(CR);
    return 0;
}

// ============================================================================
//  FUNCIONES DE CÁLCULO (fieles al original celulas.ino / extension.ino)
// ============================================================================

// Coeficientes para el paso de unidades (idéntico al original)
void ComandoController::RecalcularCoeficientes()
{
    _vg->CCF_N = 1;
    _vg->CCF_KN = 1 / 1000.0;
    _vg->CCF_K = 1 / 9.81;
    _vg->CCF_LB = 1 * 0.225;

    if (_vgEEprom->PasosEncoder != 0.0)
    {
        _vg->CCE_MM = _vgEEprom->PasoHusillo / _vgEEprom->PasosEncoder;
        _vg->CCE_IN = (_vgEEprom->PasoHusillo / _vgEEprom->PasosEncoder) * 0.039;
    }
    else
    {
        _vg->CCE_MM = 0;
        _vg->CCE_IN = 0;
    }
}

// Conversión de pasos a fuerza (idéntico al original)
float ComandoController::pasos_a_fuerza(long valor, int unidad)
{
    double res;
    float IPF;
    float res_fuerza = _vg->res_celula;

    if (valor >= 0)
    {
        IPF = (_vg->gan_celulaPos > 0) ? _vg->gan_celulaPos : 1.0f;
    }
    else
    {
        IPF = (_vg->gan_celulaNeg > 0) ? _vg->gan_celulaNeg : 1.0f;
    }

    switch (unidad)
    {
    case UF_N:
        res = (double)valor * _vg->CCF_N;
        break;
    case UF_KN:
        res = (double)valor * _vg->CCF_KN;
        break;
    case UF_K:
        res = (double)valor * _vg->CCF_K;
        break;
    case UF_LB:
        res = (double)valor * _vg->CCF_LB;
        break;
    default:
        return 9999.0f;
    }

    res = res / (double)IPF;

    if (res_fuerza > 0.0f)
    {
        res = round(res / (double)res_fuerza) * (double)res_fuerza;
    }

    return (float)res;
}

// Conversión de fuerza a pasos (idéntico al original)
long ComandoController::fuerza_a_pasos(float valor, int unidad)
{
    long res;
    float IPF;

    if (valor >= 0)
    {
        IPF = _vg->gan_celulaPos;
    }
    else
    {
        IPF = _vg->gan_celulaNeg;
    }

    switch (unidad)
    {
    case UF_N:
        res = (long)(valor / _vg->CCF_N * IPF);
        break;
    case UF_KN:
        res = (long)(valor / _vg->CCF_KN * IPF);
        break;
    case UF_K:
        res = (long)(valor / _vg->CCF_K * IPF);
        break;
    case UF_LB:
        res = (long)(valor / _vg->CCF_LB * IPF);
        break;
    default:
        res = 9999;
        break;
    }

    return res;
}

// Conversión de pasos a deformación (idéntico al original)
float ComandoController::pasos_a_deformacion(long valor, int unidad)
{
    float res;

    switch (unidad)
    {
    case UE_MM:
        res = valor * _vg->CCE_MM * _vgEEprom->IPE;
        break;
    case UE_IN:
        res = valor * _vg->CCE_IN * _vgEEprom->IPE;
        break;
    default:
        res = 9999;
        break;
    }

    res = ((long)(res / 0.001)) * 0.001;
    return res;
}

// ============================================================================
//  FUNCIONES DE LECTURA (fieles al original)
// ============================================================================

long ComandoController::LecturaFuerzaPasosFiltrada()
{
    return _vg->dac_filtrado_CH1;
}

long ComandoController::LecturaFuerzaPasosNoFiltrada()
{
    return _vg->dac_CH1;
}

float ComandoController::LecturaFuerzaFiltrada()
{
    long value = LecturaFuerzaPasosFiltrada();
    value = value - _vg->Cero_canal1;
    return pasos_a_fuerza(value, UF_N);
}

float ComandoController::LecturaFuerzaNoFiltrada()
{
    long value = LecturaFuerzaPasosNoFiltrada();
    value = value - _vg->Cero_canal1;
    return pasos_a_fuerza(value, UF_N);
}

float ComandoController::LecturaFuerzaMaxima()
{
    noInterrupts();
    long value = _vg->max_dac_CH1 - _vg->Cero_canal1;
    _vg->max_dac_CH1 = _vg->dac_CH1;
    interrupts();
    return pasos_a_fuerza(value, UF_N);
}

float ComandoController::LecturaFuerzaMaximaTramo()
{
    noInterrupts();
    long value = _vg->max_dac_tramo_CH1 - _vg->Cero_canal1;
    _vg->max_dac_tramo_CH1 = _vg->dac_CH1;
    interrupts();
    return pasos_a_fuerza(value, UF_N);
}

float ComandoController::LecturaFuerzaMaximaTramoFiltrada()
{
    noInterrupts();
    long value = _vg->max_dac_tramo_filtrado_CH1 - _vg->Cero_canal1;
    _vg->max_dac_tramo_filtrado_CH1 = _vg->dac_filtrado_CH1;
    interrupts();
    return pasos_a_fuerza(value, UF_N);
}

float ComandoController::LecturaExtension()
{
    long pasos = _ls7366->leer();
    return pasos_a_deformacion(pasos, UE_MM);
}

// ============================================================================
//  UTILIDADES
// ============================================================================

void ComandoController::float32_to_serial_binary(float datoF)
{
    union conversor valor;
    valor.valorf = datoF;
    for (int i = 0; i < 4; i++)
    {
        Serial.write(valor.Data[3 - i]);
    }
}
