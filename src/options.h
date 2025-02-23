#pragma once

#include <getopt.h>

#include <array>
#include <iostream>
#include <string>

#include "config.h"

namespace deinvert {

enum class InputType { stdin, sndfile };
enum class OutputType { raw_stdout, wavfile };

struct Options {
  bool        just_exit{};
  bool        is_split_band{};
  int         quality{2};
  float       samplerate{44100};
  float       frequency_lo{};
  float       frequency_hi{};
  float       split_frequency{};
  InputType   input_type{InputType::stdin};
  OutputType  output_type{OutputType::raw_stdout};
  std::string infilename;
  std::string outfilename;
};

inline void PrintUsage() {
  std::cout << "deinvert [OPTIONS]\n"
               "\n"
               "-f, --frequency FREQ   Frequency of the inversion carrier, in "
               "Hertz.\n"
               "\n"
               "-h, --help             Display this usage help.\n"
               "\n"
               "-i, --input-file FILE  Use an audio file as input. All formats\n"
               "                       supported by libsndfile should work.\n"
               "\n"
               "-o, --output-file FILE Write output to a WAV file instead of stdout. "
               "An\n"
               "                       existing file will be overwritten.\n"
               "\n"
               "-p, --preset NUM       Scrambler frequency preset (1-8), referring "
               "to\n"
               "                       the set of common carrier frequencies used "
               "by\n"
               "                       e.g. the Selectone ST-20B scrambler.\n"
               "\n"
               "-q, --quality NUM      Filter quality, from 0 (worst and fastest) "
               "to\n"
               "                       3 (best and slowest). The default is 2.\n"
               "\n"
               "-r, --samplerate RATE  Sampling rate of raw input audio, in Hertz.\n"
               "\n"
               "-s, --split-frequency  Split point for split-band inversion, in "
               "Hertz.\n"
               "\n"
               "-v, --version          Display version string.\n";
}

inline void PrintVersion() {
  std::cout << "deinvert " << VERSION << " by OH2EIQ" << std::endl;
}

inline Options GetOptions(int argc, char **argv) {
  Options options;

  // clang-format off
  const std::array<struct option, 11> long_options{{
      {"frequency",       no_argument,       nullptr, 'f'},
      {"preset",          required_argument, nullptr, 'p'},
      {"input-file",      required_argument, nullptr, 'i'},
      {"help",            no_argument,       nullptr, 'h'},
      {"nofilter",        no_argument,       nullptr, 'n'},
      {"output-file",     required_argument, nullptr, 'o'},
      {"quality",         required_argument, nullptr, 'q'},
      {"samplerate",      required_argument, nullptr, 'r'},
      {"split-frequency", required_argument, nullptr, 's'},
      {"version",         no_argument,       nullptr, 'v'},
      {0,                 0,                 nullptr, 0  }
  }};
  // clang-format on

  constexpr std::array<float, 8> selectone_carriers(
      {2632.f, 2718.f, 2868.f, 3023.f, 3196.f, 3339.f, 3495.f, 3729.f});

  options.frequency_hi = selectone_carriers.at(0);

  options.quality = 2;

  int  option_index{};
  int  option_char{};
  int  selectone_num{};
  bool samplerate_set        = false;
  bool carrier_frequency_set = false;
  bool carrier_preset_set    = false;

  while ((option_char = getopt_long(argc, argv, "f:hi:no:p:q:r:s:v", long_options.data(),
                                    &option_index)) >= 0) {
    switch (option_char) {
      case 'i':
        options.infilename = std::string(optarg);
        options.input_type = deinvert::InputType::sndfile;
        break;
      case 'f':
        options.frequency_hi  = std::atoi(optarg);
        carrier_frequency_set = true;
        break;
      case 'n': options.quality = 0; break;
      case 'o':
        options.output_type = deinvert::OutputType::wavfile;
        options.outfilename = std::string(optarg);
        break;
      case 'p':
        selectone_num      = std::atoi(optarg);
        carrier_preset_set = true;
        if (selectone_num >= 1 && selectone_num <= 8)
          options.frequency_hi = selectone_carriers.at(selectone_num - 1);
        else
          throw std::runtime_error("preset should be a number from 1 to 8");
        break;
      case 'q':
        options.quality = static_cast<int>(std::strtol(optarg, nullptr, 10));
        if (options.quality < 0 || options.quality > 3)
          throw std::runtime_error("please specify filter quality from 0 to 3");
        break;
      case 'r':
        options.samplerate = static_cast<float>(std::atoi(optarg));
        samplerate_set     = true;
        break;
      case 's':
        options.frequency_lo  = std::atoi(optarg);
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

  if (!carrier_preset_set && !carrier_frequency_set)
    std::cerr << "deinvert: warning: carrier frequency not set, trying "
              << "2632 Hz\n";

  if (options.input_type == InputType::stdin && !samplerate_set)
    throw std::runtime_error("must specify sample rate for stdin; use the -r option");

  if (options.input_type == InputType::sndfile && samplerate_set)
    throw std::runtime_error(
        "don't specify sample rate (-r) with -i; I want to read it from the sound file");

  if (options.is_split_band && options.frequency_lo >= options.frequency_hi)
    throw std::runtime_error("split point must be below the inversion carrier");

  if (options.samplerate < options.frequency_hi * 2.0f)
    throw std::runtime_error(
        "sample rate must be at least twice the inversion frequency (see Nyquist-Shannon theorem)");

  return options;
}

}  // namespace deinvert