#include "my_decimal.h"
#include <tx_common.h>
#include <time.h>

#ifndef MYSQL_CLIENT
/**
   report result of decimal operation.

   @param mask    bitmask filtering result, most likely E_DEC_FATAL_ERROR
   @param result  decimal library return code (E_DEC_* see include/decimal.h)

   @return
     result
*/
int my_decimal::check_result(uint mask, int result) const
{
  if (result & mask)
  {
    int length= DECIMAL_MAX_STR_LENGTH + 1;
    char strbuff[DECIMAL_MAX_STR_LENGTH + 2];

    switch (result) {
    case E_DEC_TRUNCATED:
      // "Data truncated for column \'%s\' at row %ld"
     // push_warning_printf(current_thd, Sql_condition::SL_WARNING,
      //                    WARN_DATA_TRUNCATED, ER(WARN_DATA_TRUNCATED),
       //                   "", -1L);
      break;
    case E_DEC_OVERFLOW:
      // "Truncated incorrect %-.32s value: \'%-.128s\'"
      decimal2string(this, strbuff, &length, 0, 0, 0);
    //  push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                //          ER_TRUNCATED_WRONG_VALUE,
                 //         ER(ER_TRUNCATED_WRONG_VALUE),
                   //       "DECIMAL", strbuff);
      break;
    case E_DEC_DIV_ZERO:
      // "Division by 0"
    //  push_warning(current_thd, Sql_condition::SL_WARNING,
                //   ER_DIVISION_BY_ZERO, ER(ER_DIVISION_BY_ZERO));
      break;
    case E_DEC_BAD_NUM:
      // "Incorrect %-.32s value: \'%-.128s\' for column \'%.192s\' at row %ld"
      decimal2string(this, strbuff, &length, 0, 0, 0);
    //  push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                   //       ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
                   //       ER(ER_TRUNCATED_WRONG_VALUE_FOR_FIELD),
                     //     "DECIMAL", strbuff, "", -1L);
      break;
    case E_DEC_OOM:
     // my_error(ER_OUT_OF_RESOURCES, MYF(0));
      break;
    default:
      DBUG_ASSERT(0);
	  break;
    }
  }
  return result;
}
/*
int my_decimal2string(uint mask, const my_decimal *d,
                      uint fixed_prec, uint fixed_dec,
                      char filler, String *str)
{
  int length= (fixed_prec
               ? (fixed_prec + ((fixed_prec == fixed_dec) ? 1 : 0) + 1 + 1)
               : my_decimal_string_length(d));
  int result;
  if (str->alloc(length))
    return d->check_result(mask, E_DEC_OOM);
  result= decimal2string((decimal_t*) d, (char*) str->ptr(),
                         &length, (int)fixed_prec, fixed_dec,
                         filler);
  str->length(length);
  str->set_charset(&my_charset_numeric);
  return d->check_result(mask, result);
}


bool
str_set_decimal(uint mask, const my_decimal *val,
                uint fixed_prec, uint fixed_dec, char filler,
                String *str, const CHARSET_INFO *cs)
{
  if (!(cs->state & MY_CS_NONASCII))
  {
    my_decimal2string(mask, val, fixed_prec, fixed_dec, filler, str);
    str->set_charset(cs);
    return FALSE;
  }
  else
  {

    uint errors;
    char buf[DECIMAL_MAX_STR_LENGTH];
    String tmp(buf, sizeof(buf), &my_charset_latin1);
    my_decimal2string(mask, val, fixed_prec, fixed_dec, filler, &tmp);
    return str->copy(tmp.ptr(), tmp.length(), &my_charset_latin1, cs, &errors);
  }
}



int my_decimal2binary(uint mask, const my_decimal *d, uchar *bin, int prec,
		      int scale)
{
  int err1= E_DEC_OK, err2;
  my_decimal rounded;
  my_decimal2decimal(d, &rounded);
  rounded.frac= decimal_actual_fraction(&rounded);
  if (scale < rounded.frac)
  {
    err1= E_DEC_TRUNCATED;
    decimal_round(&rounded, &rounded, scale, HALF_UP);
  }
  err2= decimal2bin(&rounded, bin, prec, scale);
  if (!err2)
    err2= err1;
  return d->check_result(mask, err2);
}




int str2my_decimal(uint mask, const char *from, size_t length,
                   const CHARSET_INFO *charset, my_decimal *decimal_value)
{
  char *end, *from_end;
  int err;
  char buff[STRING_BUFFER_USUAL_SIZE];
  String tmp(buff, sizeof(buff), &my_charset_bin);
  if (charset->mbminlen > 1)
  {
    uint dummy_errors;
    tmp.copy(from, length, charset, &my_charset_latin1, &dummy_errors);
    from= tmp.ptr();
    length=  tmp.length();
    charset= &my_charset_bin;
  }
  from_end= end= (char*) from+length;
  err= string2decimal((char *)from, (decimal_t*) decimal_value, &end);
  if (end != from_end && !err)
  {
    for ( ; end < from_end; end++)
    {
      if (!my_isspace(&my_charset_latin1, *end))
      {
        err= E_DEC_TRUNCATED;
        break;
      }
    }
  }
  check_result_and_overflow(mask, err, decimal_value);
  return err;
}


static my_decimal *lldiv_t2my_decimal(const lldiv_t *lld, bool neg,
                                      my_decimal *dec)
{
  if (int2my_decimal(E_DEC_FATAL_ERROR, lld->quot, FALSE, dec))
    return dec;
  if (lld->rem)
  {
    dec->buf[(dec->intg-1) / 9 + 1]= static_cast<decimal_digit_t>(lld->rem);
    dec->frac= 6;
  }
  if (neg)
    my_decimal_neg(dec);
  return dec;
}


my_decimal *date2my_decimal(const MYSQL_TIME *ltime, my_decimal *dec)
{
  lldiv_t lld;
  lld.quot= ltime->time_type > MYSQL_TIMESTAMP_DATE ?
            TIME_to_ulonglong_datetime(ltime) :
            TIME_to_ulonglong_date(ltime);
  lld.rem= (longlong) ltime->second_part * 1000;
  return lldiv_t2my_decimal(&lld, ltime->neg, dec);
}


my_decimal *time2my_decimal(const MYSQL_TIME *ltime, my_decimal *dec)
{
  lldiv_t lld;
  lld.quot= TIME_to_ulonglong_time(ltime);
  lld.rem= (longlong) ltime->second_part * 1000;
  return lldiv_t2my_decimal(&lld, ltime->neg, dec);
}


my_decimal *timeval2my_decimal(const struct timeval *tm, my_decimal *dec)
{
  lldiv_t lld;
  lld.quot= tm->tv_sec;
  lld.rem= (longlong) tm->tv_usec * 1000;
  return lldiv_t2my_decimal(&lld, 0, dec);
}


void my_decimal_trim(ulong *precision, uint *scale)
{
  if (!(*precision) && !(*scale))
  {
    *precision= 10;
    *scale= 0;
    return;
  }
}


#ifndef DBUG_OFF

#define DIG_PER_DEC1 9
#define ROUND_UP(X)  (((X)+DIG_PER_DEC1-1)/DIG_PER_DEC1)

void
print_decimal(const my_decimal *dec)
{
  int i, end;
  char buff[512], *pos;
  pos= buff;
  pos+= sprintf(buff, "Decimal: sign: %d  intg: %d  frac: %d  { ",
                dec->sign(), dec->intg, dec->frac);
  end= ROUND_UP(dec->frac)+ROUND_UP(dec->intg)-1;
  for (i=0; i < end; i++)
    pos+= sprintf(pos, "%09d, ", dec->buf[i]);
  pos+= sprintf(pos, "%09d }\n", dec->buf[i]);
  fputs(buff, DBUG_FILE);
}
void
print_decimal_buff(const my_decimal *dec, const uchar* ptr, int length)
{
  print_decimal(dec);
  fprintf(DBUG_FILE, "Record: ");
  for (int i= 0; i < length; i++)
  {
    fprintf(DBUG_FILE, "%02X ", (uint)((uchar *)ptr)[i]);
  }
  fprintf(DBUG_FILE, "\n");
}


const char *dbug_decimal_as_string(char *buff, const my_decimal *val)
{
  int length= DECIMAL_MAX_STR_LENGTH + 1;    
  if (!val)
    return "NULL";
  (void)decimal2string((decimal_t*) val, buff, &length, 0,0,0);
  return buff;
}

#endif

*/
#endif /*MYSQL_CLIENT*/
