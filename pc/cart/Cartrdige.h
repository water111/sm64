#ifndef SM64_CARTRDIGE_H
#define SM64_CARTRDIGE_H

#include <vector>
#include <string>

class Cartrdige {
public:

  struct Entry {
    void* dest;
    void* backup;
    int size;
  };

  Cartrdige() = default;

  void add(void* data, int size);
  void reset_all();

private:
  std::vector<Entry> entries;
};


#endif //SM64_CARTRDIGE_H
