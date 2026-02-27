#ifndef CELULACONTROLLER_H
#define CELULACONTROLLER_H

#include "tipos.h" // Para acceder a las estructuras TVarGlobal y TVarEEprom

// Forward declaration para evitar includes circulares
class AD7730Driver;

class CelulaController
{
public:
    // MODIFICAR constructor
    CelulaController(TVarGlobal *vg, TVarEEprom *vgEEprom, AD7730Driver *ad7730);
    void init();
    bool seleccionar(byte id_celula);
    void fuerzaCero();
    long getPasosFromFuerza(float fuerza);
    float getFuerzaFromPasos(long pasos);
    AD7730Driver *getAD7730() { return _ad7730; }

private:
    TVarGlobal *_vg;
    TVarEEprom *_vgEEprom;
    AD7730Driver *_ad7730;
};

#endif
