#include "vec3.h"
#include "fastmath.h"
#include "classifiers.h"
#include "MicroBitTouchDevelop.h" // Only 1 source file can include this header
#include "main.h"

// Constants
const int sampleRate = 18; // in ms
const int eventDisplayPeriod = 48; // in ms

// Globals
unsigned long g_prevTime = 0;
unsigned long g_turnOffDisplayTime = 0;

// Exported functions that access uBit
void updateAccelerometer()
{
    uBit.accelerometer.update();
}

byteVec3 getAccelData()
{
    // Actually, I don't think we need the clampByte here
    // TODO: test if clamp was necessary and display something if so, just to make sure
    int x = clampByte(uBit.accelerometer.getX() >> 4);
    int y = clampByte(uBit.accelerometer.getY() >> 4);
    int z = clampByte(uBit.accelerometer.getZ() >> 4);

    return byteVec3(x, y, z);
}

bool buttonA()
{
    return uBit.buttonA.isPressed();
}

bool buttonB()
{
    return uBit.buttonB.isPressed();
}

void showChar(char ch, unsigned long dur)
{
    unsigned long time = uBit.systemTime();
    uBit.display.print(ch);

    g_turnOffDisplayTime = time + dur;
}

void printFloat(float val)
{
    char minus[3] = "\0\0";
    if(val < 0)
    {
        minus[0] = '-';
        val = -val;
    }
    float frac = val - int(val);
    printf("%s%d.%03d", minus, int(val), int(1000*frac));
}

void serialPrint(const char* str)
{
    printf("%s", str);
}

void serialPrint(int val)
{
    printf("%d", val);
}

void serialPrint(unsigned long val)
{
    printf("%lu", val);
}

void serialPrint(float val)
{
    char minus[3] = "\0\0";
    if(val < 0)
    {
        minus[0] = '-';
        val = -val;
    }
    float frac = val - int(val);
    printf("%s%d.%03d", minus, int(val), int(1000*frac));
}

void serialPrint(const byteVec3& v)
{
    printf("%d\t%d\t%d", v.x, v.y, v.z);
}

void serialPrint(const floatVec3& v)
{
    printFloat(v.x);
    printf("\t");
    printFloat(v.y);
    printf("\t");
    printFloat(v.z);
}

unsigned long systemTime()
{
    return uBit.systemTime();
}

// Local code
void accelerometer_poll()
{
    while(true)
    {
        unsigned long time = uBit.systemTime();
        // If enough time has elapsed or the timer rolls over, do something
        if ((time-g_prevTime) >= sampleRate || time < g_prevTime) 
        {
            if (time > g_turnOffDisplayTime)
            {
                uBit.display.clear();
            }

            int detectedGesture = detectGesture();
            if(detectedGesture != 0)
            {
                g_turnOffDisplayTime = time + eventDisplayPeriod;

                // The event constructor has the side-effect of dispatching the event onto the message bus
                MicroBitEvent(MICROBIT_ID_ACCELEROMETER, detectedGesture);
            }
            g_prevTime = time;
        }
        uBit.sleep(1);
    }
}

// Event handlers
void onShake(MicroBitEvent)
{
  uBit.display.print('#');
}

void onTap(MicroBitEvent)
{
    uBit.display.print('.');
}

void app_main()
{
    initClassifiers();

    // create background worker that polls for shake events
    create_fiber(accelerometer_poll);

    // ... and listen for them
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_SHAKE, onShake);
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_TAP, onTap);
}
