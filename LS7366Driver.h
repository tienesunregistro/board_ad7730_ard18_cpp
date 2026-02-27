#ifndef LS7366DRIVER_H
#define LS7366DRIVER_H

#include <SPI.h>
#include "config.h"

class LS7366Driver
{
public:
    LS7366Driver(int csPin);
    void init(byte mode);
    void reset();
    void cero();
    long leer();

private:
    int _csPin;
    SPISettings _spiSettings;

    void _writeReg(byte reg, byte value);
    long _readReg(byte reg);
};

#endif // LS7366DRIVER_H