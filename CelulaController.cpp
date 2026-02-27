#include "CelulaController.h"
#include "AD7730Driver.h"
#include "config.h"

CelulaController::CelulaController(TVarGlobal *vg, TVarEEprom *vgEEprom, AD7730Driver *ad7730)
{
    _vg = vg;
    _vgEEprom = vgEEprom;
    _ad7730 = ad7730;
}

void CelulaController::init()
{
    // Inicializa con la primera celula por defecto
    seleccionar(0);
}

bool CelulaController::seleccionar(byte id_celula)
{
    if (id_celula >= ID_MAXIMO_CELULA)
    {
        return false;
    }

    TCelula *celula = &_vgEEprom->celulas[id_celula];

    _vg->id_celula = id_celula;
    _vg->cap_celula = celula->cap;
    _vg->res_celula = celula->res;
    _vg->pol_celula = celula->pol;
    _vg->gan_celulaPos = celula->gainpasostoFPos;
    _vg->gan_celulaNeg = celula->gainpasostoFNeg;
    _vg->limite_carga_celPos = celula->limite_carga_celP;
    _vg->limite_carga_celNeg = celula->limite_carga_celN;
    _vg->datarate_celula = (int)celula->datarate;

    // Calcular los límites en pasos
    _vg->pasos_limite_carga_celPos = getPasosFromFuerza(celula->limite_carga_celP);
    _vg->pasos_limite_carga_celNeg = getPasosFromFuerza(celula->limite_carga_celN);

    // Re-inicializar el ADC con el nuevo datarate
    _ad7730->init(celula->datarate);

    return true;
}

void CelulaController::fuerzaCero()
{
    _vg->Cero_canal1 = _vg->dac_filtrado_CH1;
}

long CelulaController::getPasosFromFuerza(float fuerza)
{
    if (fuerza == 0)
    {
        return 0;
    }

    float ganancia = (fuerza > 0) ? _vg->gan_celulaPos : _vg->gan_celulaNeg;
    if (ganancia == 0)
    {
        return 0;
    }

    return (long)(fuerza * ganancia / 1000.0);
}

float CelulaController::getFuerzaFromPasos(long pasos)
{
    if (pasos == 0)
    {
        return 0.0;
    }

    float ganancia = (pasos > 0) ? _vg->gan_celulaPos : _vg->gan_celulaNeg;
    if (ganancia == 0)
    {
        return 0.0;
    }

    return (float)(pasos * 1000.0 / ganancia);
}
