#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "granny.h"
#include "device_output.h"
#include "device_file.h"

int
main(int argc, char* argv[])
{
    Granny granny;

    Device a[] = { outputDevice(), fileDevice() };

    initGranny(&granny, 2048);

    granny.devices = a;

    if (getenv("GRANNY_IMAGE") != NULL) {
        FILE *fileHandle = fopen(getenv("GRANNY_IMAGE"), "rb");
        size_t fileLength;

        if (fileHandle == NULL) {
            printf("failed to open file %s\n", getenv("GRANNY_IMAGE"));
            exit(1);
        }

        fseek(fileHandle, 0, SEEK_END);
        fileLength = ftell(fileHandle) / sizeof(uint32_t);
        rewind(fileHandle);

        if (fileLength > granny.ramSize) {
            printf("program is %zu blocks, but ram is only %zu blocks!\n", fileLength, granny.ramSize);
            exit(1);
        }

        fread(granny.ram, sizeof(uint32_t), fileLength, fileHandle);
    } else {
        granny.ram[1] = Literal;
        granny.ram[2] = 'z';
        granny.ram[3] = Literal;
        granny.ram[4] = 0;
        granny.ram[5] = IOAct;

        granny.ram[6] = Literal;
        granny.ram[7] = '\n';
        granny.ram[8] = Literal;
        granny.ram[9] = 0;
        granny.ram[10] = IOAct;

        granny.ram[11] = Halt;
    }

    startGranny(&granny, 1);

    return 0;
}