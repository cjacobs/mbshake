#pragma once

#include "vec3.h"
// Functions that reference uBit, which can only be accessed from one source file:

void updateAccelerometer();
byteVec3 getAccelData();
bool buttonA();
bool buttonB();

void serialPrint(const char* str);
void serialPrint(int val);
void serialPrint(float val);
void serialPrint(const byteVec3& v);
void serialPrint(const floatVec3& v);
void serialPrint(const char* label, int val);
void serialPrint(const char* label, float val);
void serialPrint(const char* label, const byteVec3& v);
void serialPrint(const char* label, const floatVec3& v);
