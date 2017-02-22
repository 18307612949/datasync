#ifndef ROW_DATA_H
#define ROW_DATA_H
#include <iostream>
#include <deque>
#include <vector>
#include "tx_ctype.h"
#include "tx_rowvalue.h"
#include <export.h>
using namespace std;
class CRowData {
public:
	uint32 m_column_count;
	uint32 m_thread_id;
	uint16 m_event_type;
	uint m_timestamp;
	string m_event_type_str;
	string m_table_name;
	string m_db_name;
	vector<vector<ROW_VALUE> > m_row_before_values;
	vector<vector<ROW_VALUE> > m_row_after_values;
};

extern bool productRowData(CRowData *data);
extern DLL_SAMPLE_API CRowData *consumerRowData();
#endif //ROW_DATA_H