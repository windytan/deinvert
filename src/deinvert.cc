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
#include <iostream>
#include <string>

#include "config.h"
#include "src/liquid_wrappers.h"
#include "src/wdsp.h"

namespace deinvert {

namespace {

const int   kMaxFilterLength            = 2047;

int FilterLengthInSamples(float len_seconds, float samplerate) {
  int filter_length = 2 * std::round(samplerate *
                                     len_seconds) + 1;
  filter_length = filter_length < deinvert::kMaxFilterLength ?
                  filter_length : deinvert::kMaxFilterLength;

  return filter_length;
}

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
     "-o, --output-file FILE Write output to a WAV file instead of stdout. An\n"
     "                       existing file will be overwritten.\n"
     "\n"
     "-p, --preset NUM       Scrambler frequency preset (1-8), referring to\n"
     "                       the set of common carrier frequencies used by\n"
     "                       e.g. the Selectone ST-20B scrambler.\n"
     "\n"
     "-q, --quality NUM      Filter quality, from 0 (worst and fastest) to\n"
     "                       3 (best and slowest). The default is 2.\n"
     "\n"
     "-r, --samplerate RATE  Sampling rate of raw input audio, in Hertz.\n"
     "\n"
     "-s, --split-frequency  Split point for split-band inversion, in Hertz.\n"
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
    { "frequency",       no_argument, 0, 'f'},
    { "preset",          1,           0, 'p'},
    { "input-file",      1,           0, 'i'},
    { "help",            no_argument, 0, 'h'},
    { "nofilter",        no_argument, 0, 'n'},
    { "output-file",     1,           0, 'o'},
    { "quality",         1,           0, 'q'},
    { "samplerate",      1,           0, 'r'},
    { "split-frequency", 1,           0, 's'},
    { "version",         no_argument, 0, 'v'},
    {0,                  0,           0,  0}};

  static const std::vector<float> selectone_carriers({
      2632.f, 2718.f, 2868.f, 3023.f, 3196.f, 3339.f, 3495.f, 3729.f
  });

  options.frequency_hi = selectone_carriers.at(0);

  int option_index = 0;
  int option_char;
  int selectone_num;

  while ((option_char = getopt_long(argc, argv, "f:hi:no:p:q:r:s:v",
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
        options.frequency_hi = std::atoi(optarg);
        break;
      case 'n':
        options.quality = 0;
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
          options.frequency_lo = selectone_carriers.at(selectone_num - 1);
        } else {
          std::cerr << "error: please specify scrambler group from 1 to 8"
                    << std::endl;
          options.just_exit = true;
        }
        break;
      case 'q':
        options.quality = std::atoi(optarg);
        if (options.quality < 0 || options.quality > 3) {
          std::cerr << "error: please specify filter quality from 0 to 3"
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
      case 's':
        options.frequency_lo = std::atoi(optarg);
        options.is_split_band = true;
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
  int num_read = fread(buffer_, sizeof(buffer_[0]), kIOBufferSize, stdin);

  if (num_read < kIOBufferSize)
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

  int to_read = kIOBufferSize / info_.channels;

  sf_count_t num_read = sf_readf_float(file_, buffer_, to_read);
  if (num_read != to_read)
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

RawPCMWriter::RawPCMWriter() : buffer_pos_(0) {
}

bool RawPCMWriter::push(float sample) {
  int16_t outsample = sample * 32767.f;
  buffer_[buffer_pos_] = outsample;
  buffer_pos_++;
  if (buffer_pos_ == kIOBufferSize) {
    fwrite(buffer_, sizeof(buffer_[0]), kIOBufferSize, stdout);
    buffer_pos_ = 0;
  }
  return true;
}

#ifdef HAVE_SNDFILE
SndfileWriter::SndfileWriter(const std::string& fname, int rate) :
    info_({0, rate, 1, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 0, 0}),
    file_(sf_open(fname.c_str(), SFM_WRITE, &info_)),
    buffer_pos_(0) {
  if (file_ == nullptr) {
    std::cerr << fname << ": " << sf_strerror(nullptr) <<
                 std::endl;
  }
}

SndfileWriter::~SndfileWriter() {
  write();
  sf_close(file_);
}

bool SndfileWriter::push(float sample) {
  bool success = true;
  buffer_[buffer_pos_] = sample;
  if (buffer_pos_ == kIOBufferSize - 1) {
    success = write();
  }

  buffer_pos_ = (buffer_pos_ + 1) % kIOBufferSize;
  return success;
}

bool SndfileWriter::write() {
  sf_count_t num_to_write = buffer_pos_ + 1;
  return (file_ != nullptr &&
          sf_write_float(file_, buffer_, num_to_write) == num_to_write);
}
#endif

Inverter::Inverter(float freq_prefilter, float freq_shift,
                   float freq_postfilter, float samplerate, int quality) :
#ifdef HAVE_LIQUID
    filter_lengths_({0.f, 0.0012f, 0.0024f, 0.0042f}),
    filter_attenuation_({60.f, 60.f, 60.f, 80.f}),
    prefilter_(FilterLengthInSamples(filter_lengths_.at(quality), samplerate),
               freq_prefilter / samplerate,
               filter_attenuation_.at(quality)),
    postfilter_(FilterLengthInSamples(filter_lengths_.at(quality), samplerate),
                freq_postfilter / samplerate,
                filter_attenuation_.at(quality)),
    oscillator_(LIQUID_VCO, freq_shift * 2.0f * M_PI / samplerate),
    do_filter_(quality > 0)
#else
    oscillator_(freq_shift * 2.0f * M_PI / samplerate);
    do_filter_(false)
#endif
{}

float Inverter::execute(float insample) {
  oscillator_.Step();

  float result;

#ifdef HAVE_LIQUID
  if (do_filter_) {
    prefilter_.push({insample, 0.0f});
    postfilter_.push(oscillator_.MixUp(prefilter_.execute()).real());
    result = postfilter_.execute().real();
  } else {
    result = oscillator_.MixUp({insample, 0.0f}).real();
  }
#else
  result = oscillator_.MixUp({insample, 0.0f}).real();
#endif

  return result;
}

}  // namespace deinvert

int main(int argc, char** argv) {
  deinvert::Options options = deinvert::GetOptions(argc, argv);

  if (options.just_exit)
    return EXIT_FAILURE;

  deinvert::AudioReader* reader;
  deinvert::AudioWriter* writer;

  if (options.input_type == deinvert::INPUT_SNDFILE) {
#ifdef HAVE_SNDFILE
    reader = new deinvert::SndfileReader(options);
    options.samplerate = reader->samplerate();
#endif
  } else {
    reader = new deinvert::StdinReader(options);
  }

#ifdef HAVE_SNDFILE
  if (options.output_type == deinvert::OUTPUT_WAVFILE)
    writer = new deinvert::SndfileWriter(options.outfilename,
                                         options.samplerate);
  else
#endif
    writer = new deinvert::RawPCMWriter();


  if (options.is_split_band) {
    static const std::vector<float> filter_gain_compensation({
        0.5f, 1.4f, 1.8f, 1.8f
    });
    float gain = filter_gain_compensation.at(options.quality);

    deinvert::Inverter inverter1(options.frequency_lo, options.frequency_lo,
                                 options.frequency_lo, options.samplerate,
                                 options.quality);
    deinvert::Inverter inverter2(options.frequency_hi,
                                 options.frequency_lo + options.frequency_hi,
                                 options.frequency_hi, options.samplerate,
                                 options.quality);

    while (!reader->eof()) {
      for (float insample : reader->ReadBlock()) {
        bool can_still_write = writer->push(gain *
                                            (inverter1.execute(insample) +
                                             inverter2.execute(insample)));
        if (!can_still_write)
          continue;
      }
    }
  } else {
    static const std::vector<float> filter_gain_compensation({
        1.0f, 1.4f, 2.f, 2.f
    });
    float gain = filter_gain_compensation.at(options.quality);

    deinvert::Inverter inverter(options.frequency_hi - 150.f,
                                options.frequency_hi,
                                options.frequency_hi, options.samplerate,
                                options.quality);

    while (!reader->eof()) {
      for (float insample : reader->ReadBlock()) {
        bool can_still_write = writer->push(gain * inverter.execute(insample));
        if (!can_still_write)
          continue;
      }
    }
  }

  delete reader;
  delete writer;
}
