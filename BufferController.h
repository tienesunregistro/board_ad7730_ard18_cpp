#ifndef BUFFERCONTROLLER_H
#define BUFFERCONTROLLER_H

#include "tipos.h"
#include "config.h"

typedef struct _tpDatoCanal
{
    volatile unsigned long secuencia;
    volatile unsigned long t_ms;
    volatile long dato;
} TDatoCanal;

class BufferController {
public:
    BufferController();
    void init();
    void flush();
    bool isDataAvailable();
    bool read(TDatoCanal* dato);
    void store(TDatoCanal* dato);

private:
    TDatoCanal _buffer[DATOS_BUFFER_SIZE];
    volatile unsigned int _head;
    volatile unsigned int _tail;
};

#endif // BUFFERCONTROLLER_H