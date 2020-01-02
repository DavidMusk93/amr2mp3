#pragma once

#include <stdio.h>

#include "noncopyable.h"

namespace sun
{

struct Line
{
  char *buf;
  size_t n;
  Line();
  ~Line();
};

class LineReader
{
 public:
  LineReader(FILE *fp);
  const char *Next();

 private:
  FILE *fp_;
  Line line;
};

}
