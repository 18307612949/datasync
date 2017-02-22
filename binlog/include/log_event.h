#pragma once

/* 4 bytes which all binlogs should begin with */
#define BINLOG_MAGIC        "\xfe\x62\x69\x6e"
namespace log_state {
	enum load_event_state{
		EVENT_SUCCESS = 0,
		EVENT_STREAM_UNABLED,
		EVENT_HEADER_FAILED,
		EVENT_EVENT_LENGTH_TOO_SMALL,
		EVENT_MAP_HEADER_LEN_ERROR,
		EVENT_FIELD_META_SIZE_TOO_BIG,
		EVENT_EXTRA_HEADER_LEN_TOO_SMALL,
		EVENT_FIND_TABLEMAP_FAILED,
		EVENT_END = 100
	};
}
