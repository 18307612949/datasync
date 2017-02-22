#ifndef TX_COMMON
#define TX_COMMON
#include <algorithm>
#include <iostream>
#include <vector>
#include "my_global.h"
#include <tx_ctype.h>
#include "log_event.h"
#include "binlog_event.h"
#include "my_byteorder.h"
#include "byteorder.h"
#include "my_dbug.h"
#include "my_compiler.h"
class CHeaderEvent;

//extern uint16 global_binlog_version;
//extern char global_binlog_szversion[50];
extern CHeaderEvent global_description_event;

#endif //TX_COMMON