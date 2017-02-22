#pragma once
#include "tx_common.h"
#include "TableDef.h"
#include "my_bitmap.h"
#include "tx_rowvalue.h"
#include "CRowData.h"
using namespace log_state;
/**
   1 byte length, 1 byte format
   Length is total length in bytes, including 2 byte header
   Length values 0 and 1 are currently invalid and reserved.
*/
#define EXTRA_ROW_INFO_LEN_OFFSET 0
#define EXTRA_ROW_INFO_FORMAT_OFFSET 1
#define EXTRA_ROW_INFO_HDR_BYTES 2
#define EXTRA_ROW_INFO_MAX_PAYLOAD (255 - EXTRA_ROW_INFO_HDR_BYTES)

#define ROWS_MAPID_OFFSET    0
#define ROWS_FLAGS_OFFSET    6
#define ROWS_VHLEN_OFFSET    8
#define ROWS_V_TAG_LEN       1
#define ROWS_V_EXTRAINFO_TAG 0
class CLogEvent
{
public:
	CLogEvent(void);
	CLogEvent(uint eventPos, uchar *ucStream);
	virtual enum log_state::load_event_state loadEvent() = 0;
	~CLogEvent(void);
	uint getNextEventPos(){return m_next_event_pos;}
	uint getEventPos(){return m_event_pos;}

protected:
	enum load_event_state parseHeader();
	uint m_timestamp;
	uchar m_event_type;
	uint m_server_id;
	uint m_event_size;
	uint m_next_event_pos;
	uint m_event_pos;
	uint16 m_comm_flags;

	uint8 m_common_header_length;
	uchar *m_ucStream;
public:
	static const int LOG_EVENT_TYPES= (binary_log::ENUM_END_EVENT - 1);
	enum enum_post_header_length{
		QUERY_HEADER_MINIMAL_LEN= (4 + 4 + 1 + 2),
		QUERY_HEADER_LEN=(QUERY_HEADER_MINIMAL_LEN + 2),
		STOP_HEADER_LEN= 0,
		LOAD_HEADER_LEN= (4 + 4 + 4 + 1 +1 + 4),
		START_V3_HEADER_LEN= (2 + ST_SERVER_VER_LEN + 4),
		ROTATE_HEADER_LEN= 8,
		INTVAR_HEADER_LEN= 0,
		CREATE_FILE_HEADER_LEN= 4,
		APPEND_BLOCK_HEADER_LEN= 4,
		EXEC_LOAD_HEADER_LEN= 4,
		DELETE_FILE_HEADER_LEN= 4,
		NEW_LOAD_HEADER_LEN= LOAD_HEADER_LEN,
		RAND_HEADER_LEN= 0,
		USER_VAR_HEADER_LEN= 0,
		FORMAT_DESCRIPTION_HEADER_LEN= (START_V3_HEADER_LEN + 1 + LOG_EVENT_TYPES),
		XID_HEADER_LEN= 0,
		BEGIN_LOAD_QUERY_HEADER_LEN= APPEND_BLOCK_HEADER_LEN,
		ROWS_HEADER_LEN_V1= 8,
		TABLE_MAP_HEADER_LEN= 8,
		EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN= (4 + 4 + 4 + 1),
		EXECUTE_LOAD_QUERY_HEADER_LEN= (QUERY_HEADER_LEN +\
		EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN),
		INCIDENT_HEADER_LEN= 2,
		HEARTBEAT_HEADER_LEN= 0,
		IGNORABLE_HEADER_LEN= 0,
		ROWS_HEADER_LEN_V2= 10,
		TRANSACTION_CONTEXT_HEADER_LEN= 18,
		VIEW_CHANGE_HEADER_LEN= 52,
		XA_PREPARE_HEADER_LEN= 0
	};
};

class CHeaderEvent: public CLogEvent{
public:
	CHeaderEvent();
	CHeaderEvent(uint eventPos, uchar *ucStream);
	~CHeaderEvent();
	void setStream(uchar *ucStream){m_ucStream = ucStream;}
	uint8 getEventHeaderLength(){return m_common_header_length;}
	enum load_event_state loadEvent();

	std::vector<uchar> m_post_header_len;
private:
	uint m_create_timestamp;
	uint16 m_binlog_version;
	char m_binlog_szversion[50];

};

class CQueryEvent: public CLogEvent {
public:
	CQueryEvent();
	CQueryEvent(uint eventPos, uchar *ucStream);
	~CQueryEvent();
	enum load_event_state loadEvent();
	uint32 getThreadId() const;
	int16 getErrorCode() const;
private:
	uint32 m_thread_id;
	uint32 m_exec_time;
	uint8 m_schema_length;
	int16 m_error_code;
	uint16 m_statusvars_length;
	std::string m_schema_name;
	
public:
	enum Query_event_post_header_offset{
		Q_THREAD_ID_OFFSET= 0,
		Q_EXEC_TIME_OFFSET= 4,
		Q_DB_LEN_OFFSET= 8,
		Q_ERR_CODE_OFFSET= 9,
		Q_STATUS_VARS_LEN_OFFSET= 11,
		Q_DATA_OFFSET= QUERY_HEADER_LEN
	};

	enum Query_event_status_vars {
		Q_FLAGS2_CODE= 0,
		Q_SQL_MODE_CODE,
		Q_CATALOG_CODE,
		Q_AUTO_INCREMENT,
		Q_CHARSET_CODE,
		Q_TIME_ZONE_CODE,
		Q_CATALOG_NZ_CODE,
		Q_LC_TIME_NAMES_CODE,
		Q_CHARSET_DATABASE_CODE,
		Q_TABLE_MAP_FOR_UPDATE_CODE,
		Q_MASTER_DATA_WRITTEN_CODE,
		Q_INVOKER,
		Q_UPDATED_DB_NAMES,
		Q_MICROSECONDS,
		Q_COMMIT_TS,
		Q_COMMIT_TS2,
		Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP
	};
};


class CTableMapEvent: public CLogEvent {
public:
	CTableMapEvent();
	CTableMapEvent(uint eventPos, uchar *ucStream);
	~CTableMapEvent();
	enum load_event_state loadEvent();
	CTableDef* createTableDef();
	std::string getTableName(){return m_table_name;}
	std::string getDbName(){return m_db_name;}
	uint32 getTableId(){return m_table_id;}
private:
	uint32 m_table_id;
	uint16 m_table_flags;
	uint8 m_db_length;
	uint8 m_table_length;

	uint32 m_column_count;
	uint32 m_field_metadata_size;
	std::vector<uchar> m_column_type;
	std::vector<uchar> m_field_metadata;
	std::vector<uchar> m_null_bits;

	std::string m_db_name;
	std::string m_table_name;


public:
	enum Table_map_event_offset {
		TM_MAPID_OFFSET= 0,
		TM_FLAGS_OFFSET= 6
	};
	enum {
		TYPE_CODE = binary_log::TABLE_MAP_EVENT
	};
};

class CRowsEvent: public CLogEvent {
public:
	CRowsEvent();
	CRowsEvent(uint eventPos, uchar *ucStream);
	enum load_event_state loadEvent();
	enum load_event_state parseEvent();
//	void setTableDef(CTableDef *tabledef){m_p_tabledef = tabledef;}
	void setTableMap(CTableMapEvent *pEvent){m_p_tablemap_event = pEvent;}
	uint32 getTableId(){return m_table_id;}
	~CRowsEvent();
	virtual binary_log::Log_event_type getGeneralTypeCode() {
		return (binary_log::Log_event_type)TYPE_CODE;
	}

	virtual CRowData* createRowData(uint32 threadid, cchar* dbname, cchar* tablename);

private:
	uint32 m_table_id;
	uint16 m_table_flags;
	uint32 m_column_count;
	std::vector<uchar> m_extra_row_data;
	std::vector<uchar> m_column_before_image;
	std::vector<uchar> m_column_after_image;
	std::vector<uchar> m_row_data;
	uchar m_crc32[4];

	uint32    m_bitbuf[128/(sizeof(uint32)*8)];
	uint32    m_bitbuf_ai[128/(sizeof(uint32)*8)];
	MY_BITMAP m_cols;
	MY_BITMAP m_cols_ai;
	CTableMapEvent *m_p_tablemap_event;

	
	uchar *m_ptr_rows_data;
	uint32 m_rows_data_len;

	uint parseOneRowData(std::vector<ROW_VALUE>& row_values, const uchar *ptrData, uint type, uint meta);
	uint parseRowDataToString(const uchar *ptrData, uint32 length, std::string& strOutValue);
	uint printOneRow(std::vector<ROW_VALUE>& row_values, CTableDef *td, MY_BITMAP *clos_bitmap, const uchar *value);

	
protected:
public:
	std::vector<std::vector<ROW_VALUE> > m_row_before_values;
	std::vector<std::vector<ROW_VALUE> > m_row_after_values;
	enum enum_flag {
		STMT_END_F = (1U << 0),
		NO_FOREIGN_KEY_CHECKS_F = (1U << 1),
		RELAXED_UNIQUE_CHECKS_F = (1U << 2),
		COMPLETE_ROWS_F = (1U << 3)
	};
	enum{
		TYPE_CODE = binary_log::UNKNOWN_EVENT
	};
};

class CRowWriteEvent: public CRowsEvent {
public:
	CRowWriteEvent();
	CRowWriteEvent(uint eventPos, uchar *ucStream);
	~CRowWriteEvent();
	enum load_event_state loadEvent();
private:
	virtual binary_log::Log_event_type getGeneralTypeCode() {
		return (binary_log::Log_event_type)TYPE_CODE;
	}
public:
	enum{
		TYPE_CODE = binary_log::WRITE_ROWS_EVENT
	};
};

class CRowDeleteEvent: public CRowsEvent {
public:
	CRowDeleteEvent();
	CRowDeleteEvent(uint eventPos, uchar *ucStream);
	~CRowDeleteEvent();
	enum load_event_state loadEvent();
private:
	virtual binary_log::Log_event_type getGeneralTypeCode() {
		return (binary_log::Log_event_type)TYPE_CODE;
	}
public:
	enum{
		TYPE_CODE = binary_log::DELETE_ROWS_EVENT
	};
};

class CRowUpdateEvent: public CRowsEvent {
public:
	CRowUpdateEvent();
	CRowUpdateEvent(uint eventPos, uchar *ucStream);
	~CRowUpdateEvent();
	enum load_event_state loadEvent();
private:
	virtual binary_log::Log_event_type getGeneralTypeCode() {
		return (binary_log::Log_event_type)TYPE_CODE;
	}
public:
	enum{
		TYPE_CODE = binary_log::UPDATE_ROWS_EVENT
	};
};