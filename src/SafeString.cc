#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "SafeString.h"
#include "Logger.h"

namespace sun
{

class Random
{
 private:
  void Open()
  {
    fd_ = open("/dev/urandom", O_RDONLY);
    if (fd_ < 0)
      log_perror("Open entropy source failed");
  }

 public:
  Random(bool do_open=false): fd_(-1) { if (do_open) Open(); }

  ~Random() { if (fd_ > 0) close(fd_); }

  char GetChar()
  {
    if (fd_ < 0)
      Open();
    read(fd_, buffer_, sizeof(char));
    // read(fd_, buffer_+1, 4);
    return *buffer_;
  }

 private:
  int fd_;
  char buffer_[1024];
};

class Seq
{
 public:
  Seq(uint8_t *s): s_(s) {}

  friend Seq& operator<<(Seq& seq, const uint8_t a);
  friend Seq& operator>>(Seq& seq, uint8_t *a);

 private:
  uint8_t *s_;
};

Seq& operator<<(Seq& seq, const uint8_t a)
{
  *seq.s_++ = a;
  return seq;
}

Seq& operator>>(Seq& seq, uint8_t *a)
{
  *a = *seq.s_++;
  return seq;
}

static void combine(uint8_t *s, const uint8_t a, const uint8_t b, const uint8_t c, int n)
{
  Seq seq(s);
  switch (n) {
    case 0:
      seq << a << b << c;
      break;
    case 1:
      seq << a << c << b;
      break;
    case 2:
      seq << b << a << c;
      break;
    case 3:
      seq << b << c << a;
      break;
    case 4:
      seq << c << a << b;
      break;
    case 5:
      seq << c << b << a;
      break;
  }
}

static void recover(const uint8_t *s, uint8_t *a, uint8_t *b, uint8_t *c, int n)
{
  Seq seq(const_cast<uint8_t *>(s));
  switch (n) {
    case 0:
      seq >> a >> b >> c;
      break;
    case 1:
      seq >> a >> c >> b;
      break;
    case 2:
      seq >> b >> a >> c;
      break;
    case 3:
      seq >> b >> c >> a;
      break;
    case 4:
      seq >> c >> a >> b;
      break;
    case 5:
      seq >> c >> b >> a;
      break;
  }
}

std::string SafeString::Encode(const std::string& in)
{
  std::string out;
  Random random;
  uint8_t r, *s, a;
  int i, n;
  n = in.size();
  out.resize(n*3);
  s = reinterpret_cast<uint8_t *>(&out.front());
  for (i = 0; i < n; ++i) {
    a = in[i], r = random.GetChar();
    combine(s, r, a|r, ~(a&r), i%6);
    s += 3;
  }
  return out;
}

std::string SafeString::Decode(const std::string& in)
{
  std::string out;
  uint8_t a, b, c, *s;
  int i, n;
  n = in.size();
  if (n%3)
    return out;
  out.resize((n/=3));
  s = reinterpret_cast<uint8_t *>(&out.front());
  for (i = 0; i < n; ++i) {
    recover(reinterpret_cast<const uint8_t *>(&in[i*3]), &a, &b, &c, i%6);
    *s++ = a^(b&c);
  }
  return out;
}

}
