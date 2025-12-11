#ifndef HASHUTILS_H
#define HASHUTILS_H

#include <string>

namespace utils {

class HashUtils {
public:
  static std::string sha256(const std::string& data);
};

}

#endif //HASHUTILS_H
