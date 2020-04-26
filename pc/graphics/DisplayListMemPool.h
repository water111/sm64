/*!
 * Memory Pool which will hold vertices, matrices, viewports, etc...
 */

#ifndef SM64_DISPLAYLISTMEMPOOL_H
#define SM64_DISPLAYLISTMEMPOOL_H

#include <cstdio>
#include <cassert>

class DisplayListMemPool {
public:
  DisplayListMemPool() = default;

  void init_mem(uint32_t size) {
    delete[] mem;
    mem = new uint8_t[size];
    mem_size = size;
    mem_used = 0;
  }

  template<typename T>
  T* alloc(int sz = 1) {
    T* obj = (T*)(mem + mem_used);
    mem_used += sizeof(T) * sz;
    if(mem_used > mem_size) {
      printf("[ERROR] DisplayListMemPool out of memory.\n");
      assert(false);
    }
    return obj;
  }

  void free_all() {
    mem_used = 0;
  }

  uint32_t size() {
    return mem_size;
  }

  uint32_t used() {
    return mem_used;
  }

private:
  uint8_t* mem = nullptr;
  uint32_t mem_size = 0;
  uint32_t mem_used = 0;
};

#endif //SM64_DISPLAYLISTMEMPOOL_H
