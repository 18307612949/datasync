
#ifndef BINARY_LOG_FUNCS_INCLUDED
#define BINARY_LOG_FUNCS_INCLUDED
#include "tx_common.h"
#include "binary_log_types.h"
#if __cplusplus > 201100L
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

uint my_time_binary_length(uint dec);
uint my_datetime_binary_length(uint dec);
uint my_timestamp_binary_length(uint dec);
uint32 calc_field_size(uchar column_type, const uchar *field_ptr,
                         uint metadata);

uint max_display_length_for_field(enum_field_types sql_type,
                                          uint metadata);

int decimal_binary_size(int precision, int scale);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* BINARY_LOG_FUNCS_INCLUDED */