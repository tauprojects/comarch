/*
 * asm.h
 *
 *  Created on: May 12, 2017
 *      Author: user
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memory Size = 65535
#define MEM_SIZE (1 << 16)

//Number of internal registers
#define NUM_REGS  16

//Word size - 32 bit
#define BIT_SIZE (1 << 4)

#define LINE_SIZE (500)

//Error Messages Declarations
#define ERR_MSG_INVALID_ARGS_NUM "Invalid Number Of Arguments - Expected: 5 , Actual: "
#define INVALID_ARG "Invalid Argument"
#define ERR_MSG_OPEN_FILE "Error While Opening File: "


/////////////////////////
//Function Declarations//
/////////////////////////

/*
 * Print Trace method()
 * prints the register status into trace.txxt
 * @return - void
 */
void printTrace();

/*
 * Print Regout method()
 * prints the finite register states into regout.txxt
 * @return - void
 */
void printRegout(char* regoutPath);
/*
 * Print Memout method()
 * prints the  memory state into memout.txt after each instruction execution
 * @return - void
 */
void printMemout(char* memoutPath);
/*
 * Print cCount method()
 * prints the Instruction counter into count.txt after each instruction execution
 * @return - void
 */
void printCount(char* countPath);

/*
 * Instruction Execution method()
 * execute a current instruction - using switch-case
 * works with and modify  global variables
 * @return - void
 */
void instExec();

/*
 * Shift Right Logical method
 * @x - input number to be shifted
 * @n - input shift number.
 * @return - x<<n.
 */
void gracfullyExit();

/*
 * Shift Right Logical method
 * @x - input number to be shifted
 * @n - input shift number.
 * @return - x>>n.
 */
int logicalRightShift(int x, int n);

/*
 * Shift Right Arithmetic method - with sign extension
 * @x - input number to be shifted
 * @n - input shift number.
 * @return - x>>n.
 */
int arithmeticRightShift(int x, int n);

/*
 * Shift Left Logical method
 * @x - input number to be shifted
 * @n - input shift number.
 * @return - x<<n.
 */
int logicalLeftShift(int x, int n);

/*
 * Extract multiple bits method()
 * @x - input value
 * @lsb
 * @msb
 */
int sbs(int x, int msb, int lsb);

/*
 * Extract single bit method()
 * @x - input value
 * @bit - bit number.
 */
int sb(int x, int bit);

#endif /* MAIN_H_ */
