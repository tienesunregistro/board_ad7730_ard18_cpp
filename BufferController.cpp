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
    // Si head == tail, el buffer está vacío
    if (_head == _tail) {
        return false;
    }

    // Desactivamos interrupciones solo durante la copia para que la ISR
    // no mueva el _tail mientras estamos leyendo.
    uint8_t sreg = SREG; // Guardar estado de interrupciones
    noInterrupts();

    unsigned int targetIndex = (_tail + 1) & BUFFER_MASK;
    
    dato->secuencia = _buffer[targetIndex].secuencia;
    dato->t_ms = _buffer[targetIndex].t_ms;
    dato->dato = _buffer[targetIndex].dato;
    
    _tail = targetIndex;

    SREG = sreg; // Restaurar estado de interrupciones
    return true;
}

// bool BufferController::read(TDatoCanal* dato) {
//     if (_head == _tail) {
//         return false; // Buffer vacío
//     }

//     _tail = (_tail + 1) & BUFFER_MASK;
    
//     dato->secuencia = _buffer[_tail].secuencia;
//     dato->t_ms = _buffer[_tail].t_ms;
//     dato->dato = _buffer[_tail].dato;
    
//     return true;
// }

void BufferController::store(TDatoCanal* dato) {
    unsigned int nextHead = (_head + 1) & BUFFER_MASK;

    // Si la cabeza va a alcanzar a la cola, significa que el buffer está lleno
    if (nextHead == _tail) {
        // "Empujamos" la cola un paso adelante. 
        // Esto descarta el dato más antiguo y hace espacio para el nuevo.
        _tail = (_tail + 1) & BUFFER_MASK;
    }

    // Guardamos el dato en la posición de la cabeza
    _buffer[nextHead].secuencia = dato->secuencia;
    _buffer[nextHead].t_ms = dato->t_ms;
    _buffer[nextHead].dato = dato->dato;

    // Actualizamos la cabeza
    _head = nextHead;
}

// void BufferController::store(TDatoCanal* dato) {
//     unsigned int i = (_head + 1) & BUFFER_MASK;

//     if (i == _tail) {
//         return; // Buffer lleno, se descarta el dato
//     }

//     _head = i;
//     _buffer[_head].secuencia = dato->secuencia;
//     _buffer[_head].t_ms = dato->t_ms;
//     _buffer[_head].dato = dato->dato;
// }