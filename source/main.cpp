#include "vec3.h"
#include "classifiers.h"
#include "MicroBitTouchDevelop.h" // Only 1 source file can include this header

// Constants
const int sampleRate = 18; // in ms
const int eventDisplayPeriod = 250; // ms

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

void serialPrint(const char* str)
{
    printf("%s\r\n", str);
}

void serialPrint(int val)
{
    printf("%d\r\n", val);
}

void serialPrint(float val)
{
    float frac = val - int(val);
    printf("%d.%03d\r\n", int(val), int(1000*frac));
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
  uBit.display.print('S');
}

void onTap(MicroBitEvent)
{
    uBit.display.print('T');
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
