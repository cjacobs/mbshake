#include "main.h"
#include "vec3.h"

// define stubs for functions in microbit main.cpp file

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
void serialPrint(float val) {}
void serialPrint(const byteVec3& v) {}
void serialPrint(const char* label, int val) {}
void serialPrint(const char* label, float val) {}
void serialPrint(const char* label, const byteVec3& v) {}


