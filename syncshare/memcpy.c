#include <string.h>
void *__memcpy_glibc_2_2_5(void *, const void *, size_t);
//__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
asm(".symver __memcpy_glibc_2_2_5, memcpy@GLIBC_2.2.5");
void *__wrap_memcpy(void *dest, const void *src, size_t n) {
    return __memcpy_glibc_2_2_5(dest, src, n); 
}