#pragma once

#include <array>
#include <vector>

#include <sndfile.h>

#include "src/options.h"

namespace deinvert {

constexpr int kIOBufferSize = 4096;

class AudioReader {
 public:
  virtual ~AudioReader() = default;
  bool eof() const {
    return is_eof_;
  };
  virtual std::vector<float> ReadBlock()        = 0;
  virtual float              samplerate() const = 0;

 protected:
  bool is_eof_{};
};

class StdinReader : public AudioReader {
 public:
  explicit StdinReader(const Options &options) : samplerate_(options.samplerate) {}
  ~StdinReader() override = default;
  std::vector<float> ReadBlock() override {
    const int num_read = fread(buffer_.data(), sizeof(buffer_[0]), kIOBufferSize, stdin);

    if (num_read < kIOBufferSize)
      is_eof_ = true;

    std::vector<float> result(num_read);
    for (int i = 0; i < num_read; i++) result[i] = buffer_[i] * (1.f / 32768.f);

    return result;
  };
  float samplerate() const override {
    return samplerate_;
  };

 private:
  float                              samplerate_;
  std::array<int16_t, kIOBufferSize> buffer_{};
};

class SndfileReader : public AudioReader {
 public:
  explicit SndfileReader(const Options &options)
      : info_({0, 0, 0, 0, 0, 0}), file_(sf_open(options.infilename.c_str(), SFM_READ, &info_)) {
    if (file_ == nullptr) {
      throw std::runtime_error(options.infilename + ": " + sf_strerror(nullptr));
    } else if (info_.samplerate < options.frequency_hi * 2.0f) {
      throw std::runtime_error("sample rate must be at least twice the inversion frequency");
    }
  }
  ~SndfileReader() override {
    sf_close(file_);
  };
  std::vector<float> ReadBlock() override {
    std::vector<float> result;
    if (is_eof_)
      return result;

    const int to_read = kIOBufferSize / info_.channels;

    const sf_count_t num_read = sf_readf_float(file_, buffer_.data(), to_read);
    if (num_read != to_read)
      is_eof_ = true;

    if (info_.channels == 1) {
      result = std::vector<float>(buffer_.begin(), buffer_.begin() + num_read);
    } else {
      result = std::vector<float>(num_read);
      for (size_t i = 0; i < result.size(); i++) result[i] = buffer_[i * info_.channels];
    }
    return result;
  };
  float samplerate() const override {
    return info_.samplerate;
  };

 private:
  SF_INFO                          info_;
  SNDFILE                         *file_;
  std::array<float, kIOBufferSize> buffer_{};
};

class AudioWriter {
 public:
  virtual ~AudioWriter()          = default;
  virtual bool push(float sample) = 0;
};

class RawPCMWriter : public AudioWriter {
 public:
  RawPCMWriter() = default;
  bool push(float sample) override {
    const int16_t outsample = static_cast<int16_t>(sample * 32767.f);
    buffer_[buffer_pos_]    = outsample;
    buffer_pos_++;
    if (buffer_pos_ == kIOBufferSize) {
      fwrite(buffer_.data(), sizeof(buffer_[0]), kIOBufferSize, stdout);
      buffer_pos_ = 0;
    }
    return true;
  }

 private:
  std::array<int16_t, kIOBufferSize> buffer_{};
  size_t                             buffer_pos_{};
};

class SndfileWriter : public AudioWriter {
 public:
  SndfileWriter(const std::string &fname, int rate)
      : info_({0, rate, 1, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 0, 0}),
        file_(sf_open(fname.c_str(), SFM_WRITE, &info_)) {
    if (file_ == nullptr)
      throw std::runtime_error(fname + ": " + sf_strerror(nullptr));
  }

  ~SndfileWriter() override {
    write();
    sf_close(file_);
  };

  bool push(float sample) override {
    bool success         = true;
    buffer_[buffer_pos_] = sample;
    if (buffer_pos_ == kIOBufferSize - 1) {
      success = write();
    }

    buffer_pos_ = (buffer_pos_ + 1) % kIOBufferSize;
    return success;
  };

 private:
  bool write() {
    const sf_count_t num_to_write = buffer_pos_ + 1;
    return (file_ != nullptr &&
            sf_write_float(file_, buffer_.data(), num_to_write) == num_to_write);
  }
  SF_INFO                          info_;
  SNDFILE                         *file_;
  std::array<float, kIOBufferSize> buffer_{};
  size_t                           buffer_pos_{};
};

}  // namespace deinvert