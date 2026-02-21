#include <SoftwareSerial.h>

class WebInterface{
  private:
    SoftwareSerial espSerial;

  public:
    WebInterface() : espSerial(7, 8) {
      espSerial.begin(1000000);
    }
    
    String readCommand() {
      if (espSerial.available()) return espSerial.readStringUntil('\n').trim();
      return "";
    }

    Stream& getStream() {
      return espSerial;
    }

    void println(const String& s) {
      espSerial.println(s);
    }

    void print(const String& s) {
      espSerial.print(s);
    }
};
