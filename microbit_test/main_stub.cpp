#include "main.h"
#include "vec3.h"

// define stubs for functions in microbit main.cpp file

unsigned long systemTime()
{
    static unsigned long currentTime = 0;
    return currentTime += 18;
}

void updateAccelerometer() {}
byteVec3 getAccelData()
{
    return byteVec3();
}

bool buttonA()
{
    return false;
}

bool buttonB()
{
    return false;
}

void serialPrint(const char* str) {}
void serialPrint(int val) {}
void serialPrint(unsigned long val) {}
void serialPrint(float val) {}
void serialPrint(const byteVec3& v) {}
void serialPrint(const floatVec3& v) {}
