#include <stdio.h>

namespace __cxxabiv1 
{
  __extension__ typedef int __guard __attribute__((mode(__DI__)));
  extern "C" int __cxa_guard_acquire (__guard *g) 
  {
    return !*(char *)(g);
  }

  extern "C" void __cxa_guard_release (__guard *g)
  {
    *(char *)g = 1;
  }

  extern "C" void __cxa_guard_abort (__guard *)
  {
  }

  extern "C" void __cxa_end_cleanup()
  {
  }

  extern "C" int __dso_handle()
  {
    return 42;
  }
  
  // As a kernel we do not exit.
  extern "C" void __aeabi_atexit()
  {
  }
 
  extern "C" void __cxa_atexit()
  {
  }

  extern "C" void __cxa_pure_virtual()
  {
    printf("pure virtual\n");
  }
}

