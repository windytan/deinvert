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
#include "src/deinvert.h"

#include <getopt.h>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

#include "config.h"
#include "src/liquid_wrappers.h"
#include "src/wdsp.h"

namespace deinvert {

namespace {

const float kLowpassFilterLengthSeconds = 0.0018f;
const int   kMaxFilterLength            = 2047;

}

void PrintUsage() {
  std::cout <<
     "deinvert [OPTIONS]\n"
     "\n"
     "-f, --frequency FREQ   Frequency of the inversion carrier, in Hertz.\n"
     "\n"
     "-h, --help             Display this usage help.\n"
     "\n"
     "-i, --input-file FILE  Use an audio file as input. All formats\n"
     "                       supported by libsndfile should work.\n"
     "\n"
     "-n, --nofilter         Disable filtering (faster).\n"
     "\n"
     "-o, --output-file FILE Write output to a WAV file instead of stdout. An\n"
     "                       existing file will be overwritten.\n"
     "\n"
     "-p, --preset NUM       Scrambler frequency preset (1-8), referring to\n"
     "                       the set of common carrier frequencies used by\n"
     "                       e.g. the Selectone ST-20B scrambler.\n"
     "\n"
     "-r, --samplerate RATE  Sampling rate of raw input audio, in Hertz.\n"
     "\n"
     "-v, --version          Display version string.\n";
}

void PrintVersion() {
#ifdef DEBUG
  std::cout << PACKAGE_STRING << "-debug by OH2EIQ" << std::endl;
#else
  std::cout << PACKAGE_STRING << " by OH2EIQ" << std::endl;
#endif
}

Options GetOptions(int argc, char** argv) {
  deinvert::Options options;

  static struct option long_options[] = {
    { "frequency",     no_argument, 0, 'f'},
    { "preset",        1,           0, 'p'},
    { "input-file",    1,           0, 'i'},
    { "help",          no_argument, 0, 'h'},
    { "nofilter",      no_argument, 0, 'n'},
    { "samplerate",    1,           0, 'r'},
    { "version",       no_argument, 0, 'v'},
    {0,                0,           0,  0}};

  static const std::vector<float> selectone_carriers({
      2632.f, 2718.f, 2868.f, 3023.f, 3196.f, 3339.f, 3495.f, 3729.f
  });

  options.frequency = selectone_carriers.at(0);

  int option_index = 0;
  int option_char;
  int selectone_num;

  while ((option_char = getopt_long(argc, argv, "f:hi:no:p:r:v",
                                    long_options,
         &option_index)) >= 0) {
    switch (option_char) {
      case 'i':
#ifdef HAVE_SNDFILE
        options.infilename = std::string(optarg);
        options.input_type = deinvert::INPUT_SNDFILE;
#else
        std::cerr << "error: deinvert was compiled without libsndfile"
                  << std::endl;
        options.just_exit = true;
#endif
        break;
      case 'f':
        options.frequency = std::atoi(optarg);
        break;
      case 'n':
        options.nofilter = true;
        break;
      case 'o':
#ifdef HAVE_SNDFILE
        options.output_type = deinvert::OUTPUT_WAVFILE;
        options.outfilename = std::string(optarg);
#else
        std::cerr << "error: deinvert was compiled without libsndfile"
                  << std::endl;
        options.just_exit = true;
#endif
        break;
      case 'p':
        selectone_num = std::atoi(optarg);
        if (selectone_num >= 1 &&
            selectone_num <= 8) {
          options.frequency = selectone_carriers.at(selectone_num - 1);
        } else {
          std::cerr << "error: please specify scrambler group from 1 to 8"
                    << std::endl;
          options.just_exit = true;
        }
        break;
      case 'r':
        options.samplerate = std::atoi(optarg);
        if (options.samplerate < 6000.f) {
          std::cerr << "error: sample rate must be 6000 Hz or higher"
                    << std::endl;
          options.just_exit = true;
        }
        break;
      case 'v':
        PrintVersion();
        options.just_exit = true;
        break;
      case 'h':
      default:
        PrintUsage();
        options.just_exit = true;
        break;
    }
    if (options.just_exit)
      break;
  }

  return options;
}

AudioReader::~AudioReader() {
}

bool AudioReader::eof() const {
  return is_eof_;
}

StdinReader::StdinReader(const Options& options) :
    samplerate_(options.samplerate) {
  is_eof_ = false;
}

StdinReader::~StdinReader() {
}

std::vector<float> StdinReader::ReadBlock() {
  int num_read = fread(buffer_, sizeof(buffer_[0]), bufsize_, stdin);

  if (num_read < bufsize_)
    is_eof_ = true;

  std::vector<float> result(num_read);
  for (int i = 0; i < num_read; i++)
    result[i] = buffer_[i] * (1.f / 32768.f);

  return result;
}

float StdinReader::samplerate() const {
  return samplerate_;
}

#ifdef HAVE_SNDFILE
SndfileReader::SndfileReader(const Options& options) :
    info_({0, 0, 0, 0, 0, 0}),
    file_(sf_open(options.infilename.c_str(), SFM_READ, &info_)) {
  is_eof_ = false;
  if (file_ == nullptr) {
    std::cerr << options.infilename << ": " << sf_strerror(nullptr) <<
                 std::endl;
    is_eof_ = true;
  } else if (info_.samplerate < 6000.f) {
    std::cerr << "error: sample rate must be 6000 Hz or higher" << std::endl;
    is_eof_ = true;
  }
}

SndfileReader::~SndfileReader() {
  sf_close(file_);
}

std::vector<float> SndfileReader::ReadBlock() {
  std::vector<float> result;
  if (is_eof_)
    return result;

  sf_count_t num_read = sf_readf_float(file_, buffer_,
      bufsize_ / info_.channels);
  if (num_read != bufsize_)
    is_eof_ = true;

  if (info_.channels == 1) {
    result = std::vector<float>(buffer_, buffer_ + num_read);
  } else {
    result = std::vector<float>(num_read);
    for (size_t i = 0; i < result.size(); i++)
      result[i] = buffer_[i * info_.channels];
  }
  return result;
}

float SndfileReader::samplerate() const {
  return info_.samplerate;
}
#endif

AudioWriter::~AudioWriter() {
}

RawPCMWriter::RawPCMWriter() : ptr_(0) {
}

bool RawPCMWriter::push(float sample) {
  int16_t outsample = sample * 32767.f;
  buffer_[ptr_] = outsample;
  ptr_++;
  if (ptr_ == bufsize_) {
    fwrite(buffer_, sizeof(buffer_[0]), bufsize_, stdout);
    ptr_ = 0;
  }
  return true;
}

#ifdef HAVE_SNDFILE
SndfileWriter::SndfileWriter(const std::string& fname, int rate) :
    info_({0, rate, 1, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 0, 0}),
    file_(sf_open(fname.c_str(), SFM_WRITE, &info_)),
    ptr_(0) {
}

SndfileWriter::~SndfileWriter() {
  write(ptr_);
  sf_close(file_);
}

bool SndfileWriter::push(float sample) {
  bool success = true;
  buffer_[ptr_] = sample;
  if (ptr_ == bufsize_ - 1) {
    success = write(ptr_);
  }

  ptr_ = (ptr_ + 1) % bufsize_;
  return success;
}

bool SndfileWriter::write(sf_count_t numsamples) {
  return (file_ != nullptr &&
          sf_write_float(file_, buffer_, numsamples) == numsamples);
}
#endif

}  // namespace deinvert

int main(int argc, char** argv) {
  deinvert::Options options = deinvert::GetOptions(argc, argv);

  if (options.just_exit)
    return EXIT_FAILURE;

  deinvert::AudioReader* reader;
  deinvert::AudioWriter* writer;

#ifdef HAVE_SNDFILE
  if (options.input_type == deinvert::INPUT_SNDFILE)
    reader = new deinvert::SndfileReader(options);
  else
#endif
    reader = new deinvert::StdinReader(options);

#ifdef HAVE_SNDFILE
  if (options.output_type == deinvert::OUTPUT_WAVFILE)
    writer = new deinvert::SndfileWriter(options.outfilename,
                                         options.samplerate);
  else
#endif
    writer = new deinvert::RawPCMWriter();

  int filter_length = 2 * std::round(options.samplerate *
                                     deinvert::kLowpassFilterLengthSeconds) + 1;
  filter_length = std::min(deinvert::kMaxFilterLength, filter_length);

#ifdef HAVE_LIQUID
  liquid::NCO nco(LIQUID_VCO,
                  options.frequency * 2.0f * M_PI / options.samplerate);
  liquid::FIRFilter prefilter(filter_length,
                           options.frequency / options.samplerate);
  liquid::FIRFilter postfilter(filter_length,
                           (options.frequency - 150.f) / options.samplerate);
#else
  wdsp::NCO nco(options.frequency * 2.0f * M_PI / options.samplerate);
#endif

  while (!reader->eof()) {
    for (float insample : reader->ReadBlock()) {
      nco.Step();
#ifdef HAVE_LIQUID
      if (options.nofilter) {
        writer->push(nco.MixUp({insample, 0.0f}).real() * M_SQRT2);
      } else {
        prefilter.push({insample, 0.0f});
        std::complex<float> mixed = nco.MixUp(prefilter.execute());
        postfilter.push(mixed.real());
        writer->push(postfilter.execute().real() * 2.f);
      }
#else
      writer->push(nco.MixUp({insample, 0.0f}).real());
#endif
    }
  }

  delete reader;
  delete writer;
}
