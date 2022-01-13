#!/usr/bin/env python3

import sys, struct

opcodes = {
    0x00: "Literal",
    0x01: "Increment",
    0x02: "Decrement",
    0x03: "Duplicate",
    0x04: "Drop",
    0x05: "Swap",
    0x06: "Toggle",
    0x07: "Equals",
    0x08: "NotEquals",
    0x09: "GreaterThan",
    0x0a: "LessThan",
    0x0b: "Jump",
    0x0c: "CondJump",
    0x0d: "LoadFrom",
    0x0e: "StoreTo",
    0x0f: "Add",
    0x10: "Subtract",
    0x11: "Multiply",
    0x12: "Divide",
    0x13: "Halt",
    0x14: "IOEnum",
    0x15: "IOQuery",
    0x16: "IOAct",
    0x17: "Call",
    0x18: "Ret",
}

def disassemble(filename):
    with open(filename, "rb") as file:
        j = 0

        data = file.read()

        while j < 1024:
            opcode = struct.unpack_from("I", data, j)[0]

            if opcode in opcodes:
                print(f"{j} {opcodes[opcode]}")
            else:
                print(f"{j} {opcode}")

            j += 1

if __name__ == "__main__":
    disassemble("grannyImage")
