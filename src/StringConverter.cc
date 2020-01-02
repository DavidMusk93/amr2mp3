#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <vector>
#include <memory>

#include "SafeString.h"
#include "Logger.h"
#include "Reader.h"

static void dumpString(const std::string& s, FILE *fp, const char *prefix)
{
  int i, n;
  n = s.size();
  fprintf(fp, "%s = {", prefix);
  for (i = 0; i < n; ++i) {
    if (!i)
      fprintf(fp, "0x%x", static_cast<uint8_t>(s[i]));
    else
      fprintf(fp, ", 0x%x", static_cast<uint8_t>(s[i]));
  }
  fputs("};\n", fp);
}

// int main(int argc, char *argv[])
// {
//   if (argc < 2) {
//     log_error("Usage: %s string", argv[0]);
//     return 1;
//   }
//   std::string in(argv[1]);
//   std::string out = sun::SafeString::Encode(in);
//   dumpString(out);
//   std::string decode = sun::SafeString::Decode(out);
//   puts(decode.c_str());
//   return 0;
// }

static void closeFp(FILE *fp) { fclose(fp); }

int main()
{
  int i = 0;
  std::string pkg;
  const char *header = "data.h";
  const char *default_pkg = "com/abc/xyz";
  std::unique_ptr<FILE, void (*)(FILE *)> prop(fopen("config.prop", "r"), closeFp);
  if (prop) {
    size_t n;
    const char *s, *line, *key = "package";
    sun::LineReader reader(prop.get());
    while ((line = reader.Next())) {
      while (isspace(*line) && ++line);
      if ((s = strstr(line, key)) && s == line) {
        s = strchr(line, '=');
        if (!s)
          break;
        for (++s; isspace(*s); ++s);
        for (n = strlen(s); isspace(s[n-1]); --n);
        pkg = std::string(s, n);
        break;
      }
    }
    if (pkg.empty())
      pkg = default_pkg;
  }
  log_info("pkg:%s", pkg.c_str());
  std::unique_ptr<FILE, void (*)(FILE *)> fp(fopen(header, "w+"), closeFp);
  if (!fp) {
    log_perror("Open %s failed", header);
    return 1;
  }
  std::vector<const char *> prefix_set = {"const unsigned char m1[]", "const unsigned char sig1[]", "const unsigned char pkg[]"};
  std::vector<const char *> in_set = {"decode", "(Ljava/lang/String;Ljava/lang/String;)I", pkg.c_str()};
  std::vector<std::string> out_set;
  for (const auto& in: in_set)
    out_set.emplace_back(sun::SafeString::Encode(in));
  for (const auto& out: out_set) {
    dumpString(out, fp.get(), prefix_set[i]);
    puts(sun::SafeString::Decode(out)==in_set[i++]?"pass":"failure");
  }
  return 0;
}
