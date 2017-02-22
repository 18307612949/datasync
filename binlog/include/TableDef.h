#pragma once
#include "tx_common.h"
#include "binary_log_types.h"
class CTableDef
{
public:
	CTableDef(std::vector<uchar> colType, uint32 colSize, std::vector<uchar> &fieldMetadata, uint32 metadataSize, std::vector<uchar> nullBitmap, uint16 flags);
	~CTableDef(void);
	uint32 size(){return m_column_size;}
	uint32 calc_field_size(uint col, uchar *master_data) const;

private:
	uint32 m_column_size;
	uint m_field_metadata_size;
	uint16 m_flags;

	std::vector<uchar> m_column_type;
	std::vector<uchar> m_null_bits;
	std::vector<uint16> m_field_metadata;

public:
	enum_field_types binlog_type(uint32 index) const {
		return static_cast<enum_field_types>(m_column_type[index]);
	}

	enum_field_types type(uint32 index) const {
		enum_field_types source_type= binlog_type(index);
		uint16 source_metadata= m_field_metadata[index];
		switch (source_type) {
			case MYSQL_TYPE_STRING: {
				int real_type= source_metadata >> 8;
				if (real_type == MYSQL_TYPE_ENUM || real_type == MYSQL_TYPE_SET)
					source_type= static_cast<enum_field_types>(real_type);
				break;
			}
			case MYSQL_TYPE_DATE:
				source_type= MYSQL_TYPE_NEWDATE;
				break;

			default:
				/* Do nothing */
			break;
		}
		return source_type;
	}

	bool maybe_null(uint index) const {;
		return ((m_null_bits[(index / 8)] & 
			(1 << (index % 8))) == (1 << (index %8)));
	}

	uint16 field_metadata(uint index) const {
		if (m_field_metadata_size)
			return m_field_metadata[index];
		else
			return 0;
	}
};

