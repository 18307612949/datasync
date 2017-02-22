#ifndef MY_COMPILER_INCLUDED
#define MY_COMPILER_INCLUDED
#include <stddef.h> 

#if defined __GNUC__
#  define MY_GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#  define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#else
#  define MY_GNUC_PREREQ(maj, min) (0)
#endif


#ifdef HAVE_BUILTIN_EXPECT

#if defined(__cplusplus)
inline bool likely(bool expr)
{
  return __builtin_expect(expr, true);
}
inline bool unlikely(bool expr)
{
  return __builtin_expect(expr, false);
}
#else
#  define likely(x)    __builtin_expect((x),1)
#  define unlikely(x)  __builtin_expect((x),0)
#endif

#else  /* HAVE_BUILTIN_EXPECT */

#if defined(__cplusplus)
inline bool likely(bool expr)
{
  return expr;
}
inline bool unlikely(bool expr)
{
  return expr;
}
#else
#  define likely(x)    (x)
#  define unlikely(x)  (x)
#endif

#endif  /* HAVE_BUILTIN_EXPECT */

#ifdef HAVE_BUILTIN_UNREACHABLE
#  define MY_ASSERT_UNREACHABLE()   __builtin_unreachable()
#else
#  define MY_ASSERT_UNREACHABLE()  do { assert(0); } while (0)
#endif

#if defined __GNUC__ || defined __SUNPRO_C || defined __SUNPRO_CC
# define MY_ALIGNOF(type)   __alignof__(type)
# define MY_ALIGNED(n)      __attribute__((__aligned__((n))))
#elif defined _MSC_VER
# define MY_ALIGNOF(type)   __alignof(type)
# define MY_ALIGNED(n)      __declspec(align(n))
#else /* Make sure they are defined for other compilers. */
# define MY_ALIGNOF(type)
# define MY_ALIGNED(size)
#endif

/* Visual Studio requires '__inline' for C code */
#if !defined(__cplusplus) && defined(_MSC_VER)
# define inline __inline
#endif

/* Provide __func__ macro definition for Visual Studio. */
#if defined(_MSC_VER)
#  define __func__ __FUNCTION__
#endif

/**
  C++ Type Traits
*/
#ifdef __cplusplus

/**
  Opaque storage with a particular alignment.
  Partial specialization used due to MSVC++.
*/
template<size_t alignment> struct my_alignment_imp;
template<> struct MY_ALIGNED(1) my_alignment_imp<1> {};
template<> struct MY_ALIGNED(2) my_alignment_imp<2> {};
template<> struct MY_ALIGNED(4) my_alignment_imp<4> {};
template<> struct MY_ALIGNED(8) my_alignment_imp<8> {};
template<> struct MY_ALIGNED(16) my_alignment_imp<16> {};

/**
  A POD type with a given size and alignment.

  @remark If the compiler does not support a alignment attribute
          (MY_ALIGN macro), the default alignment of a double is
          used instead.

  @tparam size        The minimum size.
  @tparam alignment   The desired alignment: 1, 2, 4, 8 or 16.
*/
template <size_t size, size_t alignment>
struct my_aligned_storage
{
  union
  {
    char data[size];
    my_alignment_imp<alignment> align;
  };
};

#endif /* __cplusplus */

/*
  Disable __attribute__ for Sun Studio and Visual Studio.
  Note that Sun Studio supports some __attribute__ variants,
  but not format or unused which we use quite a lot.

  Sic: We should not (re-)define identifiers that begin with
  an underscore followed by an uppercase letter or another underscore.
*/
#if defined __SUNPRO_C || defined __SUNPRO_CC || defined _MSC_VER
# define __attribute__(A)
#endif

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#if __has_attribute(no_sanitize_undefined)
# define SUPPRESS_UBSAN __attribute__((no_sanitize_undefined))
#else
# define SUPPRESS_UBSAN
#endif

#endif /* MY_COMPILER_INCLUDED */
