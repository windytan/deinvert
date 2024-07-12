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

#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "src/io.h"
#include "src/liquid_wrappers.h"
#include "src/options.h"

namespace deinvert {

namespace {

constexpr int kMaxFilterLength = 2047;

int FilterLengthInSamples(float len_seconds, float samplerate) {
  const int filter_length =
      std::min(2 * static_cast<int>(std::round(samplerate * len_seconds)) + 1, kMaxFilterLength);

  return filter_length;
}

}  // namespace

DCRemover::DCRemover(size_t length) : buffer_(length) {}

void DCRemover::push(float sample) {
  if (buffer_.size() > 0) {
    buffer_[index_] = sample;
    index_          = (index_ + 1) % buffer_.size();

    if (index_ == 0)
      is_filled_ = true;
  }
}

float DCRemover::execute(float sample) const {
  if (buffer_.size() == 0) {
    return sample;
  } else {
    float sum = std::accumulate(buffer_.begin(), buffer_.end(), 0.0f);

    if (is_filled_)
      sum /= buffer_.size();
    else
      sum /= (index_ == 0 ? 1 : index_);

    return sample - sum;
  }
}

Inverter::Inverter(float freq_prefilter, float freq_shift, float freq_postfilter, float samplerate,
                   int filter_quality)
    : filter_lengths_({0.f, 0.0006f, 0.0024f, 0.0064f}),
      filter_attenuation_({60.f, 60.f, 60.f, 80.f}),
      prefilter_(FilterLengthInSamples(filter_lengths_.at(filter_quality), samplerate),
                 freq_prefilter / samplerate, filter_attenuation_.at(filter_quality)),
      postfilter_(FilterLengthInSamples(filter_lengths_.at(filter_quality), samplerate),
                  freq_postfilter / samplerate, filter_attenuation_.at(filter_quality)),
      oscillator_(LIQUID_VCO, freq_shift * 2.0f * M_PI / samplerate),
      do_filter_(filter_quality > 0) {}

float Inverter::execute(float insample) {
  oscillator_.Step();

  float result;

  if (do_filter_) {
    prefilter_.push(insample);
    postfilter_.push(oscillator_.MixUp(prefilter_.execute()).real());
    result = postfilter_.execute();
  } else {
    result = oscillator_.MixUp({insample, 0.0f}).real();
  }

  return result;
}

}  // namespace deinvert

void SimpleDescramble(deinvert::Options options, std::unique_ptr<deinvert::AudioReader> &reader,
                      std::unique_ptr<deinvert::AudioWriter> &writer) {
  static const std::vector<float> filter_gain_compensation({1.0f, 1.4f, 1.8f, 1.8f});
  float                           gain = filter_gain_compensation.at(options.quality);

  const int dc_remover_length = (options.quality * options.samplerate * 0.002f);

  deinvert::DCRemover dcremover(dc_remover_length);

  deinvert::Inverter inverter(options.frequency_hi, options.frequency_hi, options.frequency_hi,
                              options.samplerate, options.quality);

  while (!reader->eof()) {
    for (float insample : reader->ReadBlock()) {
      dcremover.push(insample);
      const bool can_still_write =
          writer->push(gain * inverter.execute(dcremover.execute(insample)));
      if (!can_still_write)
        continue;
    }
  }
}

void SplitBandDescramble(deinvert::Options options, std::unique_ptr<deinvert::AudioReader> &reader,
                         std::unique_ptr<deinvert::AudioWriter> &writer) {
  static const std::vector<float> filter_gain_compensation({0.5f, 1.4f, 1.8f, 1.8f});
  const float                     gain = filter_gain_compensation.at(options.quality);

  const int dc_remover_length = (options.quality * options.samplerate * 0.002f);

  deinvert::DCRemover dcremover(dc_remover_length);

  deinvert::Inverter inverter1(options.frequency_lo, options.frequency_lo, options.frequency_lo,
                               options.samplerate, options.quality);
  deinvert::Inverter inverter2(options.frequency_hi, options.frequency_lo + options.frequency_hi,
                               options.frequency_hi, options.samplerate, options.quality);

  while (!reader->eof()) {
    for (const float insample : reader->ReadBlock()) {
      dcremover.push(insample);
      const float dcremoved = dcremover.execute(insample);

      const bool can_still_write =
          writer->push(gain * (inverter1.execute(dcremoved) + inverter2.execute(dcremoved)));
      if (!can_still_write)
        continue;
    }
  }
}

int main(int argc, char **argv) {
  deinvert::Options options;

  try {
    options = deinvert::GetOptions(argc, argv);
  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (options.just_exit)
    return EXIT_FAILURE;

  std::unique_ptr<deinvert::AudioReader> reader;
  std::unique_ptr<deinvert::AudioWriter> writer;

  if (options.input_type == deinvert::InputType::sndfile) {
    try {
      reader = std::unique_ptr<deinvert::AudioReader>(new deinvert::SndfileReader(options));
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;
    }
    options.samplerate = reader->samplerate();
  } else {
    reader = std::unique_ptr<deinvert::AudioReader>(new deinvert::StdinReader(options));
  }

  if (options.output_type == deinvert::OutputType::wavfile) {
    try {
      writer = std::unique_ptr<deinvert::AudioWriter>(
          new deinvert::SndfileWriter(options.outfilename, options.samplerate));
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  } else {
    writer = std::unique_ptr<deinvert::AudioWriter>(new deinvert::RawPCMWriter());
  }

  if (options.is_split_band) {
    SplitBandDescramble(options, reader, writer);
  } else {
    SimpleDescramble(options, reader, writer);
  }
}
