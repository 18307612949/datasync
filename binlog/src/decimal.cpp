#include "decimal.h"
typedef decimal_digit_t dec1;
typedef longlong      dec2;

#define DIG_PER_DEC1 9
#define DIG_MASK     100000000
#define DIG_BASE     1000000000
#define DIG_MAX      (DIG_BASE-1)
#define DIG_BASE2    ((dec2)DIG_BASE * (dec2)DIG_BASE)
#define ROUND_UP(X)  (((X)+DIG_PER_DEC1-1)/DIG_PER_DEC1)
static const dec1 powers10[DIG_PER_DEC1+1]={
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
	static const int dig2bytes[DIG_PER_DEC1+1]={0, 1, 1, 2, 2, 3, 3, 4, 4, 4};
	static const dec1 frac_max[DIG_PER_DEC1-1]={
		900000000, 990000000, 999000000,
		999900000, 999990000, 999999000,
		999999900, 999999990 };

		
#define sanity(d) DBUG_ASSERT((d)->len >0 && ((d)->buf[0] | \
	(d)->buf[(d)->len-1] | 1))
	
#define FIX_INTG_FRAC_ERROR(len, intg1, frac1, error)                   \
	do                                                              \
		{                                                               \
		if (unlikely(intg1+frac1 > (len)))                            \
		{                                                             \
		if (unlikely(intg1 > (len)))                                \
		{                                                           \
		intg1=(len);                                              \
		frac1=0;                                                  \
		error=E_DEC_OVERFLOW;                                     \
		}                                                           \
			else                                                        \
		{                                                           \
		frac1=(len)-intg1;                                        \
		error=E_DEC_TRUNCATED;                                    \
		}                                                           \
		}                                                             \
		  else                                                          \
		  error=E_DEC_OK;                                             \
		} while(0)

#define ADD(to, from1, from2, carry)  /* assume carry <= 1 */           \
	do                                                              \
		{                                                               \
		dec1 a=(from1)+(from2)+(carry);                               \
		DBUG_ASSERT((carry) <= 1);                                    \
		if (((carry)= a >= DIG_BASE)) /* no division here! */         \
		a-=DIG_BASE;                                                \
		(to)=a;                                                       \
		} while(0)

#define ADD2(to, from1, from2, carry)                                   \
	do                                                              \
		{                                                               \
		dec2 a=((dec2)(from1))+(from2)+(carry);                       \
		if (((carry)= a >= DIG_BASE))                                 \
		a-=DIG_BASE;                                                \
		if (unlikely(a >= DIG_BASE))                                  \
		{                                                             \
		a-=DIG_BASE;                                                \
		carry++;                                                    \
		}                                                             \
		(to)=(dec1) a;                                                \
		} while(0)

#define SUB(to, from1, from2, carry) /* to=from1-from2 */               \
	do                                                              \
		{                                                               \
		dec1 a=(from1)-(from2)-(carry);                               \
		if (((carry)= a < 0))                                         \
		a+=DIG_BASE;                                                \
		(to)=a;                                                       \
		} while(0)

#define SUB2(to, from1, from2, carry) /* to=from1-from2 */              \
	do                                                              \
		{                                                               \
		dec1 a=(from1)-(from2)-(carry);                               \
		if (((carry)= a < 0))                                         \
		a+=DIG_BASE;                                                \
		if (unlikely(a < 0))                                          \
		{                                                             \
		a+=DIG_BASE;                                                \
		carry++;                                                    \
		}                                                             \
		(to)=a;                                                       \
		} while(0)


/*
static int do_sub(const decimal_t *from1, const decimal_t *from2, decimal_t *to)
{
	int intg1=ROUND_UP(from1->intg), intg2=ROUND_UP(from2->intg),
		frac1=ROUND_UP(from1->frac), frac2=ROUND_UP(from2->frac);
	int frac0= MY_MAX(frac1, frac2), error;
	dec1 *buf1, *buf2, *buf0, *stop1, *stop2, *start1, *start2, carry=0;

	start1=buf1=from1->buf; stop1=buf1+intg1;
	start2=buf2=from2->buf; stop2=buf2+intg2;
	if (*buf1 == 0)
	{
		while (buf1 < stop1 && *buf1 == 0)
			buf1++;
		start1=buf1;
		intg1= (int) (stop1-buf1);
	}
	if (*buf2 == 0)
	{
		while (buf2 < stop2 && *buf2 == 0)
			buf2++;
		start2=buf2;
		intg2= (int) (stop2-buf2);
	}
	if (intg2 > intg1)
		carry=1;
	else if (intg2 == intg1)
	{
		dec1 *end1= stop1 + (frac1 - 1);
		dec1 *end2= stop2 + (frac2 - 1);
		while ((buf1 <= end1) && (*end1 == 0))
			end1--;
		while ((buf2 <= end2) && (*end2 == 0))
			end2--;
		frac1= (int) (end1 - stop1) + 1;
		frac2= (int) (end2 - stop2) + 1;
		while (buf1 <=end1 && buf2 <= end2 && *buf1 == *buf2)
			buf1++, buf2++;
		if (buf1 <= end1)
		{
			if (buf2 <= end2)
				carry= *buf2 > *buf1;
			else
				carry= 0;
		}
		else
		{
			if (buf2 <= end2)
				carry=1;
			else
			{
				if (to == 0)
					return 0;
				decimal_make_zero(to);
				return E_DEC_OK;
			}
		}
	}

	if (to == 0)
		return carry == from1->sign ? 1 : -1;

	sanity(to);

	to->sign=from1->sign;
	if (carry)
	{
		swap_variables(const decimal_t *, from1, from2);
		swap_variables(dec1 *,start1, start2);
		swap_variables(int,intg1,intg2);
		swap_variables(int,frac1,frac2);
		to->sign= 1 - to->sign;
	}

	FIX_INTG_FRAC_ERROR(to->len, intg1, frac0, error);
	buf0=to->buf+intg1+frac0;

	to->frac= MY_MAX(from1->frac, from2->frac);
	to->intg=intg1*DIG_PER_DEC1;
	if (unlikely(error))
	{
		set_if_smaller(to->frac, frac0*DIG_PER_DEC1);
		set_if_smaller(frac1, frac0);
		set_if_smaller(frac2, frac0);
		set_if_smaller(intg2, intg1);
	}
	carry=0;
	if (frac1 > frac2)
	{
		buf1=start1+intg1+frac1;
		stop1=start1+intg1+frac2;
		buf2=start2+intg2+frac2;
		while (frac0-- > frac1)
			*--buf0=0;
		while (buf1 > stop1)
			*--buf0=*--buf1;
	}
	else
	{
		buf1=start1+intg1+frac1;
		buf2=start2+intg2+frac2;
		stop2=start2+intg2+frac1;
		while (frac0-- > frac2)
			*--buf0=0;
		while (buf2 > stop2)
		{
			SUB(*--buf0, 0, *--buf2, carry);
		}
	}

	while (buf2 > start2)
	{
		SUB(*--buf0, *--buf1, *--buf2, carry);
	}
	while (carry && buf1 > start1)
	{
		SUB(*--buf0, *--buf1, 0, carry);
	}

	while (buf1 > start1)
		*--buf0=*--buf1;

	while (buf0 > to->buf)
		*--buf0=0;

	return error;
}

int decimal_cmp(const decimal_t *from1, const decimal_t *from2)
{
	if (from1->sign == from2->sign)
		return do_sub(from1, from2, 0);

	// Reject negative zero, cfr. internal_str2dec()
	//DBUG_ASSERT(!(decimal_is_zero(from1) && from1->sign));
	//DBUG_ASSERT(!(decimal_is_zero(from2) && from2->sign));
	return from1->sign > from2->sign ? -1 : 1;
}
*/
int decimal_bin_size(int precision, int scale)
{
	int intg=precision-scale,
		intg0=intg/DIG_PER_DEC1, frac0=scale/DIG_PER_DEC1,
		intg0x=intg-intg0*DIG_PER_DEC1, frac0x=scale-frac0*DIG_PER_DEC1;

	//DBUG_ASSERT(scale >= 0 && precision > 0 && scale <= precision);
	//DBUG_ASSERT(intg0x >= 0);
	//DBUG_ASSERT(intg0x <= DIG_PER_DEC1);
	//DBUG_ASSERT(frac0x >= 0);
	//DBUG_ASSERT(frac0x <= DIG_PER_DEC1);
	return intg0*sizeof(dec1)+dig2bytes[intg0x]+
		frac0*sizeof(dec1)+dig2bytes[frac0x];
}

static inline int count_leading_zeroes(int i, dec1 val) {
	int ret= 0;
	switch (i){
		case 9: if (val >= 1000000000) break; ++ret;
		case 8: if (val >= 100000000) break; ++ret;
		case 7: if (val >= 10000000) break; ++ret;
		case 6: if (val >= 1000000) break; ++ret;
		case 5: if (val >= 100000) break; ++ret;
		case 4: if (val >= 10000) break; ++ret;
		case 3: if (val >= 1000) break; ++ret;
		case 2: if (val >= 100) break; ++ret;
		case 1: if (val >= 10) break; ++ret;
		case 0: if (val >= 1) break; ++ret;
		default: { DBUG_ASSERT(FALSE); }
	}
	return ret;
}


static dec1 *remove_leading_zeroes(const decimal_t *from, int *intg_result) {
	int intg= from->intg, i;
	dec1 *buf0= from->buf;
	i= ((intg - 1) % DIG_PER_DEC1) + 1;
	while (intg > 0 && *buf0 == 0)
	{
		intg-= i;
		i= DIG_PER_DEC1;
		buf0++;
	}
	if (intg > 0)
	{
		intg-= count_leading_zeroes((intg - 1) % DIG_PER_DEC1, *buf0);
		DBUG_ASSERT(intg > 0);
	}
	else
		intg=0;
	*intg_result= intg;
	return buf0;
}

int decimal2string(const decimal_t *from, char *to, int *to_len,
                   int fixed_precision, int fixed_decimals,
                   char filler) {
  int len, intg, frac= from->frac, i, intg_len, frac_len, fill;
  int fixed_intg= (fixed_precision ?
                   (fixed_precision - fixed_decimals) : 0);
  int error=E_DEC_OK;
  char *s=to;
  dec1 *buf, *buf0=from->buf, tmp;

  DBUG_ASSERT(*to_len >= 2+from->sign);
  buf0= remove_leading_zeroes(from, &intg);
  if (unlikely(intg+frac==0))
  {
    intg=1;
    tmp=0;
    buf0=&tmp;
  }

  if (!(intg_len= fixed_precision ? fixed_intg : intg))
    intg_len= 1;
  frac_len= fixed_precision ? fixed_decimals : frac;
  len= from->sign + intg_len + MY_TEST(frac) + frac_len;
  if (fixed_precision)
  {
    if (frac > fixed_decimals)
    {
      error= E_DEC_TRUNCATED;
      frac= fixed_decimals;
    }
    if (intg > fixed_intg)
    {
      error= E_DEC_OVERFLOW;
      intg= fixed_intg;
    }
  }
  else if (unlikely(len > --*to_len)) 
  {
    int j= len - *to_len;
    error= (frac && j <= frac + 1) ? E_DEC_TRUNCATED : E_DEC_OVERFLOW;


    if (frac && j >= frac + 1)
      j--;

    if (j > frac)
    {
      intg_len= intg-= j-frac;
      frac= 0;
    }
    else
      frac-=j;
    frac_len= frac;
    len= from->sign + intg_len + MY_TEST(frac) + frac_len;
  }
  *to_len= len;
  s[len]= 0;

  if (from->sign)
    *s++='-';

  if (frac)
  {
    char *s1= s + intg_len;
    fill= frac_len - frac;
    buf=buf0+ROUND_UP(intg);
    *s1++='.';
    for (; frac>0; frac-=DIG_PER_DEC1)
    {
      dec1 x=*buf++;
      for (i= MY_MIN(frac, DIG_PER_DEC1); i; i--)
      {
        dec1 y=x/DIG_MASK;
        *s1++='0'+(uchar)y;
        x-=y*DIG_MASK;
        x*=10;
      }
    }
    for(; fill > 0; fill--)
      *s1++=filler;
  }

  fill= intg_len - intg;
  if (intg == 0)
    fill--; /* symbol 0 before digital point */
  for(; fill > 0; fill--)
    *s++=filler;
  if (intg)
  {
    s+=intg;
    for (buf=buf0+ROUND_UP(intg); intg>0; intg-=DIG_PER_DEC1)
    {
      dec1 x=*--buf;
      for (i= MY_MIN(intg, DIG_PER_DEC1); i; i--)
      {
        dec1 y=x/10;
        *--s='0'+(uchar)(x-y*10);
        x=y;
      }
    }
  }
  else
    *s= '0';

  return error;
}


int bin2decimal(const uchar *from, decimal_t *to, int precision, int scale) {
  int error=E_DEC_OK, intg=precision-scale,
      intg0=intg/DIG_PER_DEC1, frac0=scale/DIG_PER_DEC1,
      intg0x=intg-intg0*DIG_PER_DEC1, frac0x=scale-frac0*DIG_PER_DEC1,
      intg1=intg0+(intg0x>0), frac1=frac0+(frac0x>0);
  dec1 *buf=to->buf, mask=(*from & 0x80) ? 0 : -1;
  const uchar *stop;
  uchar *d_copy;
  int bin_size= decimal_bin_size(precision, scale);

  sanity(to);
  d_copy= (uchar*) alloca(bin_size);
  memcpy(d_copy, from, bin_size);
  d_copy[0]^= 0x80;
  from= d_copy;

  FIX_INTG_FRAC_ERROR(to->len, intg1, frac1, error);
  if (unlikely(error))
  {
    if (intg1 < intg0+(intg0x>0))
    {
      from+=dig2bytes[intg0x]+sizeof(dec1)*(intg0-intg1);
      frac0=frac0x=intg0x=0;
      intg0=intg1;
    }
    else
    {
      frac0x=0;
      frac0=frac1;
    }
  }

  to->sign=(mask != 0);
  to->intg=intg0*DIG_PER_DEC1+intg0x;
  to->frac=frac0*DIG_PER_DEC1+frac0x;

  if (intg0x)
  {
    int i=dig2bytes[intg0x];
    dec1 x= 0;
    switch (i)
    {
      case 1: x=mi_sint1korr(from); break;
      case 2: x=mi_sint2korr(from); break;
      case 3: x=mi_sint3korr(from); break;
      case 4: x=mi_sint4korr(from); break;
      default: DBUG_ASSERT(0);
    }
    from+=i;
    *buf=x ^ mask;
    if (((ulonglong)*buf) >= (ulonglong) powers10[intg0x+1])
      goto err;
    if (buf > to->buf || *buf != 0)
      buf++;
    else
      to->intg-=intg0x;
  }
  for (stop=from+intg0*sizeof(dec1); from < stop; from+=sizeof(dec1))
  {
    DBUG_ASSERT(sizeof(dec1) == 4);
    *buf=mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    if (buf > to->buf || *buf != 0)
      buf++;
    else
      to->intg-=DIG_PER_DEC1;
  }
  DBUG_ASSERT(to->intg >=0);
  for (stop=from+frac0*sizeof(dec1); from < stop; from+=sizeof(dec1))
  {
    DBUG_ASSERT(sizeof(dec1) == 4);
    *buf=mi_sint4korr(from) ^ mask;
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    buf++;
  }
  if (frac0x)
  {
    int i=dig2bytes[frac0x];
    dec1 x= 0;
    switch (i)
    {
      case 1: x=mi_sint1korr(from); break;
      case 2: x=mi_sint2korr(from); break;
      case 3: x=mi_sint3korr(from); break;
      case 4: x=mi_sint4korr(from); break;
      default: DBUG_ASSERT(0);
    }
    *buf=(x ^ mask) * powers10[DIG_PER_DEC1 - frac0x];
    if (((uint32)*buf) > DIG_MAX)
      goto err;
    buf++;
  }
  if (to->intg == 0 && to->frac == 0)
    decimal_make_zero(to);
  return error;

err:
  decimal_make_zero(to);
  return(E_DEC_BAD_NUM);
}