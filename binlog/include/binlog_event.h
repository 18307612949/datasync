#ifndef BINLOG_EVENT_H
#define BINLOG_EVENT_H

#define BIN_LOG_HEADER_SIZE 4U
/*version length*/
#define ST_SERVER_VER_LEN 50
#define LOG_EVENT_MINIMAL_HEADER_LEN 19U

#define MAX_DBS_IN_EVENT_MTS 16
#define OVER_MAX_DBS_IN_EVENT_MTS 254


#ifndef SYSTEM_CHARSET_MBMAXLEN
#define SYSTEM_CHARSET_MBMAXLEN 3
#endif
#ifndef NAME_CHAR_LEN
#define NAME_CHAR_LEN   64                     /* Field/table name length */
#endif
#ifndef NAME_LEN
#define NAME_LEN (NAME_CHAR_LEN*SYSTEM_CHARSET_MBMAXLEN)
#endif


#define LOG_EVENT_BINLOG_IN_USE_F       0x1
/*
   Event header offsets;
   these point to places inside the fixed header.
*/
#define EVENT_TYPE_OFFSET    4
#define SERVER_ID_OFFSET     5
#define EVENT_LEN_OFFSET     9
#define LOG_POS_OFFSET       13
#define FLAGS_OFFSET         17

/** start event post-header (for v3 and v4) */
#define ST_BINLOG_VER_OFFSET  0
#define ST_SERVER_VER_OFFSET  2
#define ST_CREATED_OFFSET     (ST_SERVER_VER_OFFSET + ST_SERVER_VER_LEN)
#define ST_COMMON_HEADER_LEN_OFFSET (ST_CREATED_OFFSET + 4)

#define LOG_EVENT_HEADER_LEN 19U    /* the fixed header length */
#define MAX_SIZE_LOG_EVENT_STATUS (1U + 4          /* type, flags2 */   + \
	1U + 8          /* type, sql_mode */ + \
	1U + 1 + 255    /* type, length, catalog */ + \
	1U + 4          /* type, auto_increment */ + \
	1U + 6          /* type, charset */ + \
	1U + 1 + 255    /* type, length, time_zone */ + \
	1U + 2          /* type, lc_time_names_number */ + \
	1U + 2          /* type, charset_database_number */ + \
	1U + 8          /* type, table_map_for_update */ + \
	1U + 4          /* type, master_data_written */ + \
	/* type, db_1, db_2, ... */  \
	1U + (MAX_DBS_IN_EVENT_MTS * (1 + NAME_LEN)) + \
	3U +            /* type, microseconds */ + \
	1U + 32*3 + 1 + 60 \
	/* type, user_len, user, host_len, host */)
namespace binary_log{
	enum Log_event_type {
		UNKNOWN_EVENT= 0,
		START_EVENT_V3= 1,
		QUERY_EVENT= 2,
		STOP_EVENT= 3,
		ROTATE_EVENT= 4,
		INTVAR_EVENT= 5,
		LOAD_EVENT= 6,
		SLAVE_EVENT= 7,
		CREATE_FILE_EVENT= 8,
		APPEND_BLOCK_EVENT= 9,
		EXEC_LOAD_EVENT= 10,
		DELETE_FILE_EVENT= 11,
		NEW_LOAD_EVENT= 12,
		RAND_EVENT= 13,
		USER_VAR_EVENT= 14,
		FORMAT_DESCRIPTION_EVENT= 15,
		XID_EVENT= 16,
		BEGIN_LOAD_QUERY_EVENT= 17,
		EXECUTE_LOAD_QUERY_EVENT= 18,
		TABLE_MAP_EVENT = 19,
		PRE_GA_WRITE_ROWS_EVENT = 20,
		PRE_GA_UPDATE_ROWS_EVENT = 21,
		PRE_GA_DELETE_ROWS_EVENT = 22,
		WRITE_ROWS_EVENT_V1 = 23,
		UPDATE_ROWS_EVENT_V1 = 24,
		DELETE_ROWS_EVENT_V1 = 25,
		INCIDENT_EVENT= 26,
		HEARTBEAT_LOG_EVENT= 27,
		IGNORABLE_LOG_EVENT= 28,
		ROWS_QUERY_LOG_EVENT= 29,
		WRITE_ROWS_EVENT = 30,
		UPDATE_ROWS_EVENT = 31,
		DELETE_ROWS_EVENT = 32,
		GTID_LOG_EVENT= 33,
		ANONYMOUS_GTID_LOG_EVENT= 34,
		PREVIOUS_GTIDS_LOG_EVENT= 35,
		TRANSACTION_CONTEXT_EVENT= 36,
		VIEW_CHANGE_EVENT= 37,
		XA_PREPARE_LOG_EVENT= 38,
		ENUM_END_EVENT /* end marker */
	};

	class Binary_log_event {
	public:
		static const int LOG_EVENT_TYPES= (ENUM_END_EVENT - 1);
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

}
#endif //BINLOG_EVENT_H