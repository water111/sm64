#include <math.h>

#ifndef __GNUC__
#pragma intrinsic(sqrtf)
#define __builtin_sqrtf sqrtf
#endif

#ifndef PC_PORT
float sqrtf(float f) {
    return __builtin_sqrtf(f);
}
#endif
