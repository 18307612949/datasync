#include "TableDef.h"
#include "binary_log_funcs.h"


CTableDef::CTableDef(std::vector<uchar> colType, uint32 colSize, std::vector<uchar> &fieldMetadata, uint32 metadataSize, std::vector<uchar> nullBitmap, uint16 flags)
	:m_column_size(colSize), m_field_metadata_size(metadataSize), m_flags(flags), m_null_bits(nullBitmap), m_column_type(colType)
{
	if(m_column_size && metadataSize) {
		int index = 0;
		for (uint i = 0; i < m_column_size; i++) {
			switch(binlog_type(i)){
				case MYSQL_TYPE_TINY_BLOB:
				case MYSQL_TYPE_BLOB:
				case MYSQL_TYPE_MEDIUM_BLOB:
				case MYSQL_TYPE_LONG_BLOB:
				case MYSQL_TYPE_DOUBLE:
				case MYSQL_TYPE_FLOAT:
				case MYSQL_TYPE_GEOMETRY:
				case MYSQL_TYPE_JSON: {
					m_field_metadata.push_back(fieldMetadata[index]);
					index++;
					break;
				}
				case MYSQL_TYPE_SET:
				case MYSQL_TYPE_ENUM:
				case MYSQL_TYPE_STRING: {
					uint16 x= fieldMetadata[index++] << 8U; // real_type
					x += fieldMetadata[index++];            // pack or field length
					m_field_metadata.push_back(x);
					break;
				}
				case MYSQL_TYPE_BIT: {
					uint16 x= fieldMetadata[index++];
					x = x + (fieldMetadata[index++] << 8U);
					m_field_metadata.push_back(x);
					break;
				}
				case MYSQL_TYPE_VARCHAR: {
					char *ptr= (char *)&fieldMetadata[index];
					m_field_metadata.push_back(uint2korr(ptr));
					index= index + 2;
					break;
				}
				case MYSQL_TYPE_NEWDECIMAL: {
					uint16 x= fieldMetadata[index++] << 8U; // precision
					x+= fieldMetadata[index++];            // decimals
					m_field_metadata.push_back(x);
					break;
				}
				case MYSQL_TYPE_TIME2:
				case MYSQL_TYPE_DATETIME2:
				case MYSQL_TYPE_TIMESTAMP2:
					m_field_metadata.push_back(fieldMetadata[index++]);
					break;
				default:
					m_field_metadata.push_back(0);
					break;
			}
		}
	}
//	printf("dd:%d\n", m_field_metadata_size);
	
}


uint32 CTableDef::calc_field_size(uint col, uchar *master_data) const {
	uint32 length= ::calc_field_size(type(col), master_data, m_field_metadata[col]);
	return length;
}


CTableDef::~CTableDef(void) {
}
