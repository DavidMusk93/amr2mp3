#pragma once

#include <memory>
#include "noncopyable.h"

namespace sun
{

class Logger: public noncopyable
{
 private:
  Logger();
  void FormatString(const char *fmt, va_list& list);
  static std::shared_ptr<Logger> Create();

 public:
  static std::shared_ptr<Logger> GetInstance();
  void i(const char *fmt, ...);
  void e(const char *fmt, ...);

 private:
  char buffer_[2048];
  static constexpr int kBufferSize = sizeof(buffer_) - 1;
};

}

#define log_info(fmt, ...)   sun::Logger::GetInstance()->i(fmt "\n", ##__VA_ARGS__)
#define log_debug(fmt, ...)  log_info("<%s:%d> " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define log_error(fmt, ...)  sun::Logger::GetInstance()->e(fmt "\n", ##__VA_ARGS__)
#define log_perror(fmt, ...) sun::Logger::GetInstance()->e(fmt ": %s\n", ##__VA_ARGS__)
