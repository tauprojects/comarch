#ifndef FILEPARSER_H_
#define FILEPARSER_H_

#define MEMORY_SIZE		4096
#define MAX_LINE_LEN	200
#define UINT32			unsigned int
#define MEMIN_FILENAME	"memin.txt"
#define CONF_FILENAME	"config.txt"
#define TRUE			1
#define FALSE			0


typedef struct _CONFIG{
	UINT32 add_nr_units;
	UINT32 mul_nr_units;
	UINT32 div_nr_units;
	UINT32 add_nr_reservation;
	UINT32 mul_nr_reservation;
	UINT32 div_nr_reservation;
	UINT32 add_delay;
	UINT32 mul_delay;
	UINT32 div_delay;
	UINT32 mem_delay;
	UINT32 mem_nr_load_buffers;
	UINT32 mem_nr_store_buffers;
} CONFIG, *PCONFIG;


typedef enum _STATUS {
	STATUS_SUCCESS,
	STATUS_MEMORY_FAIL,
	STATUS_FILE_FAIL,
	STATUS_PARSE_FAIL,
	STATUS_STRTOL_FAIL,
	STATUS_WRONG_NAME_FAIL,
	STATUS_GENERAL_FAIL
} STATUS; 


STATUS FileParser_MeminParser(UINT32** pMemin);

STATUS FileParser_ConfigParser(PCONFIG pConfig);




#endif //FILEPARSER_H_