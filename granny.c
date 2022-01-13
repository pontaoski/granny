#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "granny.h"

int
startGranny(Granny *granny, uint16_t programCounter)
{
    uint32_t a = 0;
    uint32_t b = 0;

#ifdef DBG
    static const char* dispatch_name_table[] = {
        "Literal",
        "Increment",
        "Decrement",
        "Duplicate",
        "Drop",
        "Swap",
        "Toggle",
        "Equals",
        "NotEquals",
        "GreaterThan",
        "LessThan",
        "Jump",
        "CondJump",
        "LoadFrom",
        "StoreTo",
        "Add",
        "Subtract",
        "Multiply",
        "Divide",
        "Halt",
        "IOEnum",
        "IOQuery",
        "IOAct",
        "Call",
        "Ret",
        "Over",
    };
#endif

    static void* dispatch_table[] = {
        &&_Literal,
        &&_Increment,
        &&_Decrement,
        &&_Duplicate,
        &&_Drop,
        &&_Swap,
        &&_Toggle,
        &&_Equals,
        &&_NotEquals,
        &&_GreaterThan,
        &&_LessThan,
        &&_Jump,
        &&_CondJump,
        &&_LoadFrom,
        &&_StoreTo,
        &&_Add,
        &&_Subtract,
        &&_Multiply,
        &&_Divide,
        &&_Halt,
        &&_IOEnum,
        &&_IOQuery,
        &&_IOAct,
        &&_Call,
        &&_Ret,
        &&_Over,
    };

    Stack* targetStack = &granny->valuesStack;
    Stack* otherStack = &granny->returnStack;
    Stack* valueStack = &granny->valuesStack;
    Stack* returnStack = &granny->returnStack;
    Stack* swap = NULL;

    #ifdef DBG
    #define PrintDispatch { printf("pc%d ram%d ", programCounter, granny->ram[programCounter]); printf("opcode '%s' ", dispatch_name_table[granny->ram[programCounter]]); }
    #else
    #define PrintDispatch
    #endif

    #define ProgramCounterBoundsCheck if (programCounter >= granny->ramSize-1) { printf("reached end!\n"); return targetStack->data[targetStack->pointer]; }

    #define DISPATCH() ProgramCounterBoundsCheck programCounter++; PrintDispatch; goto *dispatch_table[granny->ram[programCounter]]

    #ifdef DBG
    #define PrintStackValue(reason, reason2, stack) {\
        printf("stack size %d %s %s: start <", stack->pointer, reason, reason2); \
        for (uint32_t i = 0; i < stack->pointer; i++) {\
            printf("%d ", stack->data[i]);\
        }\
        printf("> end\n");\
    }
    #else
    #define PrintStackValue(reason, reason2, stack)
    #endif
    #define Push(stack, why, val) if (stack->pointer >= 500) { printf("overflow! (%d) pc=%d\n", __LINE__, programCounter); } stack->data[stack->pointer++] = (val); PrintStackValue("after push", why, stack)
    #define Pop(stack, why, var) if (stack->pointer == 0) { printf("underflow! (%d) pc=%d\n", __LINE__, programCounter); } var = stack->data[--stack->pointer]; PrintStackValue("after pop", why, stack);
    #define Peek(stack) stack->data[stack->pointer - 1]
    #define PeekOffset(stack, offset) stack->data[stack->pointer - 1 - offset]

    programCounter--;
    DISPATCH();
    while (1) {
    _Literal:
        Push(targetStack, "literal", granny->ram[++programCounter])
        DISPATCH();
    _Increment:
        Pop(targetStack, "take increment", a);
        Push(targetStack, "give increment", a + 1);
        DISPATCH();
    _Decrement:
        Pop(targetStack, "take decrement", a);
        Push(targetStack, "give decrement", a - 1);
        DISPATCH();
    _Duplicate:
        a = Peek(targetStack);
        Push(targetStack, "duplicate", a);
        DISPATCH();
    _Over:
        a = PeekOffset(targetStack, 1);
        Push(targetStack, "over", a);
        DISPATCH();
    _Drop:
        Pop(targetStack, "drop", a);
        DISPATCH();
    _Swap:
        Pop(targetStack, "take swap a", a);
        Pop(targetStack, "take swap b", b);
        Push(targetStack, "give swap a", a);
        Push(targetStack, "give swap b", b);
        DISPATCH();
    _Toggle:
        swap = targetStack;
        targetStack = otherStack;
        otherStack = swap;
        DISPATCH();
    _Equals:
        Pop(targetStack, "equals take a", a);
        Pop(targetStack, "equals take b", b);
        Push(targetStack, "equals value", a == b);
        DISPATCH();
    _NotEquals:
        Pop(targetStack, "not equals take a", a);
        Pop(targetStack, "not equals take b", b);
        Push(targetStack, "not equals value", a != b);
        DISPATCH();
    _GreaterThan:
        Pop(targetStack, "", a);
        Pop(targetStack, "", b);
        Push(targetStack, "", b > a);
        DISPATCH();
    _LessThan:
        Pop(targetStack, "", a);
        Pop(targetStack, "", b);
        Push(targetStack, "", b < a);
        DISPATCH();
    _Jump:
        // addr
        Pop(targetStack, "", programCounter);
        programCounter--;
        DISPATCH();
    _CondJump:
        // cond
        Pop(targetStack, "condjump cond", a);
        // addr
        Pop(targetStack, "condjump addr", b);
        if (a) {
            programCounter = b-1;
        }
        DISPATCH();
    _LoadFrom:
        // pop addr
        Pop(targetStack, "loadfrom addr", a);
        // push its value
        Push(targetStack, "loadfrom value", granny->ram[a]);
        DISPATCH();
    _StoreTo:
        // pop addr
        Pop(targetStack, "storeto addr", a);
        // pop value
        Pop(targetStack, "storeto value", b);

        // set the ram
        granny->ram[b] = a;
        DISPATCH();
    _Add:
        Pop(targetStack, "add operand", a);
        Pop(targetStack, "add operand", b);
        Push(targetStack, "add result", b + a);
        DISPATCH();
    _Subtract:
        Pop(targetStack, "subtract operand", a);
        Pop(targetStack, "subtract operand", b);
        Push(targetStack, "subtract result", b - a);
        DISPATCH();
    _Multiply:
        Pop(targetStack, "multiply operand", a);
        Pop(targetStack, "multiply operand", b);
        Push(targetStack, "multiply result", b * a);
        DISPATCH();
    _Divide:
        Pop(targetStack, "divide operand", a);
        Pop(targetStack, "divide operand", b);
        Push(targetStack, "divide result", b / a);
        DISPATCH();
    _Halt:
        if (targetStack->pointer == 0) {
            return 0;
        }
        Pop(targetStack, "halt value", a);
        return a;
    _IOEnum:
        Push(targetStack, "io count", granny->deviceCount);
        DISPATCH();
    _IOQuery:
        Pop(targetStack, "io query device", a);
        Push(targetStack, "io query version", granny->devices[a].version);
        Push(targetStack, "io query name", granny->devices[a].name);
        DISPATCH();
    _IOAct:
        Pop(targetStack, "io act", a);
        granny->devices[a].invoke(granny, &programCounter);
        DISPATCH();
    _Call:
        Pop(targetStack, "take call addr", a);
        Push(otherStack, "put call addr on ret", programCounter);
        #ifdef DBG
        printf("calling to %d!\n", a);
        #endif
        programCounter = a - 1;
        DISPATCH();
    _Ret:
        Pop(otherStack, "return addr", a);
        programCounter = a;
        DISPATCH();
    }
}

void
initGranny(Granny *granny, size_t ramSize)
{
    granny->ramSize = ramSize;
    granny->ram = calloc(ramSize, sizeof(uint32_t));

    memset(granny->registers, 0, sizeof granny->registers);

    granny->returnStack.pointer = 0;
    memset(granny->returnStack.data, 0, sizeof granny->returnStack.data);

    granny->valuesStack.pointer = 0;
    memset(granny->valuesStack.data, 0, sizeof granny->returnStack.data);
}