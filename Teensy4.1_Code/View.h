#include <SPI.h>

class View{
  private:
    #define SPI_CLK   13
    #define SPI_MOSI  11

    const uint8_t Latches[9] =  {22, 21, 20, 19, 18, 17, 16, 15, 14};  // Latches of each shift register group
    const uint8_t Blanks[9] = {41, 40, 39, 38, 37, 36, 35, 34, 33};    // Blank pins of all drivers

    uint8_t data[36] = {};

    uint8_t* Frame;

    uint8_t Layer = 0;

    unsigned long lastFrame = 0;
    uint16_t frameRate = 1000;

    void updateTLC(uint8_t data[36], uint8_t latch) {
      digitalWriteFast(latch, LOW);

      SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
      SPI.transfer(data, 36);
      SPI.endTransaction();

      digitalWriteFast(latch, HIGH);  // latch data
    }

    void shiftOutLayers(uint8_t value) {
        digitalWriteFast(Latches[8], LOW);
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        SPI.transfer(value);
        SPI.endTransaction();
        digitalWriteFast(Latches[8], HIGH);
    }

    void displayLayer(uint8_t* frame, uint8_t Level){
      if (!frame) return;

      Level = 7 - Level;

      for(int i = 0; i < 8; i++){
        memcpy(data, frame + (Level*8+i) * 36, 36);
        updateTLC(data, Latches[i]);
      }
    }

  public:
    View() {
      SPI.begin();
      for(int i = 0; i < 9; i++){
        pinMode(Latches[i], OUTPUT);
        pinMode(Blanks[i], OUTPUT);
        digitalWriteFast(Latches[i], LOW);
        digitalWriteFast(Blanks[i], LOW);
      }
      shiftOutLayers(0);
    }

    void updateCube(){
      if (micros() - lastFrame >= frameRate) {
        lastFrame = micros();
        for(int i = 0; i < 9; i++) digitalWriteFast(Blanks[i], HIGH);
        displayLayer(Frame, Layer);

        shiftOutLayers(1 << Layer);
        for(int i = 0; i < 9; i++) digitalWriteFast(Blanks[i], LOW);
        Layer = (Layer + 1) & 0x07;
      }
    }

    void receiveFrame(uint8_t* frame){
      Frame = frame;
    }

    void setFrameRate(uint16_t fps){
      if (fps < 1) fps = 1;
      frameRate = 1000000UL / (fps * 8);
    }
};