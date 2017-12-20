# Run by command line: python compiler.py <name of program>


opcodes = ['LD', 'ST', 'ADD', 'SUB', 'MULT', 'DIV', 'HALT']


def compile(filename):
    ASM = open(filename, 'r')
    BIN = open(filename + '.bin', 'w')

    for ind, line in enumerate(ASM):
        if line.startswith("#") or line[0].isspace():
            continue

        comp = line.split()
        opcode = opcodes.index(comp[0])
        DST = 0
        SRC0 = 0
        SRC1 = 0
        IMM = 0
        if (opcode == 0):  # LD
            DST = comp[1][1:]
            IMM = comp[2]
        elif (opcode == 1):  # ST
            SRC1 = comp[1][1:]
            IMM = comp[2]
        elif (opcode == 6):  # HALT
            None
        else:
            DST = comp[1][1:]
            SRC0 = comp[2][1:]
            SRC1 = comp[3][1:]

        IMM = 0xFFF & int(IMM)
        SRC1 = (0xF & int(SRC1)) << 12
        SRC0 = (0xF & int(SRC0)) << 16
        DST = (0xF & int(DST)) << 20
        OPC = (0xF & int(opcode)) << 24

        inst = OPC | DST | SRC0 | SRC1 | IMM
        inst = 0x0FFFFFFF & inst

        print(str(ind) + ". " + line, '\t', "0x{:08x}".format(inst), "0b{:032b}".format(inst))
        BIN.write("{:08x}\n".format(inst))


if __name__ == '__main__':
    from sys import argv

    if len(argv) == 1:
        print("Run by command line: python compiler.py <name of program>")
    else:
        compile(argv[1])
