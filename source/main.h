#pragma once

#include "vec3.h"
// Functions that reference uBit, which can only be accessed from one source file:

void updateAccelerometer();
byteVec3 getAccelData();
bool buttonA();
bool buttonB();
unsigned long systemTime();

void serialPrint(const char* str);
void serialPrint(int val);
void serialPrint(unsigned long val);
void serialPrint(float val);
void serialPrint(const byteVec3& v);
void serialPrint(const floatVec3& v);

template <typename FirstArg, typename... RestArgs>
struct serialPrinter
{
    static void serialPrintLn(FirstArg first, RestArgs ...rest)
    {
        ::serialPrint(first);
        serialPrinter<RestArgs...>::serialPrintLn(rest...);
    }
};

template <typename FirstArg>
struct serialPrinter<FirstArg>
{
    static void serialPrintLn(FirstArg first)
    {
        ::serialPrint(first);
        ::serialPrint("\r\n");
    }
};

template <typename... Args>
void serialPrintLn(Args ...args)
{
    serialPrinter<Args...>::serialPrintLn(args...);
}

