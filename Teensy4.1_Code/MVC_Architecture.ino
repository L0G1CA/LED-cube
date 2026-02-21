#include "Model.h"
#include "View.h"
#include "WebInterface.h"
#include "Controller.h"

Model model;
View view;
WebInterface webPage;
Controller controller(&model, &view, &webPage);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);  // timeout if no USB
}

void loop() {
  controller.Update();
}