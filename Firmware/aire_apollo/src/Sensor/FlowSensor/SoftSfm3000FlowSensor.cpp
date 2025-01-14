#include "SoftSfm3000FlowSensor.h"
#include <Arduino.h>

/*
#define I2C_PULLUP 1
//#define I2C_TIMEOUT 1000
#define SDA_PORT PORTC
#define SDA_PIN 3
#define SCL_PORT PORTC
#define SCL_PIN 4
//#define I2C_FASTMODE 1
#define I2C_7BITADDR 0x40
#define MEMLOC 0x0A
#define ADDRLEN 1
*/

//#include <SoftWire.h>



SoftwareWire softWire(A5,A9);

  SoftSfm3000FlowSensor::SoftSfm3000FlowSensor(uint8_t sampling_ms, uint8_t addr) : Sfm3000FlowSensor(sampling_ms,addr)
  {
    _lastSampleTime = 0;
    _volSinceReset  = 0;
  }

  bool SoftSfm3000FlowSensor::begin()
  {

    softWire.begin();
    //delay(100);      // give the sensor time to start after power up
    //softReset();
    //readScale();  // Read scale facor and read offset from sensor
    //readOffset();
    //startFlowMeasurement();
    softWire.beginTransmission(0x40);
    softWire.write((uint8_t)0x10); // start continuous measurement
    softWire.write((uint8_t)0x00); // command 0x1000
    softWire.endTransmission();
    _flowOffset  = 32768;
    _scaleFactor = 800;
    resetFlow();
    delay(10);
    readBytes();    // first read is always invalid
    delay(1);

    return true;
  }


/**
   * @brief Write 2 byte command to the sensor
   *
   * @param command 2 byte command
   * @return uint8_t 0 if success
   */
  uint8_t SoftSfm3000FlowSensor::sendCommand(uint16_t command)
  {
    uint8_t status = 0;

    softWire.beginTransmission(_addr);
    softWire.write(command >> 8);
    softWire.write(command  && 0xFF);
    status = softWire.endTransmission();

    return status;
  }

  /**
   * @brief Read sizeof(_data) bytes from sensor and store values in _data
   *
   * @return uint8_t number of bytes readed or -1 is error
   */
  int SoftSfm3000FlowSensor::readBytes()
  {
    int bytesReaded = 0;

    bytesReaded = softWire.requestFrom(_addr, sizeof(_data));

    if(bytesReaded != sizeof(_data))
      return -1;

    for(int i = 0; i<bytesReaded; i++)
      _data[i] = softWire.read();

    if (softWire.endTransmission() != 0)
      return -1;

    uint8_t crc = 0;
    crc = CRC(_data[0], crc);
    crc = CRC(_data[1], crc);

    if (crc != _data[2])
      return -1;             // TODO find the best value to incate error, flow may be negative

    return bytesReaded;
  }
