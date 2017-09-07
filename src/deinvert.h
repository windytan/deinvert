/*
 * deinvert - a voice inversion descrambler
 * Copyright (c) Oona Räisänen
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
#ifndef DEINVERT_H_
#define DEINVERT_H_

#include <cstdint>
#include <string>
#include <vector>

#include "config.h"

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

namespace deinvert {

const float kDefaultSampleRate_Hz = 44100.0f;
const int   kIOBufferSize         = 4096;

enum eInputType {
  INPUT_STDIN, INPUT_SNDFILE
};

enum eOutputType {
  OUTPUT_RAW_STDOUT, OUTPUT_WAVFILE
};

struct Options {
  Options() : just_exit(false),
              nofilter(false),
              samplerate(kDefaultSampleRate_Hz),
              frequency(1000),
              input_type(INPUT_STDIN),
              output_type(OUTPUT_RAW_STDOUT) {}
  bool just_exit;
  bool nofilter;
  float samplerate;
  float frequency;
  eInputType input_type;
  eOutputType output_type;
  std::string infilename;
  std::string outfilename;
};

class AudioReader {
 public:
  virtual ~AudioReader();
  bool eof() const;
  virtual std::vector<float> ReadBlock() = 0;
  virtual float samplerate() const = 0;

 protected:
  bool is_eof_;
};

class StdinReader : public AudioReader {
 public:
  explicit StdinReader(const Options& options);
  ~StdinReader() override;
  std::vector<float> ReadBlock() override;
  float samplerate() const override;

 private:
  float samplerate_;
  int16_t buffer_[kIOBufferSize];
};

#ifdef HAVE_SNDFILE
class SndfileReader : public AudioReader {
 public:
  explicit SndfileReader(const Options& options);
  ~SndfileReader();
  std::vector<float> ReadBlock() override;
  float samplerate() const override;

 private:
  SF_INFO info_;
  SNDFILE* file_;
  float buffer_[kIOBufferSize];
};
#endif

class AudioWriter {
 public:
  virtual ~AudioWriter();
  virtual bool push(float sample) = 0;
};

class RawPCMWriter : public AudioWriter {
 public:
  RawPCMWriter();
  bool push(float sample) override;

 private:
  int16_t buffer_[kIOBufferSize];
  size_t buffer_pos_;
};

#ifdef HAVE_SNDFILE
class SndfileWriter : public AudioWriter {
 public:
  SndfileWriter(const std::string& fname, int rate);
  ~SndfileWriter() override;
  bool push(float sample) override;

 private:
  bool write(sf_count_t numsamples);
  SF_INFO info_;
  SNDFILE* file_;
  float buffer_[kIOBufferSize];
  size_t buffer_pos_;
};
#endif

}  // namespace deinvert
#endif  // DEINVERT_H_
