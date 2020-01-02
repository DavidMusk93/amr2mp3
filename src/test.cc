#include <string>
#include <stdio.h>
#include "Mp3Converter.h"

struct LineReader
{
  LineReader(const char *cmdline) { fp_ = popen(cmdline, "r"); }

  ~LineReader() { if (fp_) fclose(fp_); }

  const char *GetLine()
  {
    if (!feof(fp_) && fscanf(fp_, "%[^\n]", buffer_)) {
      fscanf(fp_, "%*[\n]");
      return const_cast<const char *>(buffer_);
    }
    return nullptr;
  }

 private:
  FILE *fp_;
  char buffer_[2048];
};

static std::string basename(const std::string& path)
{
  auto pos = path.find_last_of('/');
  if (pos != std::string::npos)
    return path.substr(pos+1);
  return "";
}

int main(int argc, char *argv[])
{
  setbuf(stdout, nullptr);
  setbuf(stderr, nullptr);
  const char *line;
  std::string input;
  LineReader reader("find /sdcard/tencent -name \"*.amr\"");
  while ((line = reader.GetLine())) {
    printf("@LineReader %s\n", line);
    input = line;
    auto output = basename(input)+".mp3";
    if (Mp3Converter::GetInstance()->Decode(input, output)) {
      fprintf(stderr, "Convert failed\n");
      return 1;
    }
    puts("--- --- --- ---\n");
  }
  puts("Done!");
  return 0;

  // if (argc < 3) {
  //   fprintf(stderr, "Usage: %s input output\n", argv[0]);
  //   return 1;
  // }
  // return Mp3Converter::GetInstance()->Decode(argv[1], argv[2]);
}
