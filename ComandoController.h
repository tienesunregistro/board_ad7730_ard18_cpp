#ifndef COMANDO_CONTROLLER_H
#define COMANDO_CONTROLLER_H

#include "tipos.h"
#include "config.h"

// Incluimos las definiciones completas (no forward declarations)
#include "CelulaController.h"
#include "LS7366Driver.h"
#include "EepromController.h"

// Unidades de fuerza
#define UF_N 0
#define UF_KN 1
#define UF_K 2
#define UF_LB 3

// Unidades de extensión
#define UE_MM 0
#define UE_IN 1

class ComandoController
{
public:
    ComandoController(TVarGlobal *vg, TVarEEprom *vgEEprom,
                      CelulaController *celulaController,
                      LS7366Driver *ls7366,
                      EepromController *eepromController);
    void init();
    void procesar(const char *comando);

private:
    TVarGlobal *_vg;
    TVarEEprom *_vgEEprom;
    CelulaController *_celulaController;
    LS7366Driver *_ls7366;
    EepromController *_eepromController;

    // --- Comandos de Lectura ---
    int do_cmd_ri();
    int do_cmd_r1();
    int do_cmd_r2();
    int do_cmd_r3();
    int do_cmd_r4();
    int do_cmd_ra();
    int do_cmd_rb();
    int do_cmd_rc();
    int do_cmd_rs();
    int do_cmd_rx();
    int do_cmd_rp(int n_params, char params[][LONG_MAX_PARAMETRO]);

    // --- Comandos de Escritura ---
    int do_cmd_we();
    int do_cmd_wz();
    int do_cmd_wy();
    int do_cmd_wi(int n_params, char params[][LONG_MAX_PARAMETRO]);
    int do_cmd_wp(int n_params, char params[][LONG_MAX_PARAMETRO]);
    int do_cmd_error(const char *msg);
    int do_cmd_wr();

    // --- Funciones de Cálculo (fieles al original) ---
    void RecalcularCoeficientes();
    float pasos_a_fuerza(long valor, int unidad);
    long fuerza_a_pasos(float valor, int unidad);
    float pasos_a_deformacion(long valor, int unidad);

    // --- Funciones de Lectura ---
    float LecturaFuerzaFiltrada();
    float LecturaFuerzaNoFiltrada();
    long LecturaFuerzaPasosFiltrada();
    long LecturaFuerzaPasosNoFiltrada();
    float LecturaFuerzaMaxima();
    float LecturaFuerzaMaximaTramo();
    float LecturaFuerzaMaximaTramoFiltrada();
    float LecturaExtension();

    // --- Utilidades ---
    void float32_to_serial_binary(float f);
    int get_params(char *cadena, char params[][LONG_MAX_PARAMETRO], int nr_max);
};

#endif
