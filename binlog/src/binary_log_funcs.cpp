#include "binary_log_funcs.h"

static uint uint_max(int bits) {
	return (((1U << (bits - 1)) - 1) << 1) | 1;
}
uint my_time_binary_length(uint dec){
	return 3 + (dec + 1) / 2;
}

uint my_datetime_binary_length(uint dec) {
	return 5 + (dec + 1) / 2;
}

uint my_timestamp_binary_length(uint dec) {
	return 4 + (dec + 1) / 2;
}

uint max_display_length_for_field(enum_field_types sql_type, uint metadata) {
  switch (sql_type) {
	case MYSQL_TYPE_NEWDECIMAL:
		return metadata >> 8;
	case MYSQL_TYPE_FLOAT:
		return 12;
	case MYSQL_TYPE_DOUBLE:
		return 22;
	case MYSQL_TYPE_SET:
	case MYSQL_TYPE_ENUM:
		return metadata & 0x00ff;

	case MYSQL_TYPE_STRING: {
		unsigned char type= metadata >> 8;
		if (type == MYSQL_TYPE_SET || type == MYSQL_TYPE_ENUM)
		return metadata & 0xff;
		else
		return (((metadata >> 4) & 0x300) ^ 0x300) + (metadata & 0x00ff);
	}
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_TINY:
		return 4;
	case MYSQL_TYPE_SHORT:
		return 6;
	case MYSQL_TYPE_INT24:
		return 9;
	case MYSQL_TYPE_LONG:
		return 11;
	case MYSQL_TYPE_LONGLONG:
		return 20;
	case MYSQL_TYPE_NULL:
		return 0;
	case MYSQL_TYPE_NEWDATE:
		return 3;
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_TIME2:
		return 3;
	case MYSQL_TYPE_TIMESTAMP:
	case MYSQL_TYPE_TIMESTAMP2:
		return 4;
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_DATETIME2:
		return 8;
	case MYSQL_TYPE_BIT:
		//BAPI_ASSERT((metadata & 0xff) <= 7);
		return 8 * (metadata >> 8U) + (metadata & 0x00ff);
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_VARCHAR:
		return metadata;
	case MYSQL_TYPE_TINY_BLOB:
		return uint_max(1 * 8);
	case MYSQL_TYPE_MEDIUM_BLOB:
		return uint_max(3 * 8);
	case MYSQL_TYPE_BLOB:
		return uint_max(metadata * 8);
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_GEOMETRY:
	case MYSQL_TYPE_JSON:
		return uint_max(4 * 8);
	default:
		return UINT_MAX;
  }
}

int decimal_binary_size(int precision, int scale) {
	static const int dig2bytes[10]= {0, 1, 1, 2, 2, 3, 3, 4, 4, 4};
	int intg= precision-scale,
		intg0= intg/9, frac0= scale/9,
		intg0x= intg-intg0*9, frac0x= scale-frac0*9;
	return intg0 * sizeof(uint) + dig2bytes[intg0x]+
		frac0 * sizeof(uint) + dig2bytes[frac0x];
}
uint32 calc_field_size(uchar col, const uchar *master_data,
                         uint metadata) {
  uint32 length= 0;
  switch ((col)) {
	case MYSQL_TYPE_NEWDECIMAL:
		length= decimal_binary_size(metadata >> 8, metadata & 0xff);
		break;
	case MYSQL_TYPE_DECIMAL:
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
		length= metadata;
		break;
	case MYSQL_TYPE_SET:
	case MYSQL_TYPE_ENUM:
	case MYSQL_TYPE_STRING:{ 
		unsigned char type= metadata >> 8U;
		if ((type == MYSQL_TYPE_SET) || (type == MYSQL_TYPE_ENUM))
		length= metadata & 0x00ff;
		else {
			length= max_display_length_for_field(MYSQL_TYPE_STRING, metadata) > 255 ? 2 : 1;

			if (length == 1)
				length+= *master_data;
			else {
				uint32_t temp= 0;
				memcpy(&temp, master_data, 2);
				length= length + le32toh(temp);
			}
		}
		break;
	}
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_TINY:
		length= 1;
		break;
	case MYSQL_TYPE_SHORT:
		length= 2;
		break;
	case MYSQL_TYPE_INT24:
		length= 3;
		break;
	case MYSQL_TYPE_LONG:
		length= 4;
		break;
	case MYSQL_TYPE_LONGLONG:
		length= 8;
		break;
	case MYSQL_TYPE_NULL:
		length= 0;
		break;
	case MYSQL_TYPE_NEWDATE:
		length= 3;
		break;
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_TIME:
		length= 3;
		break;
	case MYSQL_TYPE_TIME2:
		length= my_time_binary_length(metadata);
		break;
	case MYSQL_TYPE_TIMESTAMP:
		length= 4;
		break;
	case MYSQL_TYPE_TIMESTAMP2:
		length= my_timestamp_binary_length(metadata);
		break;
	case MYSQL_TYPE_DATETIME:
		length= 8;
		break;
	case MYSQL_TYPE_DATETIME2:
		length= my_datetime_binary_length(metadata);
		break;
	case MYSQL_TYPE_BIT: {
		uint from_len= (metadata >> 8U) & 0x00ff;
		uint from_bit_len= metadata & 0x00ff;
		//BAPI_ASSERT(from_bit_len <= 7);
		length= from_len + ((from_bit_len > 0) ? 1 : 0);
		break;
	}
	case MYSQL_TYPE_VARCHAR: {
		length= metadata > 255 ? 2 : 1;
		if (length == 1)
			length+= (uint32) *master_data;
		else{ 
			uint32 temp= 0;
			memcpy(&temp, master_data, 2);
			length= length + le32toh(temp);
		}
		break;
	}
	case MYSQL_TYPE_TINY_BLOB:
	case MYSQL_TYPE_MEDIUM_BLOB:
	case MYSQL_TYPE_LONG_BLOB:
	case MYSQL_TYPE_BLOB:
	case MYSQL_TYPE_GEOMETRY:
	case MYSQL_TYPE_JSON: {
		switch (metadata) {
			case 1:
			  length= *master_data;
			  break;
			case 2:
			  memcpy(&length, master_data, 2);
			  length= le32toh(length);
			  break;
			case 3:
			  memcpy(&length, master_data, 3);
			  length= le32toh(length);
			  break;
			case 4:
			  memcpy(&length, master_data, 4);
			  length= le32toh(length);
			  break;
			default:
			  //BAPI_ASSERT(0);		// Should not come here
			  break;
		}

		length+= metadata;
		break;
	}
	default:
		length= UINT_MAX;
  }
  return length;
}
