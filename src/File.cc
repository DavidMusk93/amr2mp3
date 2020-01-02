#include "File.h"

#include <unistd.h>

namespace sun
{

File::File(const std::string& path): path_(path), remove_(false) {}

File::File(File&& f)
{
  path_.swap(f.path_);
  remove_ = f.remove_;
  f.remove_ = false;
}

File& File::operator=(File&& f)
{
  path_.swap(f.path_);
  remove_ = f.remove_;
  f.remove_ = false;
  return *this;
}

File::~File() { if (remove_) remove(path_.c_str()); }

File::operator bool() const { return access(path_.c_str(), F_OK) != -1; }

void File::Remove() { remove_ = true; }

}
