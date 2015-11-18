#include "MicroBitTouchDevelop.h"

// #define USE_GRAVITY 1


#if USE_GRAVITY
struct vec3
{
  float x;
  float y;
  float z;
};

vec3 gravity = {0,0,0};

void filterVec(float x, float y, float z, vec3& prevVec, float alpha)
{
  prevVec.x += alpha*(x-prevVec.x);
  prevVec.y += alpha*(y-prevVec.y);
  prevVec.z += alpha*(z-prevVec.z);
}
#endif

void app_main() {
  //  uBit.display.scroll(ManagedString("Accelerometer dump"));
  unsigned long prevTime = 0;

#if USE_GRAVITY
  uBit.accelerometer.update();
  gravity.x = uBit.accelerometer.getX();
  gravity.y = uBit.accelerometer.getY();
  gravity.z = uBit.accelerometer.getZ();
#endif

  while(true)
  {
    unsigned long time = uBit.systemTime();

    if (time != prevTime)
    {
      uBit.accelerometer.update();

      int buttonAVal = uBit.buttonA.isPressed() ? 1 : 0;
      int buttonBVal = uBit.buttonB.isPressed() ? 1 : 0;
      if(buttonAVal && !buttonBVal)
	uBit.display.print('a');
      else if(buttonBVal && !buttonAVal)
	uBit.display.print('b');
      else if (buttonAVal && buttonBVal)
	uBit.display.print('x');
      else
	uBit.display.clear();

#if USE_GRAVITY
      filterVec(uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ(), gravity, 0.001);
      printf("%ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", time, buttonAVal, buttonBVal, uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ(), (int)gravity.x, (int)gravity.y, (int)gravity.z);
#else
      printf("%ld\t%d\t%d\t%d\t%d\t%d\r\n", time, buttonAVal, buttonBVal, uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ());
#endif

      prevTime = time;
    }
  }
}
