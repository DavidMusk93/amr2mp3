#include <atomic>
#include <stdio.h>

#include "Mp3Converter.h"
#include "File.h"

extern "C" {
#include "lame.h"

  int AmrDecoder(void **, const char *, const char *);
  int Mp3Encoder(lame_t *, const char *, const char *);
}

Mp3Converter::Mp3Converter()
  : amr_decoder_(nullptr), mp3_encoder_(nullptr)
{
}

std::shared_ptr<Mp3Converter> Mp3Converter::Create()
{
  class T: public Mp3Converter {};
  return std::make_shared<T>();
}

Mp3Converter::~Mp3Converter()
{
  if (amr_decoder_)
    free(amr_decoder_);
  if (mp3_encoder_)
    lame_close((lame_t)mp3_encoder_);
}

std::shared_ptr<Mp3Converter> Mp3Converter::instance;

std::shared_ptr<Mp3Converter> Mp3Converter::GetInstance()
{
  std::shared_ptr<Mp3Converter> empty;
  (void)std::atomic_compare_exchange_weak_explicit(
      &instance,
      &empty,
      Create(),
      std::memory_order_release,
      std::memory_order_relaxed);
  return instance;
}

static bool ends_with(const std::string& s, const std::string& suffix)
{
  auto pos = s.find_last_of(suffix);
  return pos != std::string::npos && (pos+1) == s.size();
}

static const char AMR_[] = {'.', 'a', 'm', 'r', '\0'};
static const char PCM_[] = {'.', 'p', 'c', 'm', '\0'};
static const char MP3_[] = {'.', 'm', 'p', '3', '\0'};

static const std::string AMR = std::string(const_cast<char *>(AMR_));
static const std::string PCM = std::string(const_cast<char *>(PCM_));
static const std::string MP3 = std::string(const_cast<char *>(MP3_));

// @amr, amr file (input) path
// @mp3, mp3 file (output) path
// @return, 0 on success, 1 on failure
int Mp3Converter::Decode(const std::string& amr, const std::string& mp3)
{
  std::string pcm;
  sun::File f(amr);
  if (!ends_with(amr, AMR) || !ends_with(mp3, MP3)) {
    fprintf(stderr, "input/output format error\n");
    return 1;
  }
  if (!bool(f)) {
    fprintf(stderr, "%s not exist\n", amr.c_str());
    return 1;
  }
  pcm = mp3;
  pcm.replace(mp3.size()-4, 4, PCM);
  sun::File to_be_deleted(pcm);
  to_be_deleted.Remove();
  return AmrDecoder(&amr_decoder_, amr.c_str(), pcm.c_str())
    || Mp3Encoder((lame_t *)&mp3_encoder_, pcm.c_str(), mp3.c_str());
}
