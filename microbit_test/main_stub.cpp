#include "MicroBitAccess.h"
#include "Vector3.h"

// define stubs for functions in microbit main.cpp file

unsigned long systemTime()
{
    static unsigned long currentTime = 0;
    return currentTime += 18;
}

void updateAccelerometer() {}
byteVector3 getAccelData()
{
    return byteVector3();
}

bool buttonA()
{
    return false;
}

bool buttonB()
{
    return false;
}

void showChar(char ch, unsigned long dur) {}
void serialPrint(const char* str) {}
void serialPrint(int val) {}
void serialPrint(unsigned long val) {}
void serialPrint(float val) {}
void serialPrint(const byteVector3& v) {}
void serialPrint(const floatVector3& v) {}
