#pragma once

// Functions that reference uBit, which can only be accessed from one source file:

void updateAccelerometer();
byteVec3 getAccelData();

void serialPrint(const char* str);
void serialPrint(int val);
void serialPrint(float val);
