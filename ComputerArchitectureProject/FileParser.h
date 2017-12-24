#ifndef FILEPARSER_H_
#define FILEPARSER_H_

#include "mainDefs.h"

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

STATUS FileParser_MeminParser(UINT32* memin);

STATUS FileParser_ConfigParser(PCONFIG pConfig);




#endif //FILEPARSER_H_