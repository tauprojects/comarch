/*
 Created By: Matan Gizunterman, Roie Ben Simon
 TAU Username: gizunterman, Roieben
 ID number: 303157804, 038196309
 Part of the code was with the help of given examples code, StackOverflow Issues, and Tutorials Point C Programming Guide
 */


#include "sim.h"

// main memory
int mem[MEM_SIZE];
//Registers
int reg_list[NUM_REGS];
char str1[20];
// opcode names
char op_name[][10] = { "add", "sub", "and", "or", "sll", "sra", "srl", "beq",
		"bgt", "ble", "bne", "jal", "lw", "sw", "lhi", "halt" };

// register names
char reg_name[][10] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };

int inst, op, rd, rs, rt, imm, PC, instCnt = 0; //global variables - for easy modeling

// files to be read\written
FILE *fp_memin, *fp_memout, *fp_regout, *fp_trace, *fp_count;


//Main function
int main(int argc, char *argv[]) {
	//Variables declarations
	int i, last;

	//Checking if number of arguments is valid
	if (argc < 6) {
		printf(ERR_MSG_INVALID_ARGS_NUM);
		printf("%d\n", argc - 1);
		exit(-1);
	} else if (argc > 6) {
		printf(ERR_MSG_INVALID_ARGS_NUM);
		printf("%d\n", argc - 1);
		exit(-1);
	}
	//Memory map input file path
	char* meminPath = argv[1];

	//Memory Map output file path
	char* memoutPath = argv[2];

	//Registers program output file path
	char* regoutPath = argv[3];

	//Trace output file path
	char* tracePath = argv[4];

	//Operations count file path
	char* countPath = argv[5];

	//Opening files and validating return value for success
	fp_memin = fopen(meminPath, "rt");
	if (!fp_memin) {
		printf(ERR_MSG_OPEN_FILE);
		puts("memin.txt");
		exit(-1);
	}


	fp_trace = fopen(tracePath, "wt");
	if (!fp_trace) {
		printf(ERR_MSG_OPEN_FILE);
		puts("trace.txt");
		exit(-1);
	}


	//Initial Memory to zero
	memset(mem, 0, MEM_SIZE * sizeof(int));

	//Initial Registers to zero
	memset(reg_list, 0, NUM_REGS * sizeof(char));

	//Read memin.txt content into memory map
	i = 0;
	while (!feof(fp_memin)) {
		if (fscanf(fp_memin, "%08X\n", &mem[i]) != 1)
			break;
		i++;
	}
	//Closing memin after reading
	fclose(fp_memin);

	//Find last non-zero memory entry
	last = MEM_SIZE - 1;
	while (last >= 0 && mem[last] == 0)
		last--;

	//Decode Instruction
	PC = 0;  //Initial Program Counter
	while (PC <= last) {
		//Fetch Stage
		inst = mem[PC];
		//print non zero contents
		if (inst != 0) {

			//Decode Stage
			op = sbs(inst, 31, 28);
			rd = sbs(inst, 27, 24);
			rs = sbs(inst, 23, 20);
			rt = sbs(inst, 19, 16);
			imm = sbs(inst, 15, 0);
			reg_list[1] = (short) imm;

			if (op == 0 && rd == 0) {
				PC++;
			} else {

				//Instruction Counter Increment
				instCnt++;

				//Print count.txt
				printCount(countPath);

				//Print trace.txt
				printTrace(PC, inst);

				//Execute Stage
				instExec();

				//Printing memout.txt
				printMemout(memoutPath);

				//Printing register state
				printRegout(regoutPath);
			}
		}
		else{
			PC++;
		}
	}
	gracfullyExit();
	return 0;

}

void instExec() {
	unsigned int temp;
	switch (op) {
	case 0:		 //$add
		reg_list[rd] = reg_list[rs] + reg_list[rt];
		PC++;
		break;

	case 1:		//$sub
		reg_list[rd] = reg_list[rs] - reg_list[rt];
		PC++;
		break;

	case 2:		//$and
		reg_list[rd] = reg_list[rs] & reg_list[rt];
		PC++;
		break;

	case 3:		//$or
		reg_list[rd] = reg_list[rs] | reg_list[rt];
		PC++;
		break;

	case 4:		//$sll
		reg_list[rd] = logicalLeftShift(reg_list[rs], reg_list[rt]);
		PC++;
		break;

	case 5:		//$sra
		reg_list[rd] = arithmeticRightShift(reg_list[rs], reg_list[rt]);
		PC++;
		break;

	case 6:		//$srl
		reg_list[rd] = logicalRightShift(reg_list[rs], reg_list[rt]);
		PC++;
		break;

	case 7:		//$beq
		if (reg_list[rs] == reg_list[rt]) {
			PC = (reg_list[rd] & 0x0000FFFF) - 1; //if (R[rs] == R[rt]) pc = R[rd][low bits 15:0]
		}
		PC++;
		break;

	case 8:		//$bgt

		if (reg_list[rs] > reg_list[rt]) {
			PC = (reg_list[rd] & 0x0000FFFF) - 1; //if (R[rs] > R[rt]) pc = R[rd][low bits 15:0]
		}
		PC++;
		break;

	case 9:		//$ble
		if (reg_list[rs] <= reg_list[rt]) {
			PC = (reg_list[rd] & 0x0000FFFF) - 1; //if (R[rs] <= R[rt]) pc = R[rd] [low bits 15:0]

		}
		PC++;
		break;

	case 10:	//$bne
		if (reg_list[rs] != reg_list[rt]) {
			PC = (reg_list[rd] & 0x0000FFFF) - 1; //if (R[rs] != R[rt]) pc = R[rd] [low bits 15:0]
		}
		PC++;
		break;

	case 11:	//$jal
		reg_list[15] = PC + 1; //(next instruction address),
		PC = reg_list[rd] & 0x0000FFFF;

		break;

	case 12:	//$lw
		reg_list[rd] = mem[reg_list[rs] + reg_list[rt]];
		PC++;
		break;

	case 13:	//$sw
		mem[reg_list[rs] + reg_list[rt]] = reg_list[rd];
		PC++;
		break;

	case 14:	//$lhi
		temp = (reg_list[rs] & 0x0000FFFF) << 16;
		reg_list[rd] = (reg_list[rd] & 0x0000FFFF) | temp;
		PC++;
		break;

	case 15:	//$halt
		gracfullyExit();
		exit(0);
		break;
	}

}
void printTrace() {
	fprintf(fp_trace, "%08X %08X ", PC, inst);
	int i;
	for (i = 0; i < NUM_REGS; i++) {
		fprintf(fp_trace, "%08x ", reg_list[i]);
	}
	fprintf(fp_trace, "\n");
}

void printRegout(char* regoutPath) {
	fp_regout = fopen(regoutPath, "wt");
	if (!fp_regout) {
		printf(ERR_MSG_OPEN_FILE);
		puts("regout.txt");
		exit(-1);
	}
	int i;
	for (i = 2; i < NUM_REGS; i++) {
		fprintf(fp_regout, "%08X\n", reg_list[i]);
	}
	fclose(fp_regout);
}
void printMemout(char* memoutPath) {
	fp_memout = fopen(memoutPath, "wt");
	if (!fp_memout) {
		printf(ERR_MSG_OPEN_FILE);
		puts("memout.txt");
		exit(-1);
	}
	int i;
	for (i = 0; i < MEM_SIZE; i++) {
		fprintf(fp_memout, "%08X\n", mem[i]);
	}
	fclose(fp_memout);
}


void printCount(char* countPath){
	//Printing count.txt - the number of instructions
	fp_count = fopen(countPath, "wt");
	if (!fp_count) {
		printf(ERR_MSG_OPEN_FILE);
		puts("count.txt");
		exit(-1);
	}
	fprintf(fp_count, "%d\n", instCnt);
	fclose(fp_count);
}

void gracfullyExit() {
	// close opened files
	fclose(fp_trace);
	puts("Exited Gracefully");
}

int logicalRightShift(int x, int n) {
	return (unsigned) x >> n;
}
int arithmeticRightShift(int x, int n) {
	if (x < 0 && n > 0)
		return x >> n | ~(~0U >> n);
	else
		return x >> n;
}
int logicalLeftShift(int x, int n) {
	return (unsigned) x << n;
}

// extract single bit
int sb(int x, int bit) {
	return (x >> bit) & 1;
}

// extract multiple bits
int sbs(int x, int msb, int lsb) {
	if (msb == 31 && lsb == 0)
		return x;
	return (x >> lsb) & ((1 << (msb - lsb + 1)) - 1);
}
