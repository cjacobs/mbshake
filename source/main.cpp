#include "MicroBitTouchDevelop.h"

void app_main() {
  uBit.display.scroll(ManagedString("Accelerometer dump"));
  while (true) {
    uBit.accelerometer.update();
    printf("%d %d %d\r\n", uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ());
  }
}
