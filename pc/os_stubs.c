#include "ultra64.h"
#include "os_stubs.h"

void osInvalDCache(void *a, size_t b) {
  (void)a;
  (void)b;
}

void osInvalICache(void *a, size_t b) {
  (void)a;
  (void)b;
}

void osWritebackDCache(void *a, size_t b) {
  (void)a;
  (void)b;
}

void osWritebackDCacheAll(void) {

}

void decompress(void *a, void *b) {
  (void)a;
  (void)b;
}

u32 osGetCount(void) {
  return 0;
}

s32 __osDisableInt() {
  return 0;
}

void __osRestoreInt(s32 x) {
  (void)x;
}

u32 osTvType = TV_TYPE_NTSC;