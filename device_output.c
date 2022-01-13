#include <stdio.h>

#include "device_output.h"

void outputHandler(Granny* g, uint16_t* pc)
{
    putc(g->valuesStack.data[--g->valuesStack.pointer], stdout);
}

Device outputDevice()
{
    return (Device) { 0, 0, outputHandler };
}