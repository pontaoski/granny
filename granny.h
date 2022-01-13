#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// Literals
#define Literal 0x00

// Stack
#define Increment 0x01
#define Decrement 0x02
#define Duplicate 0x03
#define Drop 0x04
#define Swap 0x05

// Toggle between operating on value and return stack
#define Toggle 0x06

// Logic
#define Equals 0x07
#define NotEquals 0x08
#define GreaterThan 0x09
#define LessThan 0x0a
#define Jump 0x0b
#define CondJump 0x0c

// Memory
#define LoadFrom 0x0d
#define StoreTo 0x0e

// Arithmetic
#define Add 0x0f
#define Subtract 0x10
#define Multiply 0x11
#define Divide 0x12

// End
#define Halt 0x13

// IO
#define IOEnum 0x14
#define IOQuery 0x15
#define IOAct 0x16

// Functions
#define Call 0x17
#define Ret 0x18

// More stack
#define Over 0x19

typedef struct {
    uint32_t pointer;
    uint32_t data[511];
} Stack;

typedef struct Granny Granny;

typedef void (*DeviceInvoke)(Granny*, uint16_t*);

#define Handler(name) void name (Granny* granny, uint16_t* pc)

typedef struct {
    uint32_t name;
    uint32_t version;
    DeviceInvoke invoke;
} Device;

struct Granny {
    uint32_t *ram;
    size_t ramSize;

    uint32_t registers[4];

    Stack valuesStack;
    Stack returnStack;

    size_t deviceCount;
    Device* devices;
};

typedef struct Granny Granny;

int
startGranny(Granny *granny, uint16_t programCounter);

void
initGranny(Granny* granny, size_t ramSize);

// #define DBG 1

static inline uint32_t pop(Granny *granny)
{
    if (granny->valuesStack.pointer == 0) {
        printf("underflow!");
    }

    uint32_t value = granny->valuesStack.data[--granny->valuesStack.pointer];

    #ifdef DBG
    printf("popping %d from code\n", value);
    #endif

    return value;
}

static inline void push(Granny *granny, uint32_t value)
{
    if (granny->valuesStack.pointer >= 500) {
        printf("overflow!");
    }

    #ifdef DBG
    printf("pushing %d from code\n", value);
    #endif

    granny->valuesStack.data[granny->valuesStack.pointer++] = (value);
}

static inline char* extractString(Granny *granny, uint32_t at)
{
    static char buf[8192];

    uint32_t currentlyAt = at;
    uint32_t stringLength = 0;

    while (granny->ram[at] && currentlyAt < granny->ramSize)
    {
        buf[stringLength++] = (char)granny->ram[currentlyAt++];
    }
    buf[stringLength] = 0;

    return (char*)buf;
}
