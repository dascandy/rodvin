#ifndef MATH_H
#define MATH_H

inline float ceil(float arg) {
  if (arg > -8388608 && arg < 0 && int(arg) != arg)
    return int(arg) - 1;
  else if (arg > 0 && arg < 8388608 && int(arg) != arg)
    return int(arg) + 1;
  else 
    return arg;
}

template <typename T>
T round(T arg) {
  return ceil(arg + (arg < 0 ? 0.5 : - 0.5));
}

#endif


