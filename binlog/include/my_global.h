#ifndef MY_GLOBAL_
#define MY_GLOBAL_

#if (defined(_WIN32) && defined(MYSQL_DYNAMIC_PLUGIN))
#define MYSQL_PLUGIN_IMPORT __declspec(dllimport)
#else
#define MYSQL_PLUGIN_IMPORT
#endif

#define MY_MAX(a, b)	((a) > (b) ? (a) : (b))
#define MY_MIN(a, b)	((a) < (b) ? (a) : (b))
#define swap_variables(t, a, b) { t dummy; dummy= a; a= b; b= dummy; }
#define MY_TEST(a)		((a) ? 1 : 0)
#endif //MY_GLOBAL_