#include "EepromController.h"
#include <EEPROM.h>
#include "config.h"

EepromController::EepromController(TVarEEprom *vgEEprom)
{
    _vgEEprom = vgEEprom;
}

void EepromController::init()
{
    // La inicialización real se hace en recuperar()
    recuperar();
}

uint16_t EepromController::_calcularChecksum()
{
    uint16_t checksum = 0;
    byte *p = (byte *)_vgEEprom;
    for (int i = 0; i < (sizeof(TVarEEprom) - sizeof(uint16_t)); i++)
    {
        checksum += p[i];
    }
    return checksum;
}

void EepromController::salvar()
{
    _vgEEprom->clave = CLAVE_VERSION;
    _vgEEprom->checksum = _calcularChecksum();
    EEPROM.put(0, *_vgEEprom);
}

void EepromController::recuperar()
{
    EEPROM.get(0, *_vgEEprom);

    // Comprobar si los datos son válidos
    if (_vgEEprom->clave != CLAVE_VERSION || _vgEEprom->checksum != _calcularChecksum())
    {
        // Si no son válidos, cargar los valores por defecto y guardarlos
        setValoresPorDefecto();
        salvar();
    }
}

void EepromController::setValoresPorDefecto()
{
    memset(_vgEEprom, 0, sizeof(TVarEEprom)); // Limpiar todo primero

    _vgEEprom->clave = CLAVE_VERSION;
    _vgEEprom->idEstacion = ID_ESTACION;
    _vgEEprom->IPE = 1;
    _vgEEprom->count_mode = 1;
    _vgEEprom->PasoHusillo = 1.0;
    _vgEEprom->PasosEncoder = 1.0;

    for (int i = 0; i < ID_MAXIMO_CELULA; i++)
    {
        _vgEEprom->celulas[i].idcel = i;
        _vgEEprom->celulas[i].cap = 1.0;
        _vgEEprom->celulas[i].pol = 1;
        _vgEEprom->celulas[i].res = 1.0;
        _vgEEprom->celulas[i].limite_carga_celP = 1.0;
        _vgEEprom->celulas[i].limite_carga_celN = 1.0;
        _vgEEprom->celulas[i].gainpasostoFPos = 1.0;
        _vgEEprom->celulas[i].gainpasostoFNeg = 1.0;
        _vgEEprom->celulas[i].datarate = DATA_RATE_300;
    }

    _vgEEprom->filtro_on_off = 1;
    _vgEEprom->gainpos = 1.0;
    _vgEEprom->gainneg = 1.0;
    strncpy(_vgEEprom->id_maquina, "NUEVA", 14);
}