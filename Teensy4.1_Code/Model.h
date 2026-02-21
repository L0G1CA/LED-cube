#include "SdFat.h"
#include <vector>
using std::vector;

class Model{
  private:
    SdFs sd;
    FsFile animationFile;

    uint8_t bufferA[2304];
    uint8_t bufferB[2304];
    uint8_t* currentBuffer = bufferA;
    uint8_t* nextBuffer = bufferB;
    int frameIndex = 0;
    int totalFrames = 0;

    struct FileEntry {
      String name;
      uint32_t size;
    };

    void loadNextFrame() {
      if (!animationFile) {
        Serial.println("Error: no file to read");
        return;
      }
      // Swap buffers
      uint8_t* temp = currentBuffer;
      currentBuffer = nextBuffer;
      nextBuffer = temp;

      // Read packed frame into temporary buffer
      animationFile.seek(frameIndex * 2304);
      size_t bytesRead = animationFile.read(nextBuffer, 2304);

      if (bytesRead != 2304) {
        Serial.println("Frame read error");
        memset(nextBuffer, 0, 2304);  // optional fallback
      }

      frameIndex = (frameIndex + 1) % totalFrames;
    }

  public:
    Model(){
      if (!sd.begin(SdioConfig(FIFO_SDIO))) {
        Serial.println("sd card initialization failed");
        return;
      }
    }

    uint8_t* getFrame(){
      loadNextFrame();
      return currentBuffer;
    }

    void setAnimation(const String& fileName){
      if (animationFile) animationFile.close();

      animationFile = sd.open(fileName);
      if (!animationFile) {
        Serial.println("Failed to open animation file");
        return;
      }

      frameIndex = 0;
      totalFrames = animationFile.size() / 2304;
      loadNextFrame();
    }

    vector<FileEntry> listContent(){
      vector<FileEntry> files;
      SdFile root;
      if (!root.open("/")) return files;

      SdFile entry;
      while (entry.openNext(&root, O_READ)) {
        if (!entry.isDir()) {
          char name[32];
          entry.getName(name, sizeof(name));
          if (strcmp(name, "waitlist.txt") == 0) {
            entry.close();
            continue;
          }
          FileEntry f;
          f.name = String(name);
          f.size = entry.fileSize();
          files.push_back(f);
        }
        entry.close();
      }
      root.close();
      return files;
    }
    
    bool writeFile(const String& filename, Stream& stream, unsigned long timeoutMs = 10000) {
      FsFile file = sd.open(filename.c_str(), O_WRITE | O_CREAT | O_TRUNC);
      if (!file) return false;

      const size_t bufferSize = 2048;  // or 1024 if you have RAM for it
      uint8_t buffer[bufferSize];
      size_t index = 0;

      const int endMarkerLen = 8;
      char endMarker[endMarkerLen + 1] = {0};
      unsigned long timeout = millis() + timeoutMs;

      while (millis() < timeout) {
        while (stream.available()) {
          char c = stream.read();
          timeout = millis() + timeoutMs;

          // Check for ENDWRITE
          memmove(endMarker, endMarker + 1, endMarkerLen - 1);
          endMarker[endMarkerLen - 1] = c;
          endMarker[endMarkerLen] = '\0';

          if (strcmp(endMarker, "ENDWRITE") == 0) {
            // Backtrack: remove the ENDWRITE bytes from buffer
            index -= (endMarkerLen - 1);  // Already wrote 7 valid bytes
            file.write(buffer, index);
            file.close();
            return true;
          }

          buffer[index++] = c;
          if (index == bufferSize) {
            file.write(buffer, bufferSize);
            index = 0;
          }
        }
      }

      // Flush remaining data if ENDWRITE wasn't found (fail-safe)
      if (index > 0) {
        file.write(buffer, index);
      }

      file.close();
      return false;  // timeout hit, no ENDWRITE found
    }

    bool deleteFile(const String& filename){
      if (sd.exists(filename.c_str())) {
        return sd.remove(filename.c_str());
      }
      return false;
    }

    uint64_t getStorage(){
      uint32_t blocksPerCluster = sd.vol()->sectorsPerCluster();
      uint32_t freeClusters = sd.vol()->freeClusterCount();
      return (uint64_t)freeClusters * blocksPerCluster;
    }

    uint64_t getSize(){
      uint32_t blocksPerCluster = sd.vol()->sectorsPerCluster();
      uint32_t clusterCount = sd.vol()->clusterCount();
      return (uint64_t)clusterCount * blocksPerCluster;
    }
};