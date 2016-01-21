#include "vec3.h"
#include "classifiers.h"
#include "MicroBitTouchDevelop.h" // Only 1 source file can include this header

// Constants
const int sampleRate = 18; // in ms
const int eventDisplayPeriod = 18; //250; // ms

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
    return byteVec3(uBit.accelerometer.getX()>>4,
                    uBit.accelerometer.getY()>>4,
                    uBit.accelerometer.getZ()>>4);
}

bool buttonA()
{
    return uBit.buttonA.isPressed();
}

bool buttonB()
{
    return uBit.buttonB.isPressed();
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
    printf("%s\r\n", str);
}

void serialPrint(const char* label, int val)
{
    printf("%s%d\r\n", label, val);
}

void serialPrint(int val)
{
    printf("%d\r\n", val);
}

void serialPrint(const char* label, float val)
{
    char minus[3] = "\0\0";
    if(val < 0)
    {
        minus[0] = '-';
        val = -val;
    }
    float frac = val - int(val);
    printf("%s%s%d.%03d\r\n", label, minus, int(val), int(1000*frac));
}

void serialPrint(float val)
{
    serialPrint("", val);
}

void serialPrint(const char* label, const byteVec3& v)
{
    printf("%s%d\t%d\t%d\r\n", label, v.x, v.y, v.z);
}

void serialPrint(const byteVec3& v)
{
    printf("%d\t%d\t%d\r\n", v.x, v.y, v.z);
}

void serialPrint(const floatVec3& v)
{
    printFloat(v.x);
    printf("\t");
    printFloat(v.y);
    printf("\t");
    printFloat(v.z);
    printf("\r\n");
}

void serialPrint(const char* label, const floatVec3& v)
{
    printf("%s", label);
    printFloat(v.x);
    printf("\t");
    printFloat(v.y);
    printf("\t");
    printFloat(v.z);
    printf("\r\n");
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
