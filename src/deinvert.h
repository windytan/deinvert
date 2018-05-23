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
#include <array>
#include <string>
#include <vector>

#include "config.h"
#include "src/liquid_wrappers.h"
#include "src/wdsp.h"

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

namespace deinvert {

const int   kIOBufferSize         = 4096;

enum class InputType {
  stdin, sndfile
};

enum class OutputType {
  raw_stdout, wavfile
};

struct Options {
  Options() : just_exit(false),
              is_split_band(false),
              quality(2),
              samplerate(44100),
              input_type(InputType::stdin),
              output_type(OutputType::raw_stdout) {}
  bool just_exit;
  bool is_split_band;
  int quality;
  float samplerate;
  float frequency_lo;
  float frequency_hi;
  float split_frequency;
  InputType input_type;
  OutputType output_type;
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
  std::array<int16_t, kIOBufferSize> buffer_;
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
  std::array<float, kIOBufferSize> buffer_;
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
  std::array<int16_t, kIOBufferSize> buffer_;
  size_t buffer_pos_;
};

#ifdef HAVE_SNDFILE
class SndfileWriter : public AudioWriter {
 public:
  SndfileWriter(const std::string& fname, int rate);
  ~SndfileWriter() override;
  bool push(float sample) override;

 private:
  bool write();
  SF_INFO info_;
  SNDFILE* file_;
  std::array<float, kIOBufferSize> buffer_;
  size_t buffer_pos_;
};
#endif

class Inverter {
 public:
  Inverter(float freq_prefilter, float freq_shift, float freq_postfilter,
           float samplerate, int filter_quality);
  float execute(float insample);

 private:
  const std::vector<float> filter_lengths_;
  const std::vector<float> filter_attenuation_;
#ifdef HAVE_LIQUID
  liquid::FIRFilter prefilter_;
  liquid::FIRFilter postfilter_;
  liquid::NCO oscillator_;
#else
  wdsp::NCO oscillator_;
#endif
  const bool do_filter_;
};

}  // namespace deinvert
#endif  // DEINVERT_H_
