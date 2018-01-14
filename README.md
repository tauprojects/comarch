# Computer Architecture – Tomasulo Project
#### Almog Zeltsman: 204312763
#### Matan Gizunterman: 303157804
_____
The document contains an high-level description of the way we chose to implement the tomasulo algorithm, as defined in the project assignment.
All low-level description, include functions and macros description can be found inside the header and source files.


#### The project archive file the following files and directories:
- 	`/Solution/ComputerArchitectureProject/main.c` – main source file 
- 	`/Solution/ComputerArchitectureProject/mainDefs.h` – Macros, constants and structs declarations of tomasulo different units and other helper usages.
-	`/Solution/ComputerArchitectureProject/Queue.c` – FIFO Queue implementation to be used by instruction buffer 
-	`/Solution/ComputerArchitectureProject/Queue.h` –  Queue functions and features declarations. Constants and structs declarations used by implementation file.
-	`/Solution/ComputerArchitectureProject/ReservationStations.c`  Tomasulo reservation stations functionality of all different stations.
-	`/Solution/ComputerArchitectureProject/ReservationStations.h` – corresponding header file.  
-	`/Solution/ComputerArchitectureProject/Instructions.c` – functions, macros and structs declarations for parsing and working with input instructions.
-	`/Solution/ComputerArchitectureProject/Instructions.h` – corresponding header file.
-	`/Solution/ComputerArchitectureProject/FunctionalUnits.c` – functions and macros to be used by different functional units to implement tomasulu algorithm functionality. 
-	`/Solution/ComputerArchitectureProject/FunctionalUnits.h` – main source file 
-	`/Solution/ComputerArchitectureProject/FilesManager.c` – macros and function to be used with reading and parsing input files, and writing output files as required.
-	`/Solution/ComputerArchitectureProject/FilesManager.h` - corresponding header file. 
-	`/Solution/ComputerArchitectureProject/safeMalloc.c` – helper macros and functions for handling memory managements
-	`/Solution/ComputerArchitectureProject/safeMalloc.h` –  corresponding header file. 
-	`/ComputerArchitectureProject.sln` – solution file build for visual studio compile and build.
-	`/Example/Protgram2/*` - First test program input and expected output files including extra `.asm` file.
-	`/Example/Protgram3/*` - Second test program input and expected output files including extra`.asm` file.
-	`/Example/Protgram1/*` - Third test program input and expected output files including extra `.asm` file.

**Note:** The header and source files contains documentation and description of the declared functions and macros. other descriptions are commented in the source code files and probably can shed more light on the way we chose to implement the tomasulo algorithm.

The project implementations, based on the given tamasulo design and definitions, consist of similar abstractions for each block (unit) and each stage (FETCH-ISSUE-EXECUTE) of the tomasulo algorithm design.

- **Memory Map**  - Array of `unsigned int`, defined as `UINT32`, initialized with 4096 elements and represent the memory map as defined with 4096 memory entries of 32 bit.
- **FP Registers** - Array of `Register` structs that represent a floating point register. Consist of the register value or tag and other helper members such as `hasTag` flag and a pointer to the relevant instruction. The registers array initialized with 16 elements at start, contains a growing series values as defined. 
- **Instruction Queue** - Implemented with FIFO Queue, with capacity as defined. Used in FETCH stage to hold instructions from memory with `enqueue` and in `ISSUE` with `peek` and `dequeue`.
- **Reservation Stations** - Implemented as struct, defined as `RsvStation` , holding one instruction per instance (as defined) as part of an array as the number configure of reservation stations per type. Each struct consist of a pointer to the instruction that came from the instruction queue. Each struct also consist of helper flags, tags and Qj,Qk,Vj,Vk members. The reservations stations initialized at start for each of the different types, with size as defined in the configuration file. 
- **Store\Load Buffers** -  Implemented with the same `RsvStation`struct but with special treatment, due to the fact that load and store instructions does not necessarily used the structs members as other stations does. Also, the load and store buffers implementation requires us to check the memory address of the immediate value in the instructions to handle RAW and WAR to preserve the instruction order.
- **Control Lines** - Implemented with variables and pointers that accessed by by each endpoint unit, and by that represent a control line with data. 
- **Control Data Busses (4 different CDB's)** - Implemented as a global declared array of `CDB` structs, consist of tag, value and other helper members. Each global element in the array can be accesses by all tomasulo units and each unit is responsible to validate that the CDB does not contain any value (busy state). 
- **Functional Units** -  Implemented as an array of `FunctionUnit` structs, consist functional unit type, name  of relevant operands to calculate, configured delay time and other helper members. initialized at start for each of the distinct types, with size and delay as defined in the configuration file.
- **Memory Units**  - 
Implemented as `FunctionUnit` struct, as the functional unit, but with special treatment to handle pipelined instructions.

At initialization, after input configuration file is parsed into dedicated struct `CONFIG` that consist of all different configuration values, we initialize all tamsulo units aligned with given configuration values.
After all initialization are made, at steady state, the main function enters the main while loop that simulate a parallel execution of all tomasulo distributed unit’s functionality. Each iteration of the main loop consist of exactly **one** full clock cycle.
Although the main program acts as serial execution, the order of each serial function execution takes in consideration the fact that it is a part of parallel distributed functionality of the tomasulo algorithm and our implementation utilized this fact to avoid or handle some of the tamsulo scenarios and edge cases.
In each cycle, the program simulates FETCH-ISSUE-EXECUTE stages, where the **FETCH** stage is in the responsibility of the instruction queue, **ISSUE** is in the responsibility of all different reservation stations and **EXECUTE** is in the responsibility of all different functional units, that also responsible of broadcasting new calculated values into the relevant CDB.
We used variables, pointes and flags all over the different implemented data structures in order to sample the accurate relevant clock cycle and then hold them until printing the required output files.

The project consist of a lot macros and helper functions used for better, readable and manageable code.
We also managed memory allocation and de-allocation with external self-implemented functions to avoid memory leaks and monitor each allocation.

### Implementation decisions
- At ISSUE stage, the instructions from the instructions queue will be passed to the relevant reservration station in the same order of the reservation stations (ascending order), namely, it will take the first one that is not empty.
- At EXECUTION stage, in case more than one reservreation station hold a valid instuction (no tags) for a relevant functional unit, we chose to pick the instruction by the order of the reservetaions station. For example, if both Loadbuffer1 and Loadbuffer2 contains a valid instrucion, the memory unit will take the instruction from Loadbuffer1 (Unless there is dependecy between them). The relevant functional unit will be chosen also by ascending order.

### Test programs

##### As asked, the project archive contains three testing programs that verify that our implementation work as expected.

- The first program test case in which three instructions left operand is dependent on an earlier instruction and waits until value is written to the CDB and updated in the relevant reservation stations at the same clock cycle. Afterward, we expect to see all functional units finished and writes to their relevant CDB at the same clock cycle, after configuring all functional units with the same delay time.
The program also tests the case in which one instruction contains two operands that depend on earlier instructions. Both operands not necessarily become valid at the same time,  and each operand will be updated when the relevant CDB will hold the relevant tag + value.
- The second program tests the cases in which all reservation stations and functional units are full while other instructions waiting in the instruction queue. 
The programs also contain cases of WAW, RAW and WAR from the same memory address, and WAR from the same register (different address).
Also, the program test the case in which an instruction's two operands consist the same dependent register. 

- The third program tests the memory unit pipeline, RAR and independency between STORE instructions to othera.
In this program we can also see that the instruction execution order is different than the instruction issue order (if there are not dependency between them) due to the arbitrary way of the functional units are taking available instructions from the reservation stations.



  

