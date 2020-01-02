#include <stdarg.h>
#include <atomic>
#include <stdio.h>
#include "Logger.h"

namespace sun
{

class Vararg
{
 public:
  ~Vararg() { va_end(list_); }

  va_list& Get() { return list_; }

 private:
  va_list list_;
};

Logger::Logger()
{
  buffer_[kBufferSize] = 0;
}

std::shared_ptr<Logger> Logger::Create()
{
  class T: public Logger {};
  return std::make_shared<T>();
}

void Logger::FormatString(const char *fmt, va_list& list)
{
  (void)vsnprintf(buffer_, kBufferSize, fmt, list);
}

static std::shared_ptr<Logger> instance;

std::shared_ptr<Logger> Logger::GetInstance()
{
  // static std::shared_ptr<Logger> instance = Create();
  std::shared_ptr<Logger> empty;
  (void)std::atomic_compare_exchange_weak_explicit(
      &instance,
      &empty,
      Create(),
      std::memory_order_release,
      std::memory_order_relaxed);
  return instance;
}

void Logger::i(const char *fmt, ...)
{
  Vararg va;
  va_start(va.Get(), fmt);
  FormatString(fmt, va.Get());
  fputs(buffer_, stdout);
}

void Logger::e(const char *fmt, ...)
{
  Vararg va;
  va_start(va.Get(), fmt);
  FormatString(fmt, va.Get());
  fputs(buffer_, stderr);
}

}
