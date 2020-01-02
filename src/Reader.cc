#include "Reader.h"

#include <string.h>
#include <stdlib.h>

namespace sun
{

Line::Line() { bzero(this, sizeof(Line)); }

Line::~Line() { if (buf) free(buf); }

LineReader::LineReader(FILE *fp): fp_(fp) {}

const char *LineReader::Next()
{
  if (getline(&line.buf, &line.n, fp_) != -1)
    return const_cast<const char *>(line.buf);
  return nullptr;
}

}
