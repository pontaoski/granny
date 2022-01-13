#!/usr/bin/env python3

import sys, struct

labels = dict()
image = []

opcodes = {
#    "Literal": 0x00,
    "Increment": 0x01,
    "Decrement": 0x02,
    "Duplicate": 0x03,
    "Drop": 0x04,
    "Swap": 0x05,
    "Toggle": 0x06,
    "Equals": 0x07,
    "NotEquals": 0x08,
    "GreaterThan": 0x09,
    "LessThan": 0x0a,
    "Jump": 0x0b,
    "CondJump": 0x0c,
    "LoadFrom": 0x0d,
    "StoreTo": 0x0e,
    "Add": 0x0f,
    "Subtract": 0x10,
    "Multiply": 0x11,
    "Divide": 0x12,
    "Halt": 0x13,
    "IOEnum": 0x14,
    "IOQuery": 0x15,
    "IOAct": 0x16,
    "Call": 0x17,
    "Ret": 0x18,
    "Over": 0x19,
}

included = set()

def tokens(in_string: str):
    output = []
    macros = dict()

    in_block = False

    for line in in_string.splitlines():
        if line.strip() == "---":
            in_block = not in_block
            continue
        elif not in_block:
            continue

        currentToken = ""

        def eatUntilWhitespace():
            nonlocal currentToken
            nonlocal line

            currentToken = line[0]
            line = line[1:]
            while len(line) > 0 and not line[0].isspace():
                currentToken = currentToken + line[0]
                line = line[1:]

        def eatWhitespace():
            nonlocal line

            while len(line) > 0 and line[0].isspace():
                line = line[1:]

        def skip():
            nonlocal line
            line = line[1:]

        def pushToken(kind):
            nonlocal currentToken
            nonlocal output

            output += [(kind, currentToken)]
            currentToken = ""

        while len(line) > 0:
            match line[0]:
                case space if space.isspace():
                    eatWhitespace()
                case '#':
                    skip()
                    eatUntilWhitespace()
                    pushToken("number")
                case '|':
                    skip()
                    eatUntilWhitespace()
                    pushToken("padding")
                case '&':
                    skip()
                    eatUntilWhitespace()
                    pushToken("label-reference")
                case '\'':
                    skip()
                    eatUntilWhitespace()
                    pushToken("char")
                case '*':
                    skip()
                    eatUntilWhitespace()
                    pushToken("insert-byte")
                case ':':
                    skip()
                    eatUntilWhitespace()
                    pushToken("label")
                case '@':
                    skip()
                    eatUntilWhitespace()
                    pushToken("sublabel")
                case '"':
                    skip()

                    currentToken = line[0]
                    line = line[1:]
                    while len(line) > 0 and not line[0] == '"':
                        currentToken = currentToken + line[0]
                        line = line[1:]

                    skip()

                    pushToken("string")
                case '(':
                    skip()

                    currentToken = line[0]
                    line = line[1:]
                    while len(line) > 0 and not line[0] == ')':
                        currentToken = currentToken + line[0]
                        line = line[1:]

                    skip()
                    currentToken = ""
                case '!':
                    skip()
                    eatUntilWhitespace()
                    pushToken("label-reference")
                    output += [("word", "Call")]
                case '~':
                    skip()
                    skip()

                    currentToken = line[0]
                    line = line[1:]
                    while len(line) > 0 and not line[0] == '"':
                        currentToken = currentToken + line[0]
                        line = line[1:]

                    skip()

                    fname = currentToken
                    currentToken = ""

                    if fname in included:
                        continue

                    included.add(fname)

                    toks = []
                    with open(fname, 'r') as file:
                        toks = tokens(file.read())

                    output += toks
                case _:
                    eatUntilWhitespace()
                    pushToken("word")

    return output

def pass1(toks):
    global labels

    i = 0
    f = sys.argv[1]
    last_label = ""

    for token in toks:
        match token:
            case ('number', cont):
                i += 2
            case ('padding', cont):
                num = int(cont)
                while i < num:
                    i += 1
            case ('label-reference', cont):
                i += 2
            case ('char', cont):
                i += 2
            case ('insert-byte', cont):
                i += 1
            case ('string', cont):
                i += len(bytes(bytes(cont, "utf-8").decode("unicode_escape"), "utf-8"))
                i += 1
            case ('label', label):
                labels[label] = i
                last_label = label
            case ('sublabel', label):
                labels[f"{last_label}:{label}"] = i
            case ('word', word):
                i += 1

def pass2(toks):
    global image

    i = 0
    f = sys.argv[1]
    last_label = ""

    for token in toks:
        match token:
            case ('number', cont):
                image[i] = 0x00
                image[i+1] = int(cont)

                print(f"{i} Literal\n{i+1} {cont}")

                i += 2
            case ('padding', cont):
                num = int(cont)
                while i < num:
                    image[i] = 0xFFFFFFFF
                    print(f"{i} Padding")
                    i += 1
            case ('label-reference', cont):
                image[i] = 0x00
                image[i+1] = labels[cont]

                print(f"{i} Literal\n{i+1} {labels[cont]} ({cont})")

                i += 2
            case ('char', cont):
                image[i] = 0x00
                image[i+1] = ord(bytes(cont, "utf-8").decode("unicode_escape"))

                print(f"{i} Literal\n{i+1} {image[i+1]} ('{cont}')")

                i += 2
            case ('insert-byte', cont):
                image[i] = ord(bytes(cont, "utf-8").decode("unicode_escape"))

                print(f"{i} {image[i]} (Byte)")

                i += 1
            case ('string', cont):
                content = bytes(bytes(cont, "utf-8").decode("unicode_escape"), "utf-8")

                for byte in content:
                    image[i] = byte
                    print(f"{i} {image[i]} (String Byte)")
                    i += 1

                image[i] = 0
                print(f"{i} {image[i]} (String Null Terminator)")
                i += 1
            case ('label', label):
                label = label
                last_label = label
                print(f"{label}:")
            case ('sublabel', label):
                print(f"{last_label}:{label}:")
            case ('word', word):
                image[i] = opcodes[word]
                print(f"{i} {word} {opcodes[word]}")
                i += 1

def save(filename):
    with open(filename, "wb") as file:
        j = 0
        while j < 1024:
            file.write(struct.pack("I", image[j]))
            j = j + 1

if __name__ == "__main__":
    toks = []
    with open(sys.argv[1], 'r') as file:
        toks = tokens(file.read())

    image.extend([0] * 1024)
    pass1(toks)
    pass2(toks)
    save("grannyImage")
