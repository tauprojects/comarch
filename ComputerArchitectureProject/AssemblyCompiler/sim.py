

from struct import pack,unpack


def float2hex(f):
    return hex(unpack('<I', pack('<f', f))[0])


def run():
    print()
    '''registers'''
    F = [float(i) for i in range(16)]
    opcodes = ['LD', 'ST', 'ADD', 'SUB', 'MULT', 'DIV', 'HALT']

    meminFile = open('memin.txt', 'r')
    memoutFile = open('memout.txt', 'w')

    MEM = [0 for i in range(4096)]
    index = 0

    for line in meminFile:
        if line[0].isspace():
            continue

        MEM[index] = int(line[:8],16)
        index += 1

    for i,inst in enumerate(MEM):
        opcode = (inst >> 24) & 0xF
        DST = (inst >> 20) & 0xF
        SRC0 = (inst >> 16) & 0xF
        SRC1 = (inst >> 12) & 0xF
        IMM = inst & 0xFFF
		
        print("{}. ".format(i),end='')
		
        if opcode == 0:  # LD
            F[DST] = float(MEM[IMM])
            print("{}\tF{}\t{}\t\t#F{} = {}".format(opcodes[opcode],DST,IMM,DST,float(MEM[IMM])))
        elif opcode == 1:  # ST
            MEM[IMM] = F[SRC1]
            print("{}\tF{}\t{}\t\t#MEM[{}] = ".format(opcodes[opcode],SRC1,IMM,IMM,F[SRC1]))
			
        elif opcode == 2:  # ADD
            print("{}\tF{}\tF{}\tF{}\t\t#F{} = {} + {} = {}".format(opcodes[opcode],DST,SRC0,SRC1,DST,F[SRC0],F[SRC1],F[SRC0] + F[SRC1]))
            F[DST] = F[SRC0] + F[SRC1]

			
        elif opcode == 3:  # SUB
            print("{}\tF{}\tF{}\tF{}\t\t#F{} = {} - {} = {}".format(opcodes[opcode],DST,SRC0,SRC1,DST,F[SRC0],F[SRC1],F[SRC0] - F[SRC1]))
            F[DST] = F[SRC0] - F[SRC1]

        elif opcode == 4:  # MULT
            print("{}\tF{}\tF{}\tF{}\t\t#F{} = {} * {} = {}".format(opcodes[opcode],DST,SRC0,SRC1,DST,F[SRC0],F[SRC1],F[SRC0] * F[SRC1]))
            F[DST] = F[SRC0] * F[SRC1]


        elif opcode == 5:  # DIV
            print("{}\tF{}\tF{}\tF{}\t\t#F{} = {} / {} = {}".format(opcodes[opcode],DST,SRC0,SRC1,DST,F[SRC0],F[SRC1],F[SRC0] / F[SRC1]))
            F[DST] = F[SRC0] / F[SRC1]

        elif opcode == 6:  # HALT
            print("{}".format(opcodes[opcode]))
            break
        else:
            continue

    for element in MEM:
        memoutFile.write(float2hex(element)[2:]+'\n')

    regout = open("regout.txt",'w')
    print()
    for i,reg in enumerate(F):
        print("F{} = {}".format(i,reg))
        regout.write('F'+str(i)+'\t'+float2hex(reg)[2:]+'\t('+str(reg)+')\n')

if __name__ == '__main__':
    run()
