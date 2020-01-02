#pragma once

#include <string>

namespace sun
{

class SafeString
{
 public:
  // enum Type
  // {
  //   SELF,
  //   RANDOM
  // };

  SafeString() = delete;

  static std::string Encode(const std::string& in);
  static std::string Decode(const std::string& in);
};

}
