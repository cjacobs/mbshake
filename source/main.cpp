#include "MicroBitTouchDevelop.h"
#include "vec3.h"

floatVec3 gravity = {0,0,0};

void filterVec(float x, float y, float z, floatVec3& prevVec, float alpha)
{
    prevVec.x += alpha*(x-prevVec.x);
    prevVec.y += alpha*(y-prevVec.y);
    prevVec.z += alpha*(z-prevVec.z);
}

void app_main()
{
    unsigned long prevTime = 0;

    const bool useGravity = false;

    if(useGravity)
    {
        uBit.accelerometer.update();
        gravity.x = uBit.accelerometer.getX();
        gravity.y = uBit.accelerometer.getY();
        gravity.z = uBit.accelerometer.getZ();
    }
    
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

            if(useGravity)
            {
                filterVec(uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ(), gravity, 0.001);
                printf("%ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", time, buttonAVal, buttonBVal, uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ(), (int)gravity.x, (int)gravity.y, (int)gravity.z);
            }
            else
            {
                printf("%ld\t%d\t%d\t%d\t%d\t%d\r\n", time, buttonAVal, buttonBVal, uBit.accelerometer.getX(), uBit.accelerometer.getY(), uBit.accelerometer.getZ());
            }

            prevTime = time;
        }
    }
}
