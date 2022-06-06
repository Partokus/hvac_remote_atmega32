#include <stdlib.h>

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

void __cxa_pure_virtual(void)
{
  abort();
}

void __cxa_deleted_virtual(void)
{
  abort();
}
