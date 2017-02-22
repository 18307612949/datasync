#include "LogEvent.h"

#include <cstdio>
#include "my_time.h"
#include "my_decimal.h"
uint32 get_field_length(uchar **packet){
	uchar *pos= *packet;
	uint temp= 0;
	if (*pos < 251){
		(*packet)++;
		return  *pos;
	}
	if (*pos == 251){
		(*packet)++;
		return ((uint32) ~0);//NULL_LENGTH;
	}
	if (*pos == 252){
		(*packet)+= 3;
		memcpy(&temp, pos + 1, 2);
		temp= le32toh(temp);
		return (uint32)temp;
	}
	if (*pos == 253){
		(*packet)+= 4;
		memcpy(&temp, pos + 1, 3);
		temp= le32toh(temp);
		return (uint32)temp;
	}
	(*packet)+= 9;                                 /* Must be 254 when here */
	memcpy(&temp, pos + 1, 4);
	temp= le32toh(temp);
	return (uint32)temp;
}

CLogEvent::CLogEvent(void):m_ucStream(NULL)
{
}


CLogEvent::~CLogEvent(void)
{
}
CLogEvent::CLogEvent(uint eventPos, uchar *ucStream):m_event_pos(eventPos), m_ucStream(ucStream){
	m_common_header_length = global_description_event.getEventHeaderLength();
}
enum load_event_state CLogEvent::parseHeader() {
	if(m_ucStream == NULL) {
		return EVENT_STREAM_UNABLED;
	};
	m_timestamp = uint4korr(m_ucStream);
	m_event_type = *(m_ucStream + 4);
	m_server_id = uint4korr(m_ucStream + 5);
	m_event_size = uint4korr(m_ucStream + 9);
	m_next_event_pos = uint4korr(m_ucStream + 13);
	m_comm_flags = uint2korr(m_ucStream + 17);
	return EVENT_SUCCESS;
}

CHeaderEvent::CHeaderEvent():CLogEvent() {

}

CHeaderEvent::CHeaderEvent(uint eventPos, uchar *ucStream):CLogEvent(eventPos, ucStream) {

}

CHeaderEvent::~CHeaderEvent(){

}

enum load_event_state CHeaderEvent::loadEvent(){
	enum load_event_state state;
	if((state = parseHeader()) != EVENT_SUCCESS) {
		return state;
	};

	uint tmpoffset = LOG_EVENT_HEADER_LEN;
	m_binlog_version = uint2korr(m_ucStream + tmpoffset);
	memcpy(m_binlog_szversion, m_ucStream + tmpoffset + 2, ST_SERVER_VER_LEN);
	m_binlog_szversion[ST_SERVER_VER_LEN] = '\0';
	m_create_timestamp = uint4korr(m_ucStream + tmpoffset + 2 + ST_SERVER_VER_LEN);
	m_common_header_length = *(m_ucStream + tmpoffset + 2 + ST_SERVER_VER_LEN + 4);
	
	tmpoffset += 2 + ST_SERVER_VER_LEN + 4 + 1;
	uchar *ueventIndex = m_ucStream + tmpoffset;
	uint eventTypeIndexLen = std::min<uint>(m_event_size - tmpoffset, binary_log::ENUM_END_EVENT-1);
	for(uint i = 0; i < eventTypeIndexLen; i++) {
		m_post_header_len.push_back(ueventIndex[i]);
	}
//	printf("eventsize:%d, %d\n", eventTypeIndexLen, ueventIndex[0]);
	return EVENT_SUCCESS;
};

CQueryEvent::CQueryEvent():CLogEvent(){

}
CQueryEvent::CQueryEvent(uint eventPos, uchar *ucStream):CLogEvent(eventPos, ucStream){
	m_statusvars_length = 0;
}
CQueryEvent::~CQueryEvent(){

}

uint32 CQueryEvent::getThreadId() const{
	return m_thread_id;
}

int16 CQueryEvent::getErrorCode() const{
	return m_error_code;
}
enum load_event_state CQueryEvent::loadEvent() {
	enum load_event_state state;
	if((state = parseHeader()) != EVENT_SUCCESS) {
		return state;
	};
	uint8 postHeaderLen = global_description_event.m_post_header_len[m_event_type - 1];
	if(m_event_size < (uint)(m_common_header_length + postHeaderLen)) {
		return EVENT_EVENT_LENGTH_TOO_SMALL;
	}
	uint dataLen = m_event_size - m_common_header_length - postHeaderLen;
	uchar *dataBuf = m_ucStream + LOG_EVENT_HEADER_LEN;
	m_thread_id = uint4korr(dataBuf + Q_THREAD_ID_OFFSET);
	m_exec_time = uint4korr(dataBuf + Q_EXEC_TIME_OFFSET);
	m_schema_length = *(dataBuf + Q_DB_LEN_OFFSET);
	m_error_code = uint2korr(dataBuf + Q_ERR_CODE_OFFSET);
	if(postHeaderLen - QUERY_HEADER_MINIMAL_LEN) {
		memcpy(&m_statusvars_length, dataBuf + Q_STATUS_VARS_LEN_OFFSET, sizeof(m_statusvars_length));
		m_statusvars_length= le16toh(m_statusvars_length);
		//m_statusvars_length = uint2korr(dataBuf + Q_STATUS_VARS_LEN_OFFSET);
	}
	else
		m_statusvars_length = 0;

	if (m_statusvars_length > std::min<unsigned long>(dataLen, MAX_SIZE_LOG_EVENT_STATUS)) {
		return EVENT_EVENT_LENGTH_TOO_SMALL;
	}

	dataLen -= m_statusvars_length;
	uchar *payloadBuf = dataBuf + postHeaderLen;
	uchar *payloadEnd = payloadBuf + m_statusvars_length;
	for(uchar *pos = payloadBuf; pos < payloadEnd;) {
	//	printf("%d::", *pos);
		switch(*pos++) {
			case Q_FLAGS2_CODE:
				//flags2_inited= 1;
				//memcpy(&flags2, pos, sizeof(flags2));
				//flags2= le32toh(flags2);
				pos += 4;
				break;
			case Q_SQL_MODE_CODE: {
				//CHECK_SPACE(pos, end, 8);
				//sql_mode_inited= 1;
				ulonglong sql_mode;
				sql_mode= uint8korr(pos);
				//printf("\n%x\n", sql_mode);
				pos += 8;
				break;
			}
			case Q_CATALOG_NZ_CODE: {
				uint8 catalog_len;
				const char *catalog;
				if ((catalog_len= *pos))
					catalog = (const char*) (pos + 1);
				pos += catalog_len + 1;
				//printf("%s\n", catalog);
				break;
			}
			case Q_AUTO_INCREMENT: {
				uint16 auto_increment_increment = uint2korr(pos);
				uint16 auto_increment_offset = uint2korr(pos + 2);
				pos += 4;
				break;
			}
			case Q_CHARSET_CODE: {
				//charset_inited= 1;
				char charset[6];
				memcpy(charset, pos, 6);
				pos += 6;
				break;
			}
			case Q_TIME_ZONE_CODE: {
				uint8 time_zone_len;
				const char* time_zone_str;
				if ((time_zone_len= *pos))
					time_zone_str= (const char*)(pos + 1);
				pos += time_zone_len + 1;
				break;
			}
			case Q_CATALOG_CODE: { /* for 5.0.x where 0<=x<=3 masters */
				uint8 catalog_len;
				const char *catalog;
				if ((catalog_len= *pos))
					catalog= (const char*) (pos+1);
				pos+= catalog_len + 2; // leap over end 0
				break;
			}
			case Q_LC_TIME_NAMES_CODE:
				uint16 lc_time_names_number;
				lc_time_names_number= uint2korr(pos);
				pos+= 2;
				break;
			case Q_CHARSET_DATABASE_CODE:
				uint16 charset_database_number;
				charset_database_number= uint2korr(pos);
				pos+= 2;
				break;
			case Q_TABLE_MAP_FOR_UPDATE_CODE:
				uint16 table_map_for_update;
				table_map_for_update= uint8korr(pos);
				pos+= 8;
				break;
			case Q_MASTER_DATA_WRITTEN_CODE:
				uint32 master_data_written;
				master_data_written= uint4korr(pos);
				pos+= 4;
				break;
			case Q_MICROSECONDS: {
				uint32 temp_usec= 0;
				memcpy(&temp_usec, pos, 3);
				pos+= 3;
				break;
			}
			case Q_INVOKER: {
				uint8 user_len= *pos++;
				const char *user= (const char*)pos;
				if (user_len == 0)
					user= (const char *)"";
				pos+= user_len;

				
				uint8 host_len= *pos++;
				const char *host= (const char*)pos;
				if (host_len == 0)
					host= (const char *)"";
				pos+= host_len;
				break;
			}
			case Q_UPDATED_DB_NAMES: {
				unsigned char i= 0;
				char mts_accessed_db_names[MAX_DBS_IN_EVENT_MTS][NAME_LEN];
				uint8 mts_accessed_dbs= *pos++;
				if (mts_accessed_dbs > MAX_DBS_IN_EVENT_MTS) {
					mts_accessed_dbs= OVER_MAX_DBS_IN_EVENT_MTS;
					break;
				}

				for (i= 0; i < mts_accessed_dbs && pos < payloadBuf + m_statusvars_length; i++) {
					strncpy(mts_accessed_db_names[i], (char*) pos,
					std::min<unsigned long>(NAME_LEN, payloadBuf + m_statusvars_length - pos));
					mts_accessed_db_names[i][NAME_LEN - 1]= 0;
					pos+= 1 + strlen((const char*) pos);
				}
				//return EVENT_SUCCESS;
				break;
			}
			case Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
				*pos++;
				break;
			}

		}
	}

	m_schema_name = std::string((char*)payloadEnd);
	char *query = (char*)(payloadEnd + m_schema_length + 1);
	
//	int len = m_event_size - ((uchar*)query - m_ucStream);

//	int queryLen = dataLen - m_schema_length - 1;
//	query[queryLen-4] = '\0';
//	printf("%s,%d\n", query,len);

	return EVENT_SUCCESS;
};


CTableMapEvent::CTableMapEvent():CLogEvent(){

}
CTableMapEvent::CTableMapEvent(uint eventPos, uchar *ucStream):CLogEvent(eventPos, ucStream) {

}

CTableMapEvent::~CTableMapEvent(){

}

enum load_event_state CTableMapEvent::loadEvent() {
	enum load_event_state state;
	if((state = parseHeader()) != EVENT_SUCCESS) {
		return state;
	};
	uint8 postHeaderLen = global_description_event.m_post_header_len[m_event_type - 1];
	if(m_event_size < (uint)(m_common_header_length + postHeaderLen)) {
		return EVENT_EVENT_LENGTH_TOO_SMALL;
	}
	uint dataSize = m_event_size - m_common_header_length;
	uchar *postHeaderStart = m_ucStream + LOG_EVENT_HEADER_LEN;
	postHeaderStart += TM_MAPID_OFFSET;
	if(postHeaderLen == 6) {
		memcpy(&m_table_id, postHeaderStart, 4);
		m_table_id = le64toh(m_table_id);
		postHeaderStart += 4;
	}
	else {
		if(postHeaderLen != TABLE_MAP_HEADER_LEN){
			return EVENT_MAP_HEADER_LEN_ERROR;
		}
		memcpy(&m_table_id, postHeaderStart, 6);
		m_table_id = le64toh(m_table_id);
		postHeaderStart += TM_FLAGS_OFFSET;
	}
	memcpy(&m_table_flags, postHeaderStart, sizeof(m_table_flags));
	m_table_flags= le16toh(m_table_flags);
	
	uchar *payloadStart = m_ucStream + m_common_header_length + postHeaderLen;
	m_db_length = *payloadStart;
	m_table_length = *(payloadStart + m_db_length + 2);
	m_db_name = std::string((const char*)(payloadStart + 1), m_db_length);
	m_table_name = std::string((const char*)(payloadStart + m_db_length + 3), m_table_length);
	
	uchar *ptr_colcnt = payloadStart + m_db_length + m_table_length + 4;
	uchar *ptr_after_colcnt = ptr_colcnt;
	m_column_count = get_field_length(&ptr_after_colcnt);

	m_column_type.resize(m_column_count);
	memcpy((void*)&m_column_type[0], ptr_after_colcnt, m_column_count);

	ptr_after_colcnt += m_column_count;
	uint bytes_read = (ptr_after_colcnt - m_ucStream);
	if(bytes_read < m_event_size) {
		m_field_metadata_size = get_field_length(&ptr_after_colcnt);
		if(m_field_metadata_size > (m_column_count * 2)){
			return EVENT_FIELD_META_SIZE_TOO_BIG;
		}
		uint num_null_bytes = (m_column_count + 7) / 8;
		m_field_metadata.resize(m_field_metadata_size);
		memcpy((void*)&m_field_metadata[0], ptr_after_colcnt, m_field_metadata_size);
		ptr_after_colcnt += m_field_metadata_size;
		m_null_bits.resize(num_null_bytes);
		memcpy((void*)&m_null_bits[0], ptr_after_colcnt, num_null_bytes);
	}
	return EVENT_SUCCESS;
};

CTableDef* CTableMapEvent::createTableDef() {
	return new CTableDef(m_column_type, m_column_count, m_field_metadata, m_field_metadata_size, m_null_bits, m_table_flags);
}
CRowsEvent::CRowsEvent():CLogEvent(){

}
CRowsEvent::CRowsEvent(uint eventPos, uchar *ucStream):CLogEvent(eventPos, ucStream){
	memset(&m_cols, 0, sizeof(m_cols));
	memset(&m_cols_ai, 0, sizeof(m_cols_ai));
}
CRowsEvent::~CRowsEvent(){
	std::vector<std::vector<ROW_VALUE> >::iterator it_before;
	std::vector<ROW_VALUE>::iterator item;
	for(it_before = m_row_before_values.begin(); it_before != m_row_before_values.end(); it_before++) {
		for(item = (*it_before).begin(); item != (*it_before).end(); item++) {
			tx_row_value::row_value_free(&(*item));
		}
	}

	std::vector<std::vector<ROW_VALUE> >::iterator it_after;
	for(it_after = m_row_after_values.begin(); it_after != m_row_after_values.end(); it_after++) {
		for(item = (*it_after).begin(); item != (*it_after).end(); item++) {
			tx_row_value::row_value_free(&(*item));
		}
	}
	/*
	for(it_before = m_row_before_values.begin(); it_before != m_row_before_values.end(); it_before++) {
		tx_row_value::row_value_free(&(*it_before));
	}
	
	std::vector<ROW_VALUE>::iterator it_after;
	for(it_after = m_row_after_values.begin(); it_after != m_row_after_values.end(); it_after++) {
		tx_row_value::row_value_free(&(*it_after));
	}*/
}

CRowData* CRowsEvent::createRowData(uint32 threadid, cchar* dbname, cchar* tablename) {
	CRowData *pData = new CRowData();
	pData->m_db_name = dbname;
	pData->m_table_name = tablename;
	pData->m_thread_id = threadid;
	pData->m_column_count = m_column_count;
	switch(m_event_type){
		case binary_log::WRITE_ROWS_EVENT:
			pData->m_event_type = 1;
			pData->m_event_type_str = "WRITE_ROWS_EVENT";
			break;
		case binary_log::DELETE_ROWS_EVENT:
			pData->m_event_type = 2;
			pData->m_event_type_str = "DELETE_ROWS_EVENT";
			break;
		case binary_log::UPDATE_ROWS_EVENT:
			pData->m_event_type = 3;
			pData->m_event_type_str = "UPDATE_ROWS_EVENT";
			break;
		default:
			pData->m_event_type = 0;
			pData->m_event_type_str = "UNKNOWN_EVENT";
			break;
	}
	pData->m_timestamp = m_timestamp;
	pData->m_row_before_values = m_row_before_values;
	pData->m_row_after_values = m_row_after_values;
	return pData;
}

uint CRowsEvent::parseOneRowData(std::vector<ROW_VALUE>& row_values, const uchar *ptrData, uint type, uint meta) {
	uint32 length= 0;
	if (type == MYSQL_TYPE_STRING) {
		if (meta >= 256) {
			uint byte0= meta >> 8;
			uint byte1= meta & 0xFF;
			if ((byte0 & 0x30) != 0x30) {
				length= byte1 | (((byte0 & 0x30) ^ 0x30) << 4);
				type= byte0 | 0x30;
			}
			else
				length = meta & 0xFF;
		}
		else
			length= meta;
	}
	uint32 rLen = 0;
	ROW_VALUE rowValueTmp;
	tx_row_value::row_value_init(&rowValueTmp, type);
	if(ptrData == NULL) {
		tx_row_value::row_set_value_null(&rowValueTmp);
		rLen = 0;
	}
	else {
		switch(type){
			case MYSQL_TYPE_LONG: {
				int32 si = sint4korr(ptrData);
				//int32 ui = uint4korr(ptrData);
				tx_row_value::row_set_value(&rowValueTmp, (long long)si);
				//printf("long:%d\n", si);
				rLen = 4;
				break;
			}
			case MYSQL_TYPE_TINY: {
				//my_b_write_sint32_and_uint32(file, (int) (signed char) *ptr,(uint) (unsigned char) *ptr);
				int si = (int)ptrData;
				tx_row_value::row_set_value(&rowValueTmp, (long long)si);
				rLen = 1;
				break;
			}
			case MYSQL_TYPE_SHORT: {
				int32 si= (int32) sint2korr(ptrData);
				//uint32 ui= (uint32) uint2korr(ptrData);
				tx_row_value::row_set_value(&rowValueTmp, (long long)si);
				//my_b_write_sint32_and_uint32(file, si, ui);
				rLen = 2;
				break;
			}
			case MYSQL_TYPE_INT24: {
				int32 si= sint3korr(ptrData);
				//uint32 ui= uint3korr(ptrData);
				//my_b_write_sint32_and_uint32(file, si, ui);
				tx_row_value::row_set_value(&rowValueTmp, (long long)si);
				rLen = 3;
				break;
			}

			case MYSQL_TYPE_LONGLONG: {
				char tmp[64];
				longlong si= sint8korr(ptrData);
				//longlong10_to_str(si, tmp, -10);
				//my_b_printf(file, "%s", tmp);
				/*
				if (si < 0) {
					ulonglong ui= uint8korr(ptrData);
					longlong10_to_str((longlong) ui, tmp, 10);
					my_b_printf(file, " (%s)", tmp);        
				}*/
				//longlong l = uint8korr(ptrData);
				//printf("id:%d\n",l);
				rLen = 8;
				tx_row_value::row_set_value(&rowValueTmp, (long long)si);
				break;
			}
			case MYSQL_TYPE_NEWDECIMAL: {
				uint precision= meta >> 8;
				uint decimals= meta & 0xFF;
				//printf("%d,%d\n", precision, decimals);
	
				uint bin_size= my_decimal_get_binary_size(precision, decimals);
				my_decimal dec;
				binary2my_decimal(E_DEC_FATAL_ERROR, (uchar*) ptrData, &dec, precision, decimals);
				int len= DECIMAL_MAX_STR_LENGTH;
				char buff[DECIMAL_MAX_STR_LENGTH + 1];
				decimal2string(&dec,buff,&len, 0, 0, 0);
				tx_row_value::row_set_value(&rowValueTmp, (const char*)buff, len);
				rLen = bin_size;
				break;
			}

			case MYSQL_TYPE_FLOAT: {
				float fl;
				float4get(&fl, ptrData);
				char tmp[320];
				//sprintf(tmp, "%-20g", (double) fl);
				//my_b_printf(file, "%s", tmp); /* my_snprintf doesn't support %-20g */
				tx_row_value::row_set_value(&rowValueTmp, (double)fl);
				rLen = 4;
				break;
			}

			case MYSQL_TYPE_DOUBLE: {
				//strcpy(typestr, "DOUBLE");
				double dbl;
				float8get(&dbl, ptrData);
				char tmp[320];
				//sprintf(tmp, "%-.20g", dbl); /* my_snprintf doesn't support %-20g */
				//my_b_printf(file, "%s", tmp);
				tx_row_value::row_set_value(&rowValueTmp, (double)dbl);
				rLen = 8;
				break;
			}
			case MYSQL_TYPE_BIT: {
					/* Meta-data: bit_len, bytes_in_rec, 2 bytes */
				uint nbits= ((meta >> 8) * 8) + (meta & 0xFF);
				//my_snprintf(typestr, typestr_length, "BIT(%d)", nbits);
				length= (nbits + 7) / 8;
				//my_b_write_bit(file, ptr, nbits);
				rLen = length;
				break;
			}
			case MYSQL_TYPE_TIMESTAMP: {
				//my_snprintf(typestr, typestr_length, "TIMESTAMP");
				uint32 i32= uint4korr(ptrData);
				//my_b_printf(file, "%d", i32);
				rLen = 4;
				break;
			}
			case MYSQL_TYPE_TIMESTAMP2: {
				//my_snprintf(typestr, typestr_length, "TIMESTAMP(%d)", meta);
				char buf[MAX_DATE_STRING_REP_LENGTH];
				struct timeval tm;
				my_timestamp_from_binary(&tm, ptrData, meta);
				int buflen= my_timeval_to_str(&tm, buf, meta);
				//my_b_write(file, buf, buflen);
				rLen =  my_timestamp_binary_length(meta);
				break;
			}
			case MYSQL_TYPE_DATETIME: {
				size_t d, t;
				uint64 i64= uint8korr(ptrData); /* YYYYMMDDhhmmss */
				d= static_cast<size_t>(i64 / 1000000);
				t= i64 % 1000000;
				ROW_VALUE_TIME valueTime;
				tx_row_value::time_init(valueTime, static_cast<int>(d / 10000), static_cast<int>(d % 10000) / 100, static_cast<int>(d % 100), 
					static_cast<int>(t / 10000), static_cast<int>(t % 10000) / 100, static_cast<int>(t % 100));
				tx_row_value::row_set_value(&rowValueTmp, valueTime);
				rLen = 8;
				break;
			}
			case MYSQL_TYPE_DATETIME2: {
				char buf[MAX_DATE_STRING_REP_LENGTH];
				MYSQL_TIME ltime;
				longlong packed= my_datetime_packed_from_binary(ptrData, meta);
				TIME_from_longlong_datetime_packed(&ltime, packed);
				//int buflen= my_datetime_to_str(&ltime, buf, meta);
				ROW_VALUE_TIME valueTime;
				tx_row_value::time_init(valueTime, ltime.year, ltime.month, ltime.day, ltime.hour, ltime.minute, ltime.second);
				tx_row_value::row_set_value(&rowValueTmp, valueTime);
				rLen = my_datetime_binary_length(meta);
				break;
			}
			case MYSQL_TYPE_TIME: {
				uint32 i32= uint3korr(ptrData);
				ROW_VALUE_TIME valueTime;
				tx_row_value::time_init(valueTime, 0, 0, 0, 
					i32 / 10000, (i32 % 10000) / 100, i32 % 100);
				tx_row_value::row_set_value(&rowValueTmp, valueTime);
				rLen = 3;
				break;
			}
			case MYSQL_TYPE_TIME2: {
				char buf[MAX_DATE_STRING_REP_LENGTH];
				MYSQL_TIME ltime;
				longlong packed= my_time_packed_from_binary(ptrData, meta);
				TIME_from_longlong_time_packed(&ltime, packed);
				int buflen= my_time_to_str(&ltime, buf, meta);

				ROW_VALUE_TIME valueTime;
				tx_row_value::time_init(valueTime, 0, 0, 0, 
					ltime.hour, ltime.minute, ltime.second);
				tx_row_value::row_set_value(&rowValueTmp, valueTime);
				rLen = my_time_binary_length(meta);
				break;
			}
			case MYSQL_TYPE_NEWDATE: {
				uint32 tmp= uint3korr(ptrData);
				int part;
				char buf[11];
				char *pos= &buf[10];
				*pos--=0;	
				part=(int) (tmp & 31);
				*pos--= (char) ('0'+part%10);
				*pos--= (char) ('0'+part/10);
				*pos--= ':';
				part=(int) (tmp >> 5 & 15);
				*pos--= (char) ('0'+part%10);
				*pos--= (char) ('0'+part/10);
				*pos--= ':';
				part=(int) (tmp >> 9);
				*pos--= (char) ('0'+part%10); part/=10;
				*pos--= (char) ('0'+part%10); part/=10;
				*pos--= (char) ('0'+part%10); part/=10;
				*pos=   (char) ('0'+part);
				//my_b_printf(file , "'%s'", buf);
				rLen = 3;
				break;
			}
			case MYSQL_TYPE_YEAR: {
				//my_snprintf(typestr, typestr_length, "YEAR");
			
				uint32 i32= *ptrData;
				tx_row_value::row_set_value(&rowValueTmp, (long long)i32);
				//my_b_printf(file, "%04d", i32+ 1900);
				rLen = 1;
				break;
			}
			case MYSQL_TYPE_ENUM:
				switch (meta & 0xFF) {
					case 1: {
						//my_snprintf(typestr, typestr_length, "ENUM(1 byte)");
					
						//my_b_printf(file, "%d", (int) *ptr);
						int i = (int) *ptrData;
						tx_row_value::row_set_value(&rowValueTmp, (long long)i);
						rLen = 1;
						break;
					}
					case 2: {
						//my_snprintf(typestr, typestr_length, "ENUM(2 bytes)");
						int32 i32= uint2korr(ptrData);
						tx_row_value::row_set_value(&rowValueTmp, (long long)i32);
						//my_b_printf(file, "%d", i32);
						rLen = 2;
						break;
					}
					default:
						//my_b_printf(file, "!! Unknown ENUM packlen=%d", meta & 0xFF); 
						rLen = 0;
						break;
				}
				break;

			case MYSQL_TYPE_SET:
				//my_snprintf(typestr, typestr_length, "SET(%d bytes)", meta & 0xFF);
				//my_b_write_bit(file, ptr , (meta & 0xFF) * 8);
				rLen = meta & 0xFF;
				break;

			case MYSQL_TYPE_BLOB:
				switch (meta) {
					case 1:
						//my_snprintf(typestr, typestr_length, "TINYBLOB/TINYTEXT");
					
						length= *ptrData;
						//my_b_write_quoted(file, ptr + 1, length);
						rLen = length + 1;
						break;
					case 2:
						//my_snprintf(typestr, typestr_length, "BLOB/TEXT");
					
						length= uint2korr(ptrData);
						//my_b_write_quoted(file, ptr + 2, length);
						rLen = length + 2;
						break;
					case 3:
						//my_snprintf(typestr, typestr_length, "MEDIUMBLOB/MEDIUMTEXT");
					
						length= uint3korr(ptrData);
						//my_b_write_quoted(file, ptr + 3, length);
						rLen = length + 3;
						break;
					case 4:
						//my_snprintf(typestr, typestr_length, "LONGBLOB/LONGTEXT");
					
						length= uint4korr(ptrData);
						//my_b_write_quoted(file, ptr + 4, length);
						rLen = length + 4;
						break;
					default:
						//my_b_printf(file, "!! Unknown BLOB packlen=%d", length);
						rLen = 0;
						break;
				}
				break;
			case MYSQL_TYPE_VARCHAR:
			case MYSQL_TYPE_VAR_STRING: {
				length= meta;
				std::string strValue;
				rLen = parseRowDataToString(ptrData, length, strValue);
				tx_row_value::row_set_value(&rowValueTmp, strValue.c_str(), (uint32)strValue.length());
				break; 
			}

			case MYSQL_TYPE_STRING: {
				std::string strValue;
				rLen = parseRowDataToString(ptrData, length, strValue);
				break;
			}
			case MYSQL_TYPE_JSON: {
				length= uint2korr(ptrData);
				tx_row_value::row_set_value(&rowValueTmp, (const char*)(ptrData + meta), (uint32)length);
				//my_b_write_quoted(file, ptr + meta, length);
				rLen = length + meta;
				break;
			}
			default: {
				//undefined mysqltype
				rLen = 0;
				break;
			}
		}
	}
	
	if(rowValueTmp.type_ != 0x00)
		row_values.push_back(rowValueTmp);
	return rLen;
}
enum load_event_state CRowsEvent::loadEvent() {
	enum load_event_state state;
	if((state = parseHeader()) != EVENT_SUCCESS) {
		return state;
	};
	uint8 postHeaderLen = global_description_event.m_post_header_len[m_event_type - 1];
	if(m_event_size < (uint)(m_common_header_length + postHeaderLen)) {
		return EVENT_EVENT_LENGTH_TOO_SMALL;
	}
	const uchar *postHeaderStart = m_ucStream + m_common_header_length + ROWS_MAPID_OFFSET;
	if(postHeaderLen == 6) {
		memcpy(&m_table_id, postHeaderStart, 4);
		m_table_id = le64toh(m_table_id);
		postHeaderStart += 4;
	}
	else {
		memcpy(&m_table_id, postHeaderStart, 6);
		m_table_id = le64toh(m_table_id);
		postHeaderStart += ROWS_FLAGS_OFFSET;
	}

	memcpy(&m_table_flags, postHeaderStart, sizeof(m_table_flags));
	m_table_flags = le16toh(m_table_flags);
	postHeaderStart += 2;

	uint16 extra_header_len = 0;
	if(postHeaderLen == ROWS_HEADER_LEN_V2) {
		memcpy(&extra_header_len, postHeaderStart, sizeof(extra_header_len));
		extra_header_len = le16toh(extra_header_len);
		if(extra_header_len < 2) {
			return EVENT_EXTRA_HEADER_LEN_TOO_SMALL;
		}
		extra_header_len -= 2;
		const uchar *start = postHeaderStart + 2;
		const uchar *end = start + extra_header_len;
		for(const uchar * pos = start; pos < end;) {
			switch(*pos++) {
				case ROWS_V_EXTRAINFO_TAG: {
					if((end - pos) < EXTRA_ROW_INFO_HDR_BYTES) {
						return EVENT_EXTRA_HEADER_LEN_TOO_SMALL;
					}
					uint8 infoLen = pos[EXTRA_ROW_INFO_LEN_OFFSET];
					if((end - pos) < infoLen) {
						return EVENT_EXTRA_HEADER_LEN_TOO_SMALL;
					}
					m_extra_row_data.resize(infoLen);
					memcpy((void*)&m_extra_row_data[0], pos, infoLen);
					pos += infoLen;
					break;
				}
				default:
					pos = end;
			}
		}
	}

	uchar const *const var_start = m_ucStream + m_common_header_length + postHeaderLen + extra_header_len;
	uchar *ptr_after_width = (uchar*)var_start;
	m_column_count = get_field_length(&ptr_after_width);
	uint32 bitsLen= (m_column_count + 7) / 8;
	m_column_before_image.resize(bitsLen);
	memcpy((void*)&m_column_before_image[0], ptr_after_width, bitsLen);
	ptr_after_width += bitsLen;
	bitmap_init(&m_cols, m_column_count <= sizeof(m_bitbuf) * 8?m_bitbuf:NULL, m_column_count);
	if(m_column_before_image.empty() == false) {
		memcpy(m_cols.bitmap, &m_column_before_image[0], bitsLen);
		create_last_word_mask(&m_cols);
	}
	else
		m_cols.bitmap = NULL;

//	m_column_after_image = m_column_before_image;
	m_column_after_image.clear();
	if(getGeneralTypeCode() == binary_log::UPDATE_ROWS_EVENT) {
		m_column_after_image.resize(bitsLen);
		memcpy((void*)&m_column_after_image[0], ptr_after_width, bitsLen);
		ptr_after_width += bitsLen;

		bitmap_init(&m_cols_ai, m_column_count <= sizeof(m_bitbuf_ai) * 8?m_bitbuf:NULL, m_column_count);
		if(m_column_after_image.empty() == false) {
			memcpy(m_cols_ai.bitmap, &m_column_after_image[0], bitsLen);
			create_last_word_mask(&m_cols);
		}
		else
			m_cols_ai.bitmap = NULL;
	}
	m_ptr_rows_data = ptr_after_width;
	m_rows_data_len = m_event_size - (m_ptr_rows_data - m_ucStream);
//	if()
	/*
	uint dataSize = m_event_size - (ptr_rows_data - m_ucStream);
	m_row_data.resize(dataSize + 1);
	memcpy((void*)&m_row_data[0], ptr_rows_data, dataSize + 1);
	*/


//	if(m_table_flags == STMT_END_F)


//	uint dataSize = m_event_size - m_common_header_length;
//	printf("datasize:%d\n", postHeaderLen);
	return EVENT_SUCCESS;
};

enum load_event_state CRowsEvent::parseEvent() {
	if(m_p_tablemap_event == NULL) {
		return EVENT_FIND_TABLEMAP_FAILED;
	};
	CTableDef *pTableDef = m_p_tablemap_event->createTableDef();
	const uchar* ptr_rows_data = m_ptr_rows_data;
	uint32 rowLength = 0;
	int tmpLen = 0;
	uint32 tmpDataLen = 0;
	while(tmpDataLen < m_rows_data_len-4) {
		std::vector<ROW_VALUE> tmp_value;
		rowLength = printOneRow(tmp_value, pTableDef, &m_cols, ptr_rows_data);
		m_row_before_values.push_back(tmp_value);
		ptr_rows_data += rowLength;
		tmpDataLen += rowLength;
	
		if(getGeneralTypeCode() == binary_log::UPDATE_ROWS_EVENT) {
			rowLength = printOneRow(tmp_value, pTableDef, &m_cols, ptr_rows_data);
			m_row_after_values.push_back(tmp_value);
			ptr_rows_data += rowLength;
			tmpDataLen += rowLength;
		}
	}
	memcpy(m_crc32, ptr_rows_data, 4);
	delete pTableDef;
	bitmap_free(&m_cols);
	bitmap_free(&m_cols_ai);
	return EVENT_SUCCESS;
};

uint CRowsEvent::printOneRow(std::vector<ROW_VALUE>& row_values, CTableDef *td, MY_BITMAP *clos_bitmap, const uchar *value) {
	int is_null = 0;
	const uchar *ptr_null_bits = value;
	const uchar *value0 = value;

	value += (bitmap_bits_set(clos_bitmap) + 7) / 8;
	uint null_bit_index= 0;
	for(uint32 i = 0; i < m_column_count; i++) {
		int is_null= (ptr_null_bits[null_bit_index / 8] >> (null_bit_index % 8))  & 0x01;
		if(bitmap_is_set(clos_bitmap, i) == 0)
			continue;
		uint rowSize = parseOneRowData(row_values, is_null?NULL:value, td->type(i), td->field_metadata(i));
		value += rowSize;
		null_bit_index++;
	}
	return value - value0;
}

uint CRowsEvent::parseRowDataToString(const uchar *ptrData, uint32 length, std::string& strOutValue) {
	uint rlen = 0;
	uint16 offset = 0;
	if(length < 256) {
		rlen = *ptrData;
		offset = 1;
	}
	else {
		rlen = uint2korr(ptrData);
		offset = 2;
	}
	char *szTmp = (char*)malloc((rlen + 1) * sizeof(char));
	memcpy((void*)szTmp, (void*)(ptrData+offset), rlen);
	szTmp[rlen] = '\0';
	strOutValue = std::string(szTmp);
	free(szTmp);
	return rlen + offset;
}

CRowWriteEvent::CRowWriteEvent():CRowsEvent(){

}
CRowWriteEvent::CRowWriteEvent(uint eventPos, uchar *ucStream):CRowsEvent(eventPos, ucStream) {

}
CRowWriteEvent::~CRowWriteEvent(){

}

enum load_event_state CRowWriteEvent::loadEvent() {
	return CRowsEvent::loadEvent();
};

CRowDeleteEvent::CRowDeleteEvent():CRowsEvent(){}

CRowDeleteEvent::CRowDeleteEvent(uint eventPos, uchar *ucStream):CRowsEvent(eventPos, ucStream) {

}

CRowDeleteEvent::~CRowDeleteEvent(){}
enum load_event_state CRowDeleteEvent::loadEvent(){
	return CRowsEvent::loadEvent();
};


CRowUpdateEvent::CRowUpdateEvent():CRowsEvent(){}
CRowUpdateEvent::CRowUpdateEvent(uint eventPos, uchar *ucStream):CRowsEvent(eventPos, ucStream) {

}
CRowUpdateEvent::~CRowUpdateEvent(){}
enum load_event_state CRowUpdateEvent::loadEvent(){
	return CRowsEvent::loadEvent();
};