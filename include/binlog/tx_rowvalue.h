#ifndef ROW_VALUE_
#define ROW_VALUE_
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <time.h>
#include <cstdio>
#include "tx_ctype.h"
#define VALUE_TYPE_CHAR 'c'
#define VALUE_TYPE_LONGLONG 'l'
#define VALUE_TYPE_NULL 'n'
#define VALUE_TYPE_DOUBLE 'd'
#define VALUE_TYPE_TIME 't'
typedef struct row_value_time {
	uint year_;
	uint month_;
	uint day_;
	uint hour_;
	uint min_;
	uint sec_;
	uint usec_;
	ulonglong timescamp_;
	
}ROW_VALUE_TIME;
union row_value_group{
	long long long_value_;
	char* char_value_;
	double double_value_;
	ROW_VALUE_TIME time_value_;
};
typedef struct row_value{
	struct row_value(){}
	row_value(const row_value& valueOld) {
		memcpy(this, &valueOld, sizeof(row_value));
		
		if(valueOld.type_ == VALUE_TYPE_CHAR) {
			memset(&value_, 0x00, sizeof(union row_value_group));
			uint32 oldLen = strlen(valueOld.value_.char_value_);
			value_.char_value_ = (char*)malloc((oldLen + 1)*sizeof(char));
			memcpy(value_.char_value_, valueOld.value_.char_value_, oldLen);
			value_.char_value_[oldLen] = '\0';
		}
	}
	char type_;
	uint16 mysql_type_;
	union row_value_group value_;
	std::string table_name_;

}ROW_VALUE, *P_ROW_VALUE;

namespace tx_row_value{
	extern bool row_value_init(ROW_VALUE *rowValue, uint16 mysqlType);
	extern bool row_value_free(ROW_VALUE *rowValue);

	extern bool row_set_value(ROW_VALUE *rowValue, const char* szBuf, uint32 nLen);
	extern bool row_set_value(ROW_VALUE *rowValue, long long value);
	extern bool row_set_value(ROW_VALUE *rowValue, double value);
	extern bool row_set_value(ROW_VALUE *rowValue, ROW_VALUE_TIME &value);
	extern bool row_set_value_null(ROW_VALUE *rowValue);

	extern void row_value_printf(ROW_VALUE *rowValue, uint16 index = 0);

	extern void time_init(ROW_VALUE_TIME& timeValue, uint year = 0, uint month = 0, uint day = 0, uint hour = 0, uint min = 0, uint sec = 0, uint usec = 0);
	extern std::string time_struct_to_string(ROW_VALUE_TIME &timeValue,  uint16 mysqlType, const char* szSplit1="-", const char* szSplit2=":");
};

#endif //ROW_VALUE_