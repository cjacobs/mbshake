#pragma once

#include "Vector3.h"

// Various helper functions to access the micro:bit instance
void updateAccelerometer();
byteVector3 getAccelData();
bool buttonA();
bool buttonB();
unsigned long systemTime();

void showChar(char ch, unsigned long dur);

void serialPrint(const char* str);
void serialPrint(int val);
void serialPrint(unsigned long val);
void serialPrint(float val);
void serialPrint(const byteVector3& v);
void serialPrint(const floatVector3& v);

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

void panic();
