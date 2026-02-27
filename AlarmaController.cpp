#include "AlarmaController.h"

AlarmaController::AlarmaController(int pinAlarmaPost, int pinAlarmaNeg) {
    _pinAlarmaPost = pinAlarmaPost;
    _pinAlarmaNeg = pinAlarmaNeg;
}

void AlarmaController::init() {
    pinMode(_pinAlarmaPost, OUTPUT);
    pinMode(_pinAlarmaNeg, OUTPUT);
    _setAlarmaPost(false);
    _setAlarmaNeg(false);
}

void AlarmaController::_setAlarmaPost(bool on) {
    digitalWrite(_pinAlarmaPost, on ? HIGH : LOW);
}

void AlarmaController::_setAlarmaNeg(bool on) {
    digitalWrite(_pinAlarmaNeg, on ? HIGH : LOW);
}

void AlarmaController::check(long pasos_limite_positivo, long pasos_limite_negativo, long valor_actual) {
    if (pasos_limite_positivo > 0) {
        _setAlarmaPost(valor_actual > pasos_limite_positivo);
    } else {
        _setAlarmaPost(false);
    }

    if (pasos_limite_negativo < 0) {
        _setAlarmaNeg(valor_actual < pasos_limite_negativo);
    } else {
        _setAlarmaNeg(false);
    }
}