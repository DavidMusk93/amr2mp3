#pragma once

#include <memory>
#include <string>

class Mp3Converter
{
 private:
  Mp3Converter();

 public:
  ~Mp3Converter();
  int Decode(const std::string& amr, const std::string& mp3);

  static std::shared_ptr<Mp3Converter> GetInstance();

 private:
  static std::shared_ptr<Mp3Converter> Create();
  static std::shared_ptr<Mp3Converter> instance;

  void *amr_decoder_;
  void *mp3_encoder_;
};
