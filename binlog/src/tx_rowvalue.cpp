#include "tx_rowvalue.h"
#include <cstdio>
#include "binary_log_types.h"
namespace tx_row_value{
bool row_value_init(ROW_VALUE *rowValue, uint16 mysqlType) {
	memset(rowValue, 0x00, sizeof(ROW_VALUE));
	rowValue->mysql_type_ = mysqlType;
	return true;
}
bool row_set_value(ROW_VALUE *rowValue, const char* szBuf, uint32 nLen) {
	if(szBuf == NULL)
		return false;
	char *szTmp = (char*)malloc((nLen + 1) * sizeof(char));
	if(szTmp == NULL)
		return false;
	rowValue->type_ = VALUE_TYPE_CHAR;
	memcpy(szTmp, szBuf, nLen);
	szTmp[nLen] = '\0';
	rowValue->value_.char_value_ = szTmp;
	return true;
}
bool row_set_value(ROW_VALUE *rowValue, long long value) {
	rowValue->type_ = VALUE_TYPE_LONGLONG;
	rowValue->value_.long_value_ = value;
	return true;
}
bool row_set_value(ROW_VALUE *rowValue, double value) {
	rowValue->type_ = VALUE_TYPE_DOUBLE;
	rowValue->value_.double_value_ = value;
	return true;
}

bool row_set_value(ROW_VALUE *rowValue, ROW_VALUE_TIME &value) {
	rowValue->type_ = VALUE_TYPE_TIME;
	rowValue->value_.time_value_ = value;
	return true;
}

bool row_set_value_null(ROW_VALUE *rowValue){
	rowValue->type_ = VALUE_TYPE_NULL;
	return true;
}
bool row_value_free(ROW_VALUE *rowValue) {
	if(rowValue->type_ == VALUE_TYPE_CHAR && rowValue->value_.char_value_ != NULL) {
		free(rowValue->value_.char_value_);
		rowValue->value_.char_value_ = NULL;
	}
	return true;
}

void row_value_printf(ROW_VALUE *rowValue, uint16 index) {
	switch(rowValue->type_) {
		case VALUE_TYPE_CHAR:
			printf("string[%d]:%s\n", index, rowValue->value_.char_value_);
			break;
		case VALUE_TYPE_DOUBLE:
			printf("double[%d]:%lf\n", index, rowValue->value_.double_value_);
			break;
		case VALUE_TYPE_LONGLONG:
			printf("long long[%d]:%d\n", index, rowValue->value_.long_value_);
			break;
		case VALUE_TYPE_TIME:
			printf("time[%d]:%s\n", index, time_struct_to_string(rowValue->value_.time_value_, rowValue->mysql_type_).c_str());
			break;
		case VALUE_TYPE_NULL:
			printf("NULL[%d]: null", index);
			break;
		default:
			printf("unkown\n");
			break;
	}
	
}


void time_init(ROW_VALUE_TIME& timeValue, uint year, uint month, uint day, uint hour, uint min, uint sec, uint usec) {
	timeValue.year_ = year;
	timeValue.month_ = month;
	timeValue.day_ = day;
	timeValue.hour_ = hour;
	timeValue.min_ = min;
	timeValue.sec_ = sec;
	timeValue.usec_ = usec;
}

std::string time_struct_to_string(ROW_VALUE_TIME &timeValue, uint16 mysqlType, const char* szSplit1, const char* szSplit2){
	char szTmp[256];
	switch(mysqlType) {
		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_DATETIME2: {
			sprintf_s(szTmp, 255, "%04d%s%02d%s%02d %02d%s%02d%s%02d", timeValue.year_, szSplit1, timeValue.month_, szSplit1, timeValue.day_,
				timeValue.hour_, szSplit2, timeValue.min_, szSplit2, timeValue.sec_);
			break;
		}
		case MYSQL_TYPE_DATE:
		case MYSQL_TYPE_NEWDATE: {
			sprintf_s(szTmp, 255, "%04d%s%02d%s%02d", timeValue.year_, szSplit1, timeValue.month_, szSplit1, timeValue.day_);
			break;
		}
		case MYSQL_TYPE_TIME:
		case MYSQL_TYPE_TIME2: {
			sprintf_s(szTmp, 255, "%02d%s%02d%s%02d", timeValue.hour_, szSplit2, timeValue.min_, szSplit2, timeValue.sec_);
			break;
		}
	}
	
	return std::string(szTmp);
}

};