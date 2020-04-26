#ifndef SM64_M64PARSER_H
#define SM64_M64PARSER_H

#include <string>
#include "ControllerData.h"

class M64Reader {
public:
  M64Reader(const std::string& _filename);
  bool parse();
  ControllerState& next();
  int off() {
    return offset;
  }
private:
  std::string filename;
  ControllerState* data = nullptr;
  int offset = 0;
};


#endif //SM64_M64PARSER_H
