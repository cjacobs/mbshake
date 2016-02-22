#if 0
int main(int argc, char** argv)
{
    return 0;
}

#include "Vector3.h"
#include "FastMath.h"
#include "MicroBitGestureDetector.h"

#include "MicroBitTouchDevelop.h" // Only 1 source file can include this header

// Constants
const int sampleRate = 18; // in ms
const int eventDisplayPeriod = 48; // in ms

// Globals
unsigned long g_turnOffDisplayTime = 0;
unsigned long g_prevTime = 0;

class MyComponent : public MicroBitComponent
{
public:
    MyComponent()
    {
        this->id = 12;
    }

    //    virtual void systemTick()
    //    {
    //    }
};



// 
// Functions declared in MicroBitAccess.h
//

// TODO: put them in a class?

void updateAccelerometer()
{
    uBit.accelerometer.update();
}

byteVector3 getAccelData()
{
    // Actually, I don't think we need the clampByte here
    // TODO: test if clamp was necessary and display something if so, just to make sure
    int x = clampByte(uBit.accelerometer.getX() >> 4);
    int y = clampByte(uBit.accelerometer.getY() >> 4);
    int z = clampByte(uBit.accelerometer.getZ() >> 4);

    return byteVector3(x, y, z);
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

void serialPrint(const byteVector3& v)
{
    printf("%d\t%d\t%d", v.x, v.y, v.z);
}

void serialPrint(const floatVector3& v)
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

void panic()
{
    uBit.panic();
}


// what about this:
MicroBit& GetMicrobit()
{
    return uBit;
}

//
// Local code
//
MicroBitGestureDetector detector;
void accelerometer_poll()
{
    // try scheduling the component thing here
    //    MyComponent test;
    //    uBit.addIdleComponent(&test); // ugh, even here in the fiber, this doesn't work

    while(true)
    {
        detector.systemTick();

        unsigned long time = uBit.systemTime();
        // If enough time has elapsed or the timer rolls over, do something
        if ((time-g_prevTime) >= sampleRate || time < g_prevTime) 
        {
            if (time > g_turnOffDisplayTime)
            {
                uBit.display.clear();
            }

            int detectedGesture = detector.getCurrentGesture();
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

void onButtonA(MicroBitEvent)
{
    detector.togglePrinting();
}

void onButtonB(MicroBitEvent)
{
    detector.toggleAlg();
}

void app_main()
{
    //    uBit.addIdleComponent(&test); // argh! this causes the micro:bit to die
    //    initClassifiers();
    detector.init();

    // create background worker that polls for shake events
    create_fiber(accelerometer_poll);

    // ... and listen for them
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_SHAKE, onShake);
    uBit.MessageBus.listen(MICROBIT_ID_ACCELEROMETER, MICROBIT_ACCELEROMETER_TAP, onTap);
    uBit.MessageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.MessageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);
}
#endif
