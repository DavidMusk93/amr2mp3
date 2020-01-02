#pragma once

#include <string>

#include <stdio.h>

#include "noncopyable.h"

namespace sun
{

class File: noncopyable
{
 public:
  File(const std::string& path);
  File(File&& f);
  File& operator=(File&& f);
  ~File();
  operator bool() const;
  void Remove();

 private:
  std::string path_;
  bool remove_;
};

}
