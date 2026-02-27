#include "BufferController.h"

BufferController::BufferController() {
    // El constructor puede estar vacío, la inicialización se hace en init()
}

void BufferController::init() {
    _head = 0;
    _tail = 0;
}

void BufferController::flush() {
    init();
}

bool BufferController::isDataAvailable() {
    return (_head != _tail);
}

bool BufferController::read(TDatoCanal* dato) {
    if (_head == _tail) {
        return false; // Buffer vacío
    }

    _tail = (_tail + 1) & BUFFER_MASK;
    
    dato->secuencia = _buffer[_tail].secuencia;
    dato->t_ms = _buffer[_tail].t_ms;
    dato->dato = _buffer[_tail].dato;
    
    return true;
}

void BufferController::store(TDatoCanal* dato) {
    unsigned int i = (_head + 1) & BUFFER_MASK;

    if (i == _tail) {
        return; // Buffer lleno, se descarta el dato
    }

    _head = i;
    _buffer[_head].secuencia = dato->secuencia;
    _buffer[_head].t_ms = dato->t_ms;
    _buffer[_head].dato = dato->dato;
}