#ifndef ALARMACONTROLLER_H
#define ALARMACONTROLLER_H

#include <Arduino.h>

class AlarmaController
{
public:
    AlarmaController(int pinAlarmaPost, int pinAlarmaNeg);
    void init();
    void check(long pasos_limite_positivo, long pasos_limite_negativo, long valor_actual);

private:
    int _pinAlarmaPost;
    int _pinAlarmaNeg;
    void _setAlarmaPost(bool on);
    void _setAlarmaNeg(bool on);
};

#endif