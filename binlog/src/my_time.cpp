#include "my_time.h"
ulonglong log_10_int[20]=
{
  1, 10, 100, 1000, 10000UL, 100000UL, 1000000UL, 10000000UL,
  100000000ULL, 1000000000ULL, 10000000000ULL, 100000000000ULL,
  1000000000000ULL, 10000000000000ULL, 100000000000000ULL,
  1000000000000000ULL, 10000000000000000ULL, 100000000000000000ULL,
  1000000000000000000ULL, 10000000000000000000ULL
};


const char my_zero_datetime6[]= "0000-00-00 00:00:00.000000";


/* Position for YYYY-DD-MM HH-MM-DD.FFFFFF AM in default format */

static uchar internal_format_positions[]=
{0, 1, 2, 3, 4, 5, 6, (uchar) 255};

static char time_separator=':';

static uint32 const days_at_timestart=719528;	/* daynr at 1970.01.01 */
uchar days_in_month[]= {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0};

static long my_time_zone=0;
uint calc_days_in_year(uint year) {
	return ((year & 3) == 0 && (year%100 || (year%400 == 0 && year)) ?
		366 : 365);
}

void set_zero_time(MYSQL_TIME *tm, enum enum_mysql_timestamp_type time_type) {
	memset(tm, 0, sizeof(*tm));
	tm->time_type= time_type;
}

void set_max_hhmmss(MYSQL_TIME *tm){
	tm->hour= TIME_MAX_HOUR;
	tm->minute= TIME_MAX_MINUTE;
	tm->second= TIME_MAX_SECOND;
}

void set_max_time(MYSQL_TIME *tm, my_bool neg) {
	set_zero_time(tm, MYSQL_TIMESTAMP_TIME);
	set_max_hhmmss(tm);
	tm->neg= neg;
}

my_bool check_date(const MYSQL_TIME *ltime, my_bool not_zero_date, my_time_flags_t flags, int *was_cut) {
	if (not_zero_date) {
		if (((flags & TIME_NO_ZERO_IN_DATE) || !(flags & TIME_FUZZY_DATE)) &&
			(ltime->month == 0 || ltime->day == 0)) {
			*was_cut= MYSQL_TIME_WARN_ZERO_IN_DATE;
			return TRUE;
		}
		else if ((!(flags & TIME_INVALID_DATES) &&
			ltime->month && ltime->day > days_in_month[ltime->month-1] &&
			(ltime->month != 2 || calc_days_in_year(ltime->year) != 366 ||
			ltime->day != 29))) {
			*was_cut= MYSQL_TIME_WARN_OUT_OF_RANGE;
			return TRUE;
		}
	}
	else if (flags & TIME_NO_ZERO_DATE) {
		*was_cut= MYSQL_TIME_WARN_ZERO_DATE;
		return TRUE;
	}
	return FALSE;
}

my_bool check_time_mmssff_range(const MYSQL_TIME *ltime) {
	return ltime->minute >= 60 || ltime->second >= 60 ||
		ltime->second_part > 999999;
}

my_bool check_time_range_quick(const MYSQL_TIME *ltime){
	longlong hour= (longlong) ltime->hour + 24LL * ltime->day;
	/* The input value should not be fatally bad */
	//DBUG_ASSERT(!check_time_mmssff_range(ltime));
	if (hour <= TIME_MAX_HOUR &&
		(hour != TIME_MAX_HOUR || ltime->minute != TIME_MAX_MINUTE ||
		ltime->second != TIME_MAX_SECOND || !ltime->second_part))
		return FALSE;
	return TRUE;
}

my_bool check_datetime_range(const MYSQL_TIME *ltime) {
  /*
    In case of MYSQL_TIMESTAMP_TIME hour value can be up to TIME_MAX_HOUR.
    In case of MYSQL_TIMESTAMP_DATETIME it cannot be bigger than 23.
  */
  return
    ltime->year > 9999U || ltime->month > 12U  || ltime->day > 31U || 
    ltime->minute > 59U || ltime->second > 59U || ltime->second_part > 999999U ||
    (ltime->hour >
     (ltime->time_type == MYSQL_TIMESTAMP_TIME ? TIME_MAX_HOUR : 23U));
}

#define MAX_DATE_PARTS 8

my_bool str_to_datetime(const char *str, size_t length, MYSQL_TIME *l_time,
                my_time_flags_t flags, MYSQL_TIME_STATUS *status) {
					/*
  uint field_length= 0, year_length= 0, digits, i, number_of_fields;
  uint date[MAX_DATE_PARTS], date_len[MAX_DATE_PARTS];
  uint add_hours= 0, start_loop;
  uint32 not_zero_date, allow_space;
  my_bool is_internal_format;
  const char *pos, *last_field_pos= NULL;
  const char *end=str+length;
  const uchar *format_position;
  my_bool found_delimitier= 0, found_space= 0;
  uint frac_pos, frac_len;
  //DBUG_ENTER("str_to_datetime");
  //DBUG_PRINT("ENTER", ("str: %.*s", (int)length, str));

  my_time_status_init(status);

  for (; str != end && my_isspace(&my_charset_latin1, *str) ; str++)
    ;
  if (str == end || ! my_isdigit(&my_charset_latin1, *str)){
    status->warnings= MYSQL_TIME_WARN_TRUNCATED;
    l_time->time_type= MYSQL_TIMESTAMP_NONE;
    return 1;
  }

  is_internal_format= 0;

  format_position= internal_format_positions;

 
  for (pos=str;
       pos != end && (my_isdigit(&my_charset_latin1,*pos) || *pos == 'T');
       pos++)
    ;

  digits= (uint) (pos-str);
  start_loop= 0;                                
  date_len[format_position[0]]= 0;              
  if (pos == end || *pos == '.')
  {
   
    year_length= (digits == 4 || digits == 8 || digits >= 14) ? 4 : 2;
    field_length= year_length;
    is_internal_format= 1;
    format_position= internal_format_positions;
  }
  else
  {
    if (format_position[0] >= 3)                
    {
    
      while (pos < end && !my_isspace(&my_charset_latin1, *pos))
        pos++;
      while (pos < end && !my_isdigit(&my_charset_latin1, *pos))
        pos++;
      if (pos == end)
      {
        if (flags & TIME_DATETIME_ONLY)
        {
          status->warnings= MYSQL_TIME_WARN_TRUNCATED;
          l_time->time_type= MYSQL_TIMESTAMP_NONE;
          return 1; 
        }
        
        date[0]= date[1]= date[2]= date[3]= date[4]= 0;
        start_loop= 5;                         
      }
    }
	
    field_length= format_position[0] == 0 ? 4 : 2;
  }

  i= MY_MAX((uint) format_position[0], (uint) format_position[1]);
  set_if_bigger(i, (uint) format_position[2]);
  allow_space= ((1 << i) | (1 << format_position[6]));
  allow_space&= (1 | 2 | 4 | 8 | 64);

  not_zero_date= 0;
  for (i = start_loop;
       i < MAX_DATE_PARTS-1 && str != end &&
         my_isdigit(&my_charset_latin1,*str);
       i++)
  {
    const char *start= str;
    uint32 tmp_value= (uint) (uchar) (*str++ - '0');

    my_bool     scan_until_delim= !is_internal_format &&
                                  ((i != format_position[6]));

    while (str != end && my_isdigit(&my_charset_latin1,str[0]) &&
           (scan_until_delim || --field_length))
    {
      tmp_value=tmp_value*10 + (uint32) (uchar) (*str - '0');
      str++;
    }
    date_len[i]= (uint) (str - start);
    if (tmp_value > 999999)                    
    {
      status->warnings= MYSQL_TIME_WARN_TRUNCATED;
      l_time->time_type= MYSQL_TIMESTAMP_NONE;
      return 1;
    }
    date[i]=tmp_value;
    not_zero_date|= tmp_value;

  
    field_length= format_position[i+1] == 0 ? 4 : 2;

    if ((last_field_pos= str) == end)
    {
      i++;                                     
      break;
    }
    
    if (i == format_position[2] && *str == 'T')
    {
      str++;                                  
      continue;
    }
    if (i == format_position[5])                
    {
      if (*str == '.')                         
      {
        str++;
      
        last_field_pos= str;
        field_length= 6;                       
      }
      else if (my_isdigit(&my_charset_latin1,str[0]))
      {

        i++;
        break;
      }
      continue;
    }
    while (str != end &&
           (my_ispunct(&my_charset_latin1,*str) ||
            my_isspace(&my_charset_latin1,*str)))
    {
      if (my_isspace(&my_charset_latin1,*str))
      {
        if (!(allow_space & (1 << i)))
        {
          status->warnings= MYSQL_TIME_WARN_TRUNCATED;
          l_time->time_type= MYSQL_TIMESTAMP_NONE;
          return 1;
        }
        found_space= 1;
      }
      str++;
      found_delimitier= 1;                     
    }
   
    if (i == format_position[6])               
    {
      i++;                                      
      if (format_position[7] != 255)            
      {
        if (str+2 <= end && (str[1] == 'M' || str[1] == 'm'))
        {
          if (str[0] == 'p' || str[0] == 'P')
            add_hours= 12;
          else if (str[0] != 'a' || str[0] != 'A')
            continue;                           
          str+= 2;                              
         
          while (str != end && my_isspace(&my_charset_latin1,*str))
            str++;
        }
      }
    }
    last_field_pos= str;
  }
  if (found_delimitier && !found_space && (flags & TIME_DATETIME_ONLY))
  {
    status->warnings= MYSQL_TIME_WARN_TRUNCATED;
    l_time->time_type= MYSQL_TIMESTAMP_NONE;
    return 1;  
  }

  str= last_field_pos;

  number_of_fields= i - start_loop;
  while (i < MAX_DATE_PARTS)
  {
    date_len[i]= 0;
    date[i++]= 0;
  }

  if (!is_internal_format)
  {
    year_length= date_len[(uint) format_position[0]];
    if (!year_length)                          
    {
      status->warnings= MYSQL_TIME_WARN_TRUNCATED;
      l_time->time_type= MYSQL_TIMESTAMP_NONE;
      return 1;
    }

    l_time->year=               date[(uint) format_position[0]];
    l_time->month=              date[(uint) format_position[1]];
    l_time->day=                date[(uint) format_position[2]];
    l_time->hour=               date[(uint) format_position[3]];
    l_time->minute=             date[(uint) format_position[4]];
    l_time->second=             date[(uint) format_position[5]];

    frac_pos= (uint) format_position[6];
    frac_len= date_len[frac_pos];
    status->fractional_digits= frac_len;
    if (frac_len < 6)
      date[frac_pos]*= (uint) log_10_int[DATETIME_MAX_DECIMALS - frac_len];
    l_time->second_part= date[frac_pos];

    if (format_position[7] != (uchar) 255)
    {
      if (l_time->hour > 12)
      {
        status->warnings= MYSQL_TIME_WARN_TRUNCATED;
        goto err;
      }
      l_time->hour= l_time->hour%12 + add_hours;
    }
  }
  else
  {
    l_time->year=       date[0];
    l_time->month=      date[1];
    l_time->day=        date[2];
    l_time->hour=       date[3];
    l_time->minute=     date[4];
    l_time->second=     date[5];
    if (date_len[6] < 6)
      date[6]*= (uint) log_10_int[DATETIME_MAX_DECIMALS - date_len[6]];
    l_time->second_part=date[6];
    status->fractional_digits= date_len[6];
  }
  l_time->neg= 0;

  if (year_length == 2 && not_zero_date)
    l_time->year+= (l_time->year < YY_PART_YEAR ? 2000 : 1900);

  
  l_time->time_type= (number_of_fields <= 3 ?
                      MYSQL_TIMESTAMP_DATE : MYSQL_TIMESTAMP_DATETIME);

  if (number_of_fields < 3 || check_datetime_range(l_time))
  {
   
    if (!not_zero_date)                         
    {
      for (; str != end ; str++)
      {
        if (!my_isspace(&my_charset_latin1, *str))
        {
          not_zero_date= 1;                    
          break;
        }
      }
    }
    status->warnings|= not_zero_date ? MYSQL_TIME_WARN_TRUNCATED :
                                       MYSQL_TIME_WARN_ZERO_DATE;
    goto err;
  }

  if (check_date(l_time, not_zero_date != 0, flags, &status->warnings))
    goto err;

 
  if (status->fractional_digits == 6 && str != end)
  {
    if (my_isdigit(&my_charset_latin1, *str))
    {
      
      status->nanoseconds= 100 * (int) (*str++ - '0');
      for (; str != end && my_isdigit(&my_charset_latin1, *str); str++)
      { }
    }
  }

  for (; str != end ; str++)
  {
    if (!my_isspace(&my_charset_latin1,*str))
    {
      status->warnings= MYSQL_TIME_WARN_TRUNCATED;
      break;
    }
  }

  return 0;

err:
  set_zero_time(l_time, MYSQL_TIMESTAMP_ERROR);*/
  return 1;
}

my_bool str_to_time(const char *str, size_t length, MYSQL_TIME *l_time,
	MYSQL_TIME_STATUS *status)
{/*
	uint date[5];
	ulonglong value;
	const char *end=str+length, *end_of_days;
	my_bool found_days,found_hours;
	uint state;

	my_time_status_init(status);
	l_time->neg=0;
	for (; str != end && my_isspace(&my_charset_latin1,*str) ; str++)
		length--;
	if (str != end && *str == '-')
	{
		l_time->neg=1;
		str++;
		length--;
	}
	if (str == end)
		return 1;


	if (length >= 12)
	{                                            
		(void) str_to_datetime(str, length, l_time,
			(TIME_FUZZY_DATE | TIME_DATETIME_ONLY), status);
		if (l_time->time_type >= MYSQL_TIMESTAMP_ERROR)
			return l_time->time_type == MYSQL_TIMESTAMP_ERROR;
		my_time_status_init(status);
	}


	for (value=0; str != end && my_isdigit(&my_charset_latin1,*str) ; str++)
		value=value*10L + (long) (*str - '0');

	if (value > UINT_MAX)
		return 1;

	
	end_of_days= str;
	for (; str != end && my_isspace(&my_charset_latin1, str[0]) ; str++)
		;

	state= 0;
	found_days=found_hours=0;
	if ((uint) (end-str) > 1 && str != end_of_days &&
		my_isdigit(&my_charset_latin1, *str))
	{                                            
		date[0]= (uint) value;
		state= 1;                                   
		found_days= 1;
	}
	else if ((end-str) > 1 &&  *str == time_separator &&
		my_isdigit(&my_charset_latin1, str[1]))
	{
		date[0]= 0;                                 
		date[1]= (uint) value;
		state=2;
		found_hours=1;
		str++;                                     
	}
	else
	{
	
		date[0]= 0;
		date[1]= (uint) (value/10000);
		date[2]= (uint) (value/100 % 100);
		date[3]= (uint) (value % 100);
		state=4;
		goto fractional;
	}


	for (;;)
	{
		for (value=0; str != end && my_isdigit(&my_charset_latin1,*str) ; str++)
			value=value*10L + (long) (*str - '0');
		date[state++]= (uint32) value;
		if (state == 4 || (end-str) < 2 || *str != time_separator ||
			!my_isdigit(&my_charset_latin1,str[1]))
			break;
		str++;                                     
	}

	if (state != 4)
	{                                         

		if (!found_hours && !found_days)
		{
			size_t len= sizeof(long) * (state - 1);
			memmove((uchar*) (date+4) - len, (uchar*) (date+state) - len, len);
			memset(date, 0, sizeof(long)*(4-state));
		}
		else
			memset((date+state), 0, sizeof(long)*(4-state));
	}

fractional:

	if ((end-str) >= 2 && *str == '.' && my_isdigit(&my_charset_latin1,str[1]))
	{
		int field_length= 5;
		str++; value=(uint) (uchar) (*str - '0');
		while (++str != end && my_isdigit(&my_charset_latin1, *str))
		{
			if (field_length-- > 0)
				value= value*10 + (uint) (uchar) (*str - '0');
		}
		if (field_length >= 0)
		{
			status->fractional_digits= DATETIME_MAX_DECIMALS - field_length;
			if (field_length > 0)
				value*= (long) log_10_int[field_length];
		}
		else
		{
	
			status->fractional_digits= 6;
			status->nanoseconds= 100 * (int) (str[-1] - '0');
			for ( ; str != end && my_isdigit(&my_charset_latin1, *str); str++)
			{ }
		}
		date[4]= (uint32) value;
	}
	else if ((end - str) == 1 && *str == '.')
	{
		str++;
		date[4]= 0;
	}
	else
		date[4]=0;


	if ((end - str) > 1 &&
		(*str == 'e' || *str == 'E') &&
		(my_isdigit(&my_charset_latin1, str[1]) ||
		((str[1] == '-' || str[1] == '+') &&
		(end - str) > 2 &&
		my_isdigit(&my_charset_latin1, str[2]))))
		return 1;

	if (internal_format_positions[7] != 255)
	{
	
		while (str != end && my_isspace(&my_charset_latin1, *str))
			str++;
		if (str+2 <= end && (str[1] == 'M' || str[1] == 'm'))
		{
			if (str[0] == 'p' || str[0] == 'P')
			{
				str+= 2;
				date[1]= date[1]%12 + 12;
			}
			else if (str[0] == 'a' || str[0] == 'A')
				str+=2;
		}
	}


	if (date[0] > UINT_MAX || date[1] > UINT_MAX ||
		date[2] > UINT_MAX || date[3] > UINT_MAX ||
		date[4] > UINT_MAX)
		return 1;

	l_time->year=         0;                      
	l_time->month=        0;

	l_time->day=  0;
	l_time->hour= date[1] + date[0] * 24; 

	l_time->minute=       date[2];
	l_time->second=       date[3];
	l_time->second_part=  date[4];
	l_time->time_type= MYSQL_TIMESTAMP_TIME;

	if (check_time_mmssff_range(l_time))
	{
		status->warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
		return TRUE;
	}

	
	adjust_time_range(l_time, &status->warnings);

	
	if (str != end)
	{
		do
		{
			if (!my_isspace(&my_charset_latin1,*str))
			{
				status->warnings|= MYSQL_TIME_WARN_TRUNCATED;
				break;
			}
		} while (++str != end);
	}*/
	return 0;
}

my_bool	number_to_time(longlong nr, MYSQL_TIME *ltime, int *warnings) {
	if (nr > TIME_MAX_VALUE)
	{
		/* For huge numbers try full DATETIME, like str_to_time does. */
		if (nr >= 10000000000LL) /* '0001-00-00 00-00-00' */
		{
			int warnings_backup= *warnings;
			if (number_to_datetime(nr, ltime, 0, warnings) != -1LL)
				return FALSE;
			*warnings= warnings_backup;
		}
		set_max_time(ltime, 0);
		*warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
		return TRUE;
	}
	else if (nr < -TIME_MAX_VALUE)
	{
		set_max_time(ltime, 1);
		*warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
		return TRUE;
	}
	if ((ltime->neg= (nr < 0)))
		nr= -nr;
	if (nr % 100 >= 60 || nr / 100 % 100 >= 60) /* Check hours and minutes */
	{
		set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
		*warnings|= MYSQL_TIME_WARN_OUT_OF_RANGE;
		return TRUE;
	}
	ltime->time_type= MYSQL_TIMESTAMP_TIME;
	ltime->year= ltime->month= ltime->day= 0;
	TIME_set_hhmmss(ltime, (uint)nr);
	ltime->second_part= 0;
	return FALSE;
}

void adjust_time_range(struct st_mysql_time *my_time, int *warning) 
{
	//DBUG_ASSERT(!check_time_mmssff_range(my_time));
	if (check_time_range_quick(my_time))
	{
		my_time->day= my_time->second_part= 0;
		set_max_hhmmss(my_time);
		*warning|= MYSQL_TIME_WARN_OUT_OF_RANGE;
	}
}

static inline struct tm *localtime_r(const time_t *timep, struct tm *tmp) {
	//localtime_s(tmp, timep);
	return tmp;
}
void my_init_time(void) {
	time_t seconds;
	struct tm *l_time,tm_tmp;
	MYSQL_TIME my_time;
	my_bool not_used;

	seconds= (time_t) time((time_t*) 0);
	localtime_r(&seconds,&tm_tmp);
	l_time= &tm_tmp;
	my_time_zone=		3600;		/* Comp. for -3600 in my_gmt_sec */
	my_time.year=		(uint) l_time->tm_year+1900;
	my_time.month=	(uint) l_time->tm_mon+1;
	my_time.day=		(uint) l_time->tm_mday;
	my_time.hour=		(uint) l_time->tm_hour;
	my_time.minute=	(uint) l_time->tm_min;
	my_time.second=	(uint) l_time->tm_sec;
	my_time.time_type= MYSQL_TIMESTAMP_DATETIME;
	my_time.neg= 0;
	my_time.second_part= 0;
	my_system_gmt_sec(&my_time, &my_time_zone, &not_used); /* Init my_time_zone */
}

uint year_2000_handling(uint year)
{
	if ((year=year+1900) < 1900+YY_PART_YEAR)
		year+=100;
	return year;
}

long calc_daynr(uint year,uint month,uint day) {
	long delsum;
	int temp;
	int y= year;                                  /* may be < 0 temporarily */
	//DBUG_ENTER("calc_daynr");

	if (y == 0 && month == 0)
		return 0;				/* Skip errors */
	/* Cast to int to be able to handle month == 0 */
	delsum= (long) (365 * y + 31 *((int) month - 1) + (int) day);
	if (month <= 2)
		y--;
	else
		delsum-= (long) ((int) month * 4 + 23) / 10;
	temp=(int) ((y/100+1)*3)/4;
	//DBUG_PRINT("exit",("year: %d  month: %d  day: %d -> daynr: %ld",
		//y+(month <= 2),month,day,delsum+y/4-temp));
	//DBUG_ASSERT(delsum+(int) y/4-temp >= 0);
	return delsum+(int) y/4-temp;
} /* calc_daynr */

my_time_t
my_system_gmt_sec(const MYSQL_TIME *t_src, long *my_timezone,
                  my_bool *in_dst_time_gap)
{
  uint loop;
  time_t tmp= 0;
  int shift= 0;
  MYSQL_TIME tmp_time;
  MYSQL_TIME *t= &tmp_time;
  struct tm *l_time,tm_tmp;
  long diff, current_timezone;
  memcpy(&tmp_time, t_src, sizeof(MYSQL_TIME));

  if (!validate_timestamp_range(t))
    return 0;
  if ((t->year == TIMESTAMP_MAX_YEAR) && (t->month == 1) && (t->day > 4))
  {
    t->day-= 2;
    shift= 2;
  }

  tmp= (time_t) (((calc_daynr((uint) t->year, (uint) t->month, (uint) t->day) -
                   (long) days_at_timestart) * SECONDS_IN_24H +
                   (long) t->hour*3600L +
                  (long) (t->minute*60 + t->second)) + (time_t) my_time_zone -
                 3600);

  current_timezone= my_time_zone;
  localtime_r(&tmp,&tm_tmp);
  l_time=&tm_tmp;
  for (loop=0;
       loop < 2 &&
	 (t->hour != (uint) l_time->tm_hour ||
	  t->minute != (uint) l_time->tm_min ||
          t->second != (uint) l_time->tm_sec);
       loop++)
  {					/* One check should be enough ? */
    int days= t->day - l_time->tm_mday;
    if (days < -1)
      days= 1;					/* Month has wrapped */
    else if (days > 1)
      days= -1;
    diff=(3600L*(long) (days*24+((int) t->hour - (int) l_time->tm_hour)) +
          (long) (60*((int) t->minute - (int) l_time->tm_min)) +
          (long) ((int) t->second - (int) l_time->tm_sec));
    current_timezone+= diff+3600;		/* Compensate for -3600 above */
    tmp+= (time_t) diff;
    localtime_r(&tmp,&tm_tmp);
    l_time=&tm_tmp;
  }
  if (loop == 2 && t->hour != (uint) l_time->tm_hour)
  {
    int days= t->day - l_time->tm_mday;
    if (days < -1)
      days=1;					/* Month has wrapped */
    else if (days > 1)
      days= -1;
    diff=(3600L*(long) (days*24+((int) t->hour - (int) l_time->tm_hour))+
	  (long) (60*((int) t->minute - (int) l_time->tm_min)) +
          (long) ((int) t->second - (int) l_time->tm_sec));
    if (diff == 3600)
      tmp+=3600 - t->minute*60 - t->second;	/* Move to next hour */
    else if (diff == -3600)
      tmp-=t->minute*60 + t->second;		/* Move to previous hour */

    *in_dst_time_gap= 1;
  }
  *my_timezone= current_timezone;


  tmp+= shift * SECONDS_IN_24H;
  if (!IS_TIME_T_VALID_FOR_TIMESTAMP(tmp))
    tmp= 0;

  return (my_time_t) tmp;
}


static inline int my_useconds_to_str(char *to, uint32 useconds, uint dec)
{
	//DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
	return sprintf(to, ".%0*lu", (int) dec,
		useconds / (uint32) log_10_int[DATETIME_MAX_DECIMALS - dec]);
}

int my_time_to_str(const MYSQL_TIME *l_time, char *to, uint dec)
{
	uint extra_hours= 0;
	int len= sprintf(to, "%s%02u:%02u:%02u", (l_time->neg ? "-" : ""),
		extra_hours + l_time->hour, l_time->minute, l_time->second);
	if (dec)
		len+= my_useconds_to_str(to + len, l_time->second_part, dec);
	return len;
}

int my_date_to_str(const MYSQL_TIME *l_time, char *to)
{
	return sprintf(to, "%04u-%02u-%02u",
		l_time->year, l_time->month, l_time->day);
}

static inline int TIME_to_datetime_str(char *to, const MYSQL_TIME *ltime)
{
	uint32 temp, temp2;
	/* Year */
	temp= ltime->year / 100;
	*to++= (char) ('0' + temp / 10);
	*to++= (char) ('0' + temp % 10);
	temp= ltime->year % 100;
	*to++= (char) ('0' + temp / 10);
	*to++= (char) ('0' + temp % 10);
	*to++= '-';
	/* Month */
	temp= ltime->month;
	temp2= temp / 10;
	temp= temp-temp2 * 10;
	*to++= (char) ('0' + (char) (temp2));
	*to++= (char) ('0' + (char) (temp));
	*to++= '-';
	/* Day */ 
	temp= ltime->day;
	temp2= temp / 10;
	temp= temp - temp2 * 10;
	*to++= (char) ('0' + (char) (temp2));
	*to++= (char) ('0' + (char) (temp));
	*to++= ' ';
	/* Hour */
	temp= ltime->hour;
	temp2= temp / 10;
	temp= temp - temp2 * 10;
	*to++= (char) ('0' + (char) (temp2));
	*to++= (char) ('0' + (char) (temp));
	*to++= ':';
	/* Minute */
	temp= ltime->minute;
	temp2= temp / 10;
	temp= temp - temp2 * 10;
	*to++= (char) ('0' + (char) (temp2));
	*to++= (char) ('0' + (char) (temp));
	*to++= ':';
	/* Second */
	temp= ltime->second;
	temp2=temp / 10;
	temp= temp - temp2 * 10;
	*to++= (char) ('0' + (char) (temp2));
	*to++= (char) ('0' + (char) (temp));
	return 19;
}

int my_datetime_to_str(const MYSQL_TIME *l_time, char *to, uint dec) {
	int len= TIME_to_datetime_str(to, l_time);
	if (dec)
		len+= my_useconds_to_str(to + len, l_time->second_part, dec);
	else
		to[len]= '\0';
	return len;
}

int my_TIME_to_str(const MYSQL_TIME *l_time, char *to, uint dec) {
	switch (l_time->time_type) {
	case MYSQL_TIMESTAMP_DATETIME:
		return my_datetime_to_str(l_time, to, dec);
	case MYSQL_TIMESTAMP_DATE:
		return my_date_to_str(l_time, to);
	case MYSQL_TIMESTAMP_TIME:
		return my_time_to_str(l_time, to, dec);
	case MYSQL_TIMESTAMP_NONE:
	case MYSQL_TIMESTAMP_ERROR:
		to[0]='\0';
		return 0;
	default:
		return 0;
	}
}

int my_timeval_to_str(const struct timeval *tm, char *to, uint dec) {
	int len= sprintf(to, "%d", (int)tm->tv_sec);
	if (dec)
		len+= my_useconds_to_str(to + len, tm->tv_usec, dec);
	return len;
}

longlong number_to_datetime(longlong nr, MYSQL_TIME *time_res,
                            my_time_flags_t flags, int *was_cut){
  long part1,part2;

  *was_cut= 0;
  memset(time_res, 0, sizeof(*time_res));
  time_res->time_type=MYSQL_TIMESTAMP_DATE;

  if (nr == 0LL || nr >= 10000101000000LL)
  {
    time_res->time_type=MYSQL_TIMESTAMP_DATETIME;
    if (nr > 99999999999999LL) /* 9999-99-99 99:99:99 */
    {
      *was_cut= MYSQL_TIME_WARN_OUT_OF_RANGE;
      return -1LL;
    }
    goto ok;
  }
  if (nr < 101)
    goto err;
  if (nr <= (YY_PART_YEAR-1)*10000L+1231L)
  {
    nr= (nr+20000000L)*1000000L;                 /* YYMMDD, year: 2000-2069 */
    goto ok;
  }
  if (nr < (YY_PART_YEAR)*10000L+101L)
    goto err;
  if (nr <= 991231L)
  {
    nr= (nr+19000000L)*1000000L;                 /* YYMMDD, year: 1970-1999 */
    goto ok;
  }
  if (nr < 10000101L && !(flags & TIME_FUZZY_DATE))
    goto err;
  if (nr <= 99991231L)
  {
    nr= nr*1000000L;
    goto ok;
  }
  if (nr < 101000000L)
    goto err;

  time_res->time_type=MYSQL_TIMESTAMP_DATETIME;

  if (nr <= (YY_PART_YEAR-1)*10000000000LL+1231235959LL)
  {
    nr= nr+20000000000000LL;                   /* YYMMDDHHMMSS, 2000-2069 */
    goto ok;
  }
  if (nr <  YY_PART_YEAR*10000000000LL+ 101000000LL)
    goto err;
  if (nr <= 991231235959LL)
    nr= nr+19000000000000LL;		/* YYMMDDHHMMSS, 1970-1999 */

 ok:
  part1=(long) (nr/1000000LL);
  part2=(long) (nr - (longlong) part1*1000000LL);
  time_res->year=  (int) (part1/10000L);  part1%=10000L;
  time_res->month= (int) part1 / 100;
  time_res->day=   (int) part1 % 100;
  time_res->hour=  (int) (part2/10000L);  part2%=10000L;
  time_res->minute=(int) part2 / 100;
  time_res->second=(int) part2 % 100;

  if (!check_datetime_range(time_res) &&
      !check_date(time_res, (nr != 0), flags, was_cut))
    return nr;

  /* Don't want to have was_cut get set if TIME_NO_ZERO_DATE was violated. */
  if (!nr && (flags & TIME_NO_ZERO_DATE))
    return -1LL;

 err:
  *was_cut= MYSQL_TIME_WARN_TRUNCATED;
  return -1LL;
}



ulonglong TIME_to_ulonglong_datetime(const MYSQL_TIME *my_time)
{
  return ((ulonglong) (my_time->year * 10000UL +
                       my_time->month * 100UL +
                       my_time->day) * 1000000ULL +
          (ulonglong) (my_time->hour * 10000UL +
                       my_time->minute * 100UL +
                       my_time->second));
}




ulonglong TIME_to_ulonglong_date(const MYSQL_TIME *my_time)
{
  return (ulonglong) (my_time->year * 10000UL + my_time->month * 100UL +
                      my_time->day);
}



ulonglong TIME_to_ulonglong_time(const MYSQL_TIME *my_time)
{
  return (ulonglong) (my_time->hour * 10000UL +
                      my_time->minute * 100UL +
                      my_time->second);
}



void TIME_set_yymmdd(MYSQL_TIME *ltime, uint yymmdd)
{
  ltime->day=   (int) (yymmdd % 100);
  ltime->month= (int) (yymmdd / 100) % 100;
  ltime->year=  (int) (yymmdd / 10000);
}



void TIME_set_hhmmss(MYSQL_TIME *ltime, uint hhmmss)
{
  ltime->second=  (int) (hhmmss % 100);
  ltime->minute=  (int) (hhmmss / 100) % 100;
  ltime->hour=    (int) (hhmmss / 10000);
}




ulonglong TIME_to_ulonglong(const MYSQL_TIME *my_time)
{
  switch (my_time->time_type) {
  case MYSQL_TIMESTAMP_DATETIME:
    return TIME_to_ulonglong_datetime(my_time);
  case MYSQL_TIMESTAMP_DATE:
    return TIME_to_ulonglong_date(my_time);
  case MYSQL_TIMESTAMP_TIME:
    return TIME_to_ulonglong_time(my_time);
  case MYSQL_TIMESTAMP_NONE:
  case MYSQL_TIMESTAMP_ERROR:
    return 0ULL;
  default:
	  return 0;
    //DBUG_ASSERT(0);
  }
  return 0;
}


longlong TIME_to_longlong_time_packed(const MYSQL_TIME *ltime) {
  /* If month is 0, we mix day with hours: "1 00:10:10" -> "24:00:10" */
  long hms= (((ltime->month ? 0 : ltime->day * 24) + ltime->hour) << 12) |
            (ltime->minute << 6) | ltime->second;
  longlong tmp= MY_PACKED_TIME_MAKE(hms, ltime->second_part);
  return ltime->neg ? -tmp : tmp;
}


void TIME_from_longlong_time_packed(MYSQL_TIME *ltime, longlong tmp)
{
  longlong hms;
  if ((ltime->neg= (tmp < 0)))
    tmp= -tmp;
  hms= MY_PACKED_TIME_GET_INT_PART(tmp);
  ltime->year=   (uint) 0;
  ltime->month=  (uint) 0;
  ltime->day=    (uint) 0;
  ltime->hour=   (uint) (hms >> 12) % (1 << 10); /* 10 bits starting at 12th */
  ltime->minute= (uint) (hms >> 6)  % (1 << 6);  /* 6 bits starting at 6th   */
  ltime->second= (uint)  hms        % (1 << 6);  /* 6 bits starting at 0th   */
  ltime->second_part= MY_PACKED_TIME_GET_FRAC_PART(tmp);
  ltime->time_type= MYSQL_TIMESTAMP_TIME;
}


#define TIMEF_OFS 0x800000000000LL
#define TIMEF_INT_OFS 0x800000LL

void my_time_packed_to_binary(longlong nr, uchar *ptr, uint dec)
{
	//DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
	/* Make sure the stored value was previously properly rounded or truncated */
	//DBUG_ASSERT((MY_PACKED_TIME_GET_FRAC_PART(nr) % 
	//	(int) log_10_int[DATETIME_MAX_DECIMALS - dec]) == 0);

	switch (dec)
	{
	case 0:
	default:
		mi_int3store(ptr, TIMEF_INT_OFS + MY_PACKED_TIME_GET_INT_PART(nr));
		break;

	case 1:
	case 2:
		mi_int3store(ptr, TIMEF_INT_OFS + MY_PACKED_TIME_GET_INT_PART(nr));
		ptr[3]= (unsigned char) (char) (MY_PACKED_TIME_GET_FRAC_PART(nr) / 10000);
		break;

	case 4:
	case 3:
		mi_int3store(ptr, TIMEF_INT_OFS + MY_PACKED_TIME_GET_INT_PART(nr));
		mi_int2store(ptr + 3, MY_PACKED_TIME_GET_FRAC_PART(nr) / 100);
		break;

	case 5:
	case 6:
		mi_int6store(ptr, nr + TIMEF_OFS);
		break;
	}
}

longlong my_time_packed_from_binary(const uchar *ptr, uint dec)
{
 // DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);

  switch (dec)
  {
  case 0:
  default:
    {
      longlong intpart= mi_uint3korr(ptr) - TIMEF_INT_OFS;
      return MY_PACKED_TIME_MAKE_INT(intpart);
    }
  case 1:
  case 2:
    {
      longlong intpart= mi_uint3korr(ptr) - TIMEF_INT_OFS;
      int frac= (uint) ptr[3];
      if (intpart < 0 && frac)
      {
        intpart++;    /* Shift to the next integer value */
        frac-= 0x100; /* -(0x100 - frac) */
      }
      return MY_PACKED_TIME_MAKE(intpart, frac * 10000);
    }

  case 3:
  case 4:
    {
      longlong intpart= mi_uint3korr(ptr) - TIMEF_INT_OFS;
      int frac= mi_uint2korr(ptr + 3);
      if (intpart < 0 && frac)
      {
        /*
          Fix reverse fractional part order: "0x10000 - frac".
          See comments for FSP=1 and FSP=2 above.
        */
        intpart++;      /* Shift to the next integer value */
        frac-= 0x10000; /* -(0x10000-frac) */
      }
      return MY_PACKED_TIME_MAKE(intpart, frac * 100);
    }

  case 5:
  case 6:
    return ((longlong) mi_uint6korr(ptr)) - TIMEF_OFS;
  }
}

longlong TIME_to_longlong_datetime_packed(const MYSQL_TIME *ltime)
{
	longlong ymd= ((ltime->year * 13 + ltime->month) << 5) | ltime->day;
	longlong hms= (ltime->hour << 12) | (ltime->minute << 6) | ltime->second;
	longlong tmp= MY_PACKED_TIME_MAKE(((ymd << 17) | hms), ltime->second_part);
	//DBUG_ASSERT(!check_datetime_range(ltime)); /* Make sure no overflow */
	return ltime->neg ? -tmp : tmp;
}

longlong TIME_to_longlong_date_packed(const MYSQL_TIME *ltime)
{
	longlong ymd= ((ltime->year * 13 + ltime->month) << 5) | ltime->day;
	return MY_PACKED_TIME_MAKE_INT(ymd << 17);
}


longlong year_to_longlong_datetime_packed(long year)
{
  longlong ymd= ((year * 13) << 5);
  return MY_PACKED_TIME_MAKE_INT(ymd << 17);
}


void TIME_from_longlong_datetime_packed(MYSQL_TIME *ltime, longlong tmp)
{
  longlong ymd, hms;
  longlong ymdhms, ym;
  if ((ltime->neg= (tmp < 0)))
    tmp= -tmp;

  ltime->second_part= MY_PACKED_TIME_GET_FRAC_PART(tmp);
  ymdhms= MY_PACKED_TIME_GET_INT_PART(tmp);

  ymd= ymdhms >> 17;
  ym= ymd >> 5;
  hms= ymdhms % (1 << 17);

  ltime->day= ymd % (1 << 5);
  ltime->month= ym % 13;
  ltime->year= (uint)(ym / 13);

  ltime->second= hms % (1 << 6);
  ltime->minute= (hms >> 6) % (1 << 6);
  ltime->hour= (uint)(hms >> 12);
  
  ltime->time_type= MYSQL_TIMESTAMP_DATETIME;
}

void TIME_from_longlong_date_packed(MYSQL_TIME *ltime, longlong tmp)
{
  TIME_from_longlong_datetime_packed(ltime, tmp);
  ltime->time_type= MYSQL_TIMESTAMP_DATE;
}


#define DATETIMEF_INT_OFS 0x8000000000LL

longlong my_datetime_packed_from_binary(const uchar *ptr, uint dec)
{
  longlong intpart= mi_uint5korr(ptr) - DATETIMEF_INT_OFS;
  int frac;
  //DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  switch (dec)
  {
  case 0:
  default:
    return MY_PACKED_TIME_MAKE_INT(intpart);
  case 1:
  case 2:
    frac= ((int) (signed char) ptr[5]) * 10000;
    break;
  case 3:
  case 4:
    frac= mi_sint2korr(ptr + 5) * 100;
    break;
  case 5:
  case 6:
    frac= mi_sint3korr(ptr + 5);
    break;
  }
  return MY_PACKED_TIME_MAKE(intpart, frac);
}

void my_datetime_packed_to_binary(longlong nr, uchar *ptr, uint dec)
{
 // DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  /* The value being stored must have been properly rounded or truncated */
  //DBUG_ASSERT((MY_PACKED_TIME_GET_FRAC_PART(nr) %
    //          (int) log_10_int[DATETIME_MAX_DECIMALS - dec]) == 0);

  mi_int5store(ptr, MY_PACKED_TIME_GET_INT_PART(nr) + DATETIMEF_INT_OFS);
  switch (dec)
  {
  case 0:
  default:
    break;
  case 1:
  case 2:
    ptr[5]= (unsigned char) (char) (MY_PACKED_TIME_GET_FRAC_PART(nr) / 10000);
    break;
  case 3:
  case 4:
    mi_int2store(ptr + 5, MY_PACKED_TIME_GET_FRAC_PART(nr) / 100);
    break;
  case 5:
  case 6:
    mi_int3store(ptr + 5, MY_PACKED_TIME_GET_FRAC_PART(nr));
  }
}


void my_timestamp_from_binary(struct timeval *tm, const uchar *ptr, uint dec)
{
  //DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  tm->tv_sec= mi_uint4korr(ptr);
  switch (dec)
  {
    case 0:
    default:
      tm->tv_usec= 0;
      break;
    case 1:
    case 2:
      tm->tv_usec= ((int) ptr[4]) * 10000;
      break;
    case 3:
    case 4:
      tm->tv_usec= mi_sint2korr(ptr + 4) * 100;
      break;
    case 5:
    case 6:
      tm->tv_usec= mi_sint3korr(ptr + 4);
  }
}


void my_timestamp_to_binary(const struct timeval *tm, uchar *ptr, uint dec)
{
  //DBUG_ASSERT(dec <= DATETIME_MAX_DECIMALS);
  /* Stored value must have been previously properly rounded or truncated */
  //DBUG_ASSERT((tm->tv_usec %
    //           (int) log_10_int[DATETIME_MAX_DECIMALS - dec]) == 0);
  mi_int4store(ptr, tm->tv_sec);
  switch (dec)
  {
    case 0:
    default:
      break;
    case 1:
    case 2:
      ptr[4]= (unsigned char) (char) (tm->tv_usec / 10000);
      break;
    case 3:
    case 4:
      mi_int2store(ptr + 4, tm->tv_usec / 100);
      break;
      /* Impossible second precision. Fall through */
    case 5:
    case 6:
      mi_int3store(ptr + 4, tm->tv_usec);
  }
}


longlong TIME_to_longlong_packed(const MYSQL_TIME *ltime)
{
  switch (ltime->time_type) {
  case MYSQL_TIMESTAMP_DATE:
    return TIME_to_longlong_date_packed(ltime);
  case MYSQL_TIMESTAMP_DATETIME:
    return TIME_to_longlong_datetime_packed(ltime);
  case MYSQL_TIMESTAMP_TIME:
    return TIME_to_longlong_time_packed(ltime);   
  case MYSQL_TIMESTAMP_NONE:
  case MYSQL_TIMESTAMP_ERROR:
    return 0;
  }
  //DBUG_ASSERT(0);
  return 0;
}
