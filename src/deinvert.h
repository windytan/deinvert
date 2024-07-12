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
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "config.h"
#include "src/io.h"
#include "src/liquid_wrappers.h"

namespace deinvert {

class DCRemover {
 public:
  explicit DCRemover(size_t length);
  void  push(float sample);
  float execute(float sample) const;

 private:
  std::vector<float> buffer_;
  size_t             index_{};
  bool               is_filled_{};
};

class Inverter {
 public:
  Inverter(float freq_prefilter, float freq_shift, float freq_postfilter, float samplerate,
           int filter_quality);
  float execute(float insample);

 private:
  const std::vector<float> filter_lengths_;
  const std::vector<float> filter_attenuation_;
  liquid::FIRFilter        prefilter_;
  liquid::FIRFilter        postfilter_;
  liquid::NCO              oscillator_;
  const bool               do_filter_;
};

}  // namespace deinvert
