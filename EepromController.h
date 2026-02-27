#ifndef EEPROMCONTROLLER_H
#define EEPROMCONTROLLER_H

#include "tipos.h"

class EepromController {
public:
    EepromController(TVarEEprom* vgEEprom);
    void init();
    void salvar();
    void recuperar();
    void setValoresPorDefecto();

private:
    TVarEEprom* _vgEEprom;
    uint16_t _calcularChecksum();
};

#endif // EEPROMCONTROLLER_H