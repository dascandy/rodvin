#include <stdlib.h>

int abs(int j) {
  if (j >= 0) return j;
  return -j;
}

long int labs(long int j) {
  if (j >= 0) return j;
  return -j;
}

long long int llabs(long long int j) {
  if (j >= 0) return j;
  return -j;
}

long int strtol(const char *nptr, char **endptr, int base) {
  bool negative = false;
  long int val = 0;
  while (*nptr) {
    if (*nptr == '-') {
      nptr++;
      negative = true;
    } else if (*nptr >= '0' && *nptr <= '9')
      val = val * base + *nptr++ - '0';
    else
      break;
  }
  if (endptr) *endptr = const_cast<char*>(nptr);
  return negative ? -val : val;
}


