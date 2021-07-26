#include "main.h"
#include "Metronome.h"
#include "TouchChannel.h"
#include "GlobalControl.h"
#include "Degrees.h"
#include "MPR121.h"
#include "MIDI.h"
#include "DAC8554.h"
#include "MCP23017.h"
#include "SX1509.h"
#include "MCP23008.h"
#include "VCOCalibrator.h"

int OCTAVE_LED_PINS_A[4] = { 0, 1, 2, 3 };     // via TLC59116
int OCTAVE_LED_PINS_B[4] = { 4, 5, 6, 7 };     // via TLC59116
int OCTAVE_LED_PINS_C[4] = { 8, 9, 10, 11 };   // via TLC59116
int OCTAVE_LED_PINS_D[4] = { 12, 13, 14, 15 }; // via TLC59116

I2C i2c1(I2C1_SDA, I2C1_SCL);
I2C i2c3(I2C3_SDA, I2C3_SCL);

DigitalOut globalGate(GLOBAL_GATE_OUT);
Ticker ticker;
Timer timer;
MIDI midi(MIDI_TX, MIDI_RX);
InterruptIn extClockInput(EXT_CLOCK_INPUT);

DAC8554 dac1(SPI2_MOSI, SPI2_SCK, DAC1_CS);
DAC8554 dac2(SPI2_MOSI, SPI2_SCK, DAC2_CS);
MCP23017 io(&i2c3, MCP23017_DEGREES_ADDR);

SX1509 ioA(&i2c3, SX1509_CHAN_A_ADDR);
SX1509 ioB(&i2c3, SX1509_CHAN_B_ADDR);
SX1509 ioC(&i2c3, SX1509_CHAN_C_ADDR);
SX1509 ioD(&i2c3, SX1509_CHAN_D_ADDR);

MPR121 touchPadA(&i2c1, TOUCH_INT_A);
MPR121 touchPadB(&i2c1, TOUCH_INT_B, MPR121::ADDR_VDD);
MPR121 touchPadC(&i2c1, TOUCH_INT_C, MPR121::ADDR_SCL);
MPR121 touchPadD(&i2c1, TOUCH_INT_D, MPR121::ADDR_SDA);

Degrees degrees(DEGREES_INT, &io);

TouchChannel channelA(0, &timer, &ticker, &globalGate, GATE_OUT_A, IO_INT_PIN_A, ADC_A, PB_ADC_A, &touchPadA, &ioA, &degrees, &midi, &dac1, DAC8554::CHAN_A, &dac2, DAC8554::CHAN_A);
TouchChannel channelB(1, &timer, &ticker, &globalGate, GATE_OUT_B, IO_INT_PIN_B, ADC_B, PB_ADC_B, &touchPadB, &ioB, &degrees, &midi, &dac1, DAC8554::CHAN_B, &dac2, DAC8554::CHAN_B);
TouchChannel channelC(2, &timer, &ticker, &globalGate, GATE_OUT_C, IO_INT_PIN_C, ADC_C, PB_ADC_C, &touchPadC, &ioC, &degrees, &midi, &dac1, DAC8554::CHAN_C, &dac2, DAC8554::CHAN_C);
TouchChannel channelD(3, &timer, &ticker, &globalGate, GATE_OUT_D, IO_INT_PIN_D, ADC_D, PB_ADC_D, &touchPadD, &ioD, &degrees, &midi, &dac1, DAC8554::CHAN_D, &dac2, DAC8554::CHAN_D);

Metronome metronome(TEMPO_LED, TEMPO_POT, INT_CLOCK_OUTPUT, PPQN, DEFAULT_SEQ_LENGTH);

GlobalControl globalCTRL(&metronome, &degrees, &i2c1, &channelA, &channelB, &channelC, &channelD);

int main() {
  i2c1.frequency(400000);
  i2c3.frequency(400000);
  
  timer.start();

  channelA.init();
  channelB.init();
  channelC.init();
  channelD.init();

  globalCTRL.init();
  globalCTRL.loadCalibrationDataFromFlash();

  while(1) {
    globalCTRL.poll();
  }
}


// NOTE: You may be able to create a seperate "thread" via the Thread api for handling the event loop