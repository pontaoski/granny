#include "device_file.h"

#define MaxHandles 32

FILE* Handles[MaxHandles + 1];

uint32_t getHandle() {
    for (uint32_t i = 1; i < MaxHandles; i++) {
        if (Handles[i] == 0) {
            return i;
        }
    }
    return 0;
}

// 0: rb
// 1: w
// 2: a
// 3: rb+
Handler(openFile) {
    uint32_t mode = pop(granny);
    const char* filename = extractString(granny, pop(granny));
    uint32_t handle = getHandle();

    if (handle == 0) {
        push(granny, 0);
        return;
    }

    Handles[handle] = NULL;
    if (handle > 0) switch (mode) {
    case 0: Handles[handle] = fopen(filename, "rb"); break;
    case 1: Handles[handle] = fopen(filename, "w"); break;
    case 2: Handles[handle] = fopen(filename, "a"); break;
    case 3: Handles[handle] = fopen(filename, "rb+"); break;
    }

    #ifdef DBG
    if (handle > 0) switch (mode) {
    case 0: printf("handle %d: opened %s with mode rb\n", handle, filename); break;
    case 1: printf("handle %d: opened %s with mode w\n", handle, filename); break;
    case 2: printf("handle %d: opened %s with mode a\n", handle, filename); break;
    case 3: printf("handle %d: opened %s with mode rb+\n", handle, filename); break;
    } else {
    printf("not enough handles!\n");
    }
    #endif

    if (Handles[handle] == NULL)
    {
        Handles[handle] = 0;
        handle = 0;
    }

    push(granny, handle);
}

Handler(closeFile) {
    uint32_t handle = pop(granny);
    fclose(Handles[handle]);
    Handles[handle] = 0;
}

Handler(readFile) {
    uint32_t handle = pop(granny);
    uint32_t c = fgetc(Handles[handle]);

    push(granny, feof(Handles[handle]) ? 0 : c);
}

Handler(writeFile) {
    uint32_t byte = pop(granny);
    uint32_t handle = pop(granny);

    #ifdef DBG
    printf("write '%c' (%d) to %d\n", (char)byte, byte, handle);
    #endif

    fputc(byte, Handles[handle]);
}

Handler(flushFile) {
    uint32_t handle = pop(granny);

    fflush(Handles[handle]);
}

DeviceInvoke FileActions[10] = {
    openFile,
    closeFile,
    readFile,
    writeFile,
    flushFile,
};

Handler(fileHandler) {
    uint32_t device = pop(granny);

    FileActions[device](granny, pc);
}

Device fileDevice()
{
    for (int i = 0; i < MaxHandles + 1; i++) {
        Handles[i] = 0;
    }
    return (Device) { 1, 0, fileHandler };
}
