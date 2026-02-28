#ifndef TIPOS_H
#define TIPOS_H

#include <Arduino.h>

#define ID_MAXIMO_CELULA 5 // CELULAS DE LA 0..4
#define NR_MAX_PARAMETROS 13
#define LONG_MAX_PARAMETRO 16

typedef enum DATA_RATE
{
    DATA_RATE_50 = 50,
    DATA_RATE_150 = 150,
    DATA_RATE_300 = 300,
    DATA_RATE_600 = 600,
    DATA_RATE_800 = 800,
    DATA_RATE_1000 = 1000,
    DATA_RATE_1200 = 1200
} TDataRate;

typedef struct
{
    int idcel; // Codigos
    float cap; // Capacidad
    int pol;   // Invertir polaridad
    float res; // resolucion
    // maxima carga soportada por la célula
    float limite_carga_celP; // positivo
    float limite_carga_celN; // negativo
    // Ganancia
    float gainpasostoFPos; // Relacion pasos-a-unidades-de-fuerza-positivo * 1000
    float gainpasostoFNeg; // Relacion pasos-a-unidades-de-fuerza-negativo * 1000
    TDataRate datarate;    // Hz data rate al que se ajusta la celula
} TCelula;

typedef struct
{

    float CCF_N;
    float CCF_KN;
    float CCF_K;
    float CCF_LB;

    // Datos de la célula en RAM
    byte id_celula; // numero de celula
    float cap_celula;
    float res_celula;
    int pol_celula;
    float gan_celulaPos; // pasos por unidad de fuerza
    float gan_celulaNeg;
    float limite_carga_celPos;
    float limite_carga_celNeg;
    long pasos_limite_carga_celPos;
    long pasos_limite_carga_celNeg;
    int datarate_celula;

    // Encoder
    long PosEncoder; // posición del encoder en pasos
    float CCE_MM;
    float CCE_IN;

    // Datos conversor
    long dac_CH1;
    long dac_CH2;
    long dac_filtrado_CH1;
    long dac_filtrado_CH2;
    long max_dac_CH1;       // máximo acumulado no filtrado
    long max_dac_tramo_CH1; // máximo parcial auto reseteable
    long max_dac_tramo_filtrado_CH1;

    long Cero_canal1;
    long Cero_canal2;

    int salida_datos_continua_start;
    int modo_salida_datos_binario;

} TVarGlobal;

typedef struct
{
    int clave;
    int idEstacion;

    int IPE;         // polaridad de la extension
    byte count_mode; // quadrature count mode x1, x2, x4
    float PasoHusillo;
    float PasosEncoder;
    TCelula celulas[ID_MAXIMO_CELULA];

    boolean filtro_on_off;
    float gainpos;
    float gainneg;

    char id_maquina[15];
    uint16_t checksum;
} TVarEEprom;

typedef struct
{
    char params[NR_MAX_PARAMETROS][LONG_MAX_PARAMETRO];

} TComando;

#endif
