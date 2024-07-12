/*
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

#include "config.h"

#include <complex>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
// https://github.com/jgaeddert/liquid-dsp/issues/229
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" {
#include "liquid/liquid.h"
}
#pragma clang diagnostic pop

namespace liquid {

class FIRFilter {
 public:
  FIRFilter(int len, float fc, float As = 80.0f, float mu = 0.0f);
  ~FIRFilter();
  void  push(float s);
  float execute();

 private:
  firfilt_rrrf object_;
};

class NCO {
 public:
  explicit NCO(liquid_ncotype type, float freq);
  ~NCO();
  std::complex<float> MixUp(std::complex<float> s);
  void                Step();

 private:
  nco_crcf object_;
};

}  // namespace liquid
