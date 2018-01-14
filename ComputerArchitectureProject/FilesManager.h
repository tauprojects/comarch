#ifndef FILESMANAGER_H_
#define FILESMANAGER_H_

#include "mainDefs.h"


/**
* This functions parses the MEMIN file into the already existing memory.
* It opens the file by its filename, initializes the memory to 0, and parses each hex number
* to unsigned int, using the #FilesManager_GetLine function.
*
* @param	memory		-	memory array
* @param	filename	-	file path to the memin file
*
* @ret		#STATUS_INVALID_ARGS if memory == NULL, #STATUS_STRTOL_FAIL if parsing a number failed,
*			#STATUS_FILE_FAIL if file opening failed, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_MeminParser(UINT32* memin, CPCHAR filename);


/**
* This functions parses the CONFIG file into the already config.
* It opens the file by its filename, initializes the config, and parses each line in the file.
*
* @param	pConfig		-	pointer to existing config struct
* @param	filename	-	file path to the CONFIG file
*
* @ret		#STATUS_PARSE_FAIL if parsing a line failed, #STATUS_WRONG_NAME_FAIL if (name = value) - name
*			is not recognized, #STATUS_FILE_FAIL if file opening failed, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_ConfigParser(PCONFIG pConfig, CPCHAR filename);



/**
* This function opens all output files using #fopen
* @ret #STATUS_FILE_FAIL if any fopen failed, otherwise #STATUS_SUCCESS.
*/
STATUS FilesManager_InitializeOutputFiles(CPCHAR memoutFile, CPCHAR regoutFile, CPCHAR traceinstFile, CPCHAR tracedbFile);



/**
* This function closes all the output files using #fclose.
*/
VOID FilesManager_FinalizeOutputFiles(VOID);



/**
* This function writes the register values to the RFGOUT file.
* @param F		- register array
* @ret #STATUS_INVALID_ARGS if F == NULL, #STATUS_FILE_FAIL if regout global is NULL, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_WriteRegisters(Register F[]);



/**
* This function writes the memory to the MEMOUT file, it writes all MEMORY_SIZE cells of the memory,
* as hexadecimal values
* @param mem		- memory array
* @ret #STATUS_INVALID_ARGS if mem == NULL, #STATUS_FILE_FAIL if memout global is NULL, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_WriteMemout(UINT32* mem);


/**
* This function writes the instructions to the traceinst, in the same order they were issued, using the global
* array instrctionByIssue.
* @ret  #STATUS_FILE_FAIL if traceinst global is NULL, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_WriteTraceinst(VOID);

/**
* This function checks all 4 CDBS in the order ADD, MUL, DIV, MEM, and writes to the tracedb file
* for every one that is not empty.
* @param CDBs	-	CDB array
* @param CC		-	Current clock cycle
* @ret  #STATUS_FILE_FAIL if tracedb global is NULL, otherwise #STATUS_SUCCESS
*/
STATUS FilesManager_WriteTracedb(pCDB CDBs, UINT32 CC);

/**
 * This function adds a pointer to an instruction context to the global #instrctionByIssue.
 * @param pInst	-	pointer to the current instruction context
 */
VOID FilesManager_AddToIssueArray(PInstCtx pInst);

#endif //FILESMANAGER_H_