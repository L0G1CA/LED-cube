class Controller{
  private:
    Model* model;
    View* view;
    WebInterface* webPage;

    uint16_t fps = 20;
    unsigned long lastFrame = 0;

    bool paused = false;

    void handleWebPage(){
      processInput(webPage->readCommand());
    }

    void processInput(String Command){
      if (Command == "") return;
      // Serial.println(Command); // just for debugging

      if (Command == "LIST"){
        auto files = model->listContent();
        for (const auto& file : files) {
          webPage->println(file.name + "," + String(file.size));
        }
        webPage->println("END");
      }

      if (Command.startsWith("WRITE:")){
        String filename = Command.substring(6);
        bool success = model->writeFile(filename, webPage->getStream());

        if (success) {
          webPage->println("OK");
        } else {
          webPage->println("ERROR: Write failed or timed out");
        }
      }
      if (Command.startsWith("PLAY:")) {
        String filename = Command.substring(5);
        model->setAnimation(filename);
        paused = false;
      }
      if (Command.startsWith("PLAY/PAUSE:")) {
        String state = Command.substring(sizeof("PLAY/PAUSE:") - 1);
        state.trim();
        if (state == "PLAY") {
          paused = false;
        } else if (state == "PAUSE") {
          paused = true;
        }
      }
      if (Command.startsWith("DELETE:")) {
        String filename = Command.substring(7);
        filename.trim();
        bool success = model->deleteFile(filename);
        if (success) {
          webPage->println("OK: File deleted");
        } else {
          webPage->println("ERROR: Could not delete file");
        }
      }
      if (Command == "STORAGE") {
            uint32_t blockSize = 512;

            uint64_t totalBlocks = model->getSize();
            uint64_t freeBlocks = model->getStorage();

            uint64_t totalBytes = totalBlocks * blockSize;
            uint64_t freeBytes = freeBlocks * blockSize;
            uint64_t usedBytes = totalBytes - freeBytes;

            webPage->print("TOTAL,");
            webPage->println(totalBytes);
            webPage->print("USED,");
            webPage->println(usedBytes);
            webPage->println("END");
      }
    }
    
  public:
    Controller(Model* model, View* view, WebInterface* webPage):
      model(model), view(view), webPage(webPage) {
        // model->setAnimation("dimensions.bin");   // Initial animation to be loaded
        view->setFrameRate(160);                // Refresh rate of the cube itself
        setFPS(50);                              // FPS of the aniamtions
      }

    void Update(){
      handleWebPage();
      view->updateCube();
      if (!paused && millis() - lastFrame >= fps) {
        lastFrame = millis();
        view->receiveFrame(model->getFrame());
      }
    }

    void setFPS(uint16_t FPS){
      if (FPS < 1) FPS = 1;
      fps = 1000UL / FPS;
    }
};