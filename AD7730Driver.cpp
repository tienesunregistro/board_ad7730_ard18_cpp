#include "AD7730Driver.h"

// Constructor
AD7730Driver::AD7730Driver(int csPin, int rdyPin, int resetPin) : _csPin(csPin),
                                                                  _rdyPin(rdyPin),
                                                                  _resetPin(resetPin),
                                                                  _spiSettings(AD7730_SPI_CLOCK, MSBFIRST, SPI_MODE1)
{
}

// Inicialización y configuración
void AD7730Driver::init(TDataRate dataRate)
{
    configurado = false;
    pinMode(_csPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_rdyPin, INPUT_PULLUP);
    digitalWrite(_csPin, HIGH);

    reset();

    // 1. Configuración del Filtro (Data Rate)
    _sendByte(CR_SINGLE_WRITE | CR_FILTER_REGISTER);
    uint8_t fr2_val;
    switch (dataRate)
    {
    case DATA_RATE_150:
        fr2_val = FR2_DATA_RATE_150;
        break;
    case DATA_RATE_300:
        fr2_val = FR2_DATA_RATE_300;
        break;
    case DATA_RATE_600:
        fr2_val = FR2_DATA_RATE_600;
        break;
    case DATA_RATE_800:
        fr2_val = FR2_DATA_RATE_800;
        break;
    case DATA_RATE_1000:
        fr2_val = FR2_DATA_RATE_1000;
        break;
    case DATA_RATE_1200:
        fr2_val = FR2_DATA_RATE_1200;
        break;
    default:
        fr2_val = FR2_DATA_RATE_300;
        break;
    }
    uint8_t filter_config[] = {fr2_val, FR1_SKIP_OFF | FR1_FAST_ON, FR0_CHOP_OFF};
    _sendBytes(filter_config, 3);
    delay(30);

    // 2. Configuración DAC
    _sendByte(CR_SINGLE_WRITE | CR_DAC_REGISTER);
    _sendByte(DACR_OFFSET_SIGN_POSITIVE | DACR_OFFSET_NONE);
    delay(30);

    // 3. Calibración Interna Zero-Scale
    _sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
    uint8_t cal_zero[] = {MR1_MODE_INTERNAL_ZERO_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS};
    _sendBytes(cal_zero, 2);
    _waitForReady();

    // 4. Calibración Interna Full-Scale
    _sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
    uint8_t cal_full[] = {MR1_MODE_INTERNAL_FULL_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS};
    _sendBytes(cal_full, 2);
    _waitForReady();

    // 5. Calibración Sistema Zero-Scale (presente en el original)
    _sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
    uint8_t cal_sys_zero[] = {MR1_MODE_SYSTEM_ZERO_CALIBRATION | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS};
    _sendBytes(cal_sys_zero, 2);
    _waitForReady();

    // 6. Modo Continuo (operación normal)
    _sendByte(CR_SINGLE_WRITE | CR_MODE_REGISTER);
    uint8_t continuous_mode[] = {MR1_MODE_CONTINUOUS | CURRENT_MODE_1_SETTINGS, CURRENT_MODE_0_SETTINGS};
    _sendBytes(continuous_mode, 2);
    _waitForReady();

    // NOTA: configurado se establece desde setup() después de attachInterrupt
    // para evitar que la ISR lea el ADC antes de estar listo
}

void AD7730Driver::reset()
{
    digitalWrite(_resetPin, HIGH);
    delay(10);
    digitalWrite(_resetPin, LOW);
    delay(10);
    digitalWrite(_resetPin, HIGH);
    delay(100);
}

// Métodos de comunicación SPI
void AD7730Driver::_sendByte(uint8_t toSend)
{
    _sendBytes(&toSend, 1);
}

void AD7730Driver::_sendBytes(const uint8_t *data, uint8_t len)
{
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    for (uint8_t i = 0; i < len; i++)
    {
        SPI.transfer(data[i]);
    }
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
}

bool AD7730Driver::_waitForReady()
{
    int32_t timeout = AD7730_TIMEOUT;
    while (digitalRead(_rdyPin) == HIGH && timeout-- > 0)
        ;
    return (timeout > 0);
}

// Lectura de datos
long AD7730Driver::_readConversionData()
{
    if (!configurado)
    {
        if (!_waitForReady())
            return _ultimoValorValido;
    }

    _sendByte(CR_SINGLE_READ | CR_DATA_REGISTER);

    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    uint8_t b1 = SPI.transfer(0);
    uint8_t b2 = SPI.transfer(0);
    uint8_t b3 = SPI.transfer(0);
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();

    int32_t lDat = ((long)b1 << 16) | ((long)b2 << 8) | b3;
    lDat >>= AD7730_SHIFT_BITS;

    if (lDat & AD7730_18BIT_MASK)
    {
        lDat -= AD7730_18BIT_RANGE;
    }
    return lDat;
}

long AD7730Driver::leerDatoConFiltro()
{
    long dataconvert = _readConversionData();
    static int rejectionCounter = 0;

    long difference = abs(dataconvert - _ultimoValorValido);

    if (difference > AD7730_GLITCH_THRESHOLD)
    {
        rejectionCounter++;
        if (rejectionCounter > 5)
        {
            rejectionCounter = 0;
            _ultimoValorValido = dataconvert;
        }
        else
        {
            dataconvert = _ultimoValorValido;
        }
    }
    else
    {
        rejectionCounter = 0;
        _ultimoValorValido = dataconvert;
    }
    return dataconvert;
}

// Manejo de interrupciones
void AD7730Driver::setIsrFlag()
{
    _isrFlag = true;
}

bool AD7730Driver::getIsrFlag()
{
    return _isrFlag;
}

void AD7730Driver::clearIsrFlag()
{
    _isrFlag = false;
}

void AD7730Driver::handleInterrupt()
{
}

// --- Métodos ISR (con SPI.beginTransaction como el original) ---

void AD7730Driver::_sendByteISR(uint8_t toSend)
{
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    SPI.transfer(toSend);
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
}

long AD7730Driver::_readConversionDataISR()
{
    if (!configurado)
    {
        return _ultimoValorValido;
    }

    // Seleccionar lectura del registro de datos
    _sendByteISR(CR_SINGLE_READ | CR_DATA_REGISTER);

    // Leer 3 bytes (24 bits)
    SPI.beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    uint8_t b1 = SPI.transfer(0);
    uint8_t b2 = SPI.transfer(0);
    uint8_t b3 = SPI.transfer(0);
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();

    int32_t lDat = ((long)b1 << 16) | ((long)b2 << 8) | b3;
    lDat >>= AD7730_SHIFT_BITS;

    if (lDat & AD7730_18BIT_MASK)
    {
        lDat -= AD7730_18BIT_RANGE;
    }
    return lDat;
}

long AD7730Driver::leerDatoConFiltroISR()
{
    long dataconvert = _readConversionDataISR();
    static int rejectionCounter = 0;

    long difference = abs(dataconvert - _ultimoValorValido);

    if (difference > AD7730_GLITCH_THRESHOLD)
    {
        rejectionCounter++;
        if (rejectionCounter > 5)
        {
            rejectionCounter = 0;
            _ultimoValorValido = dataconvert;
        }
        else
        {
            dataconvert = _ultimoValorValido;
        }
    }
    else
    {
        rejectionCounter = 0;
        _ultimoValorValido = dataconvert;
    }
    return dataconvert;
}