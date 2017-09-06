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
#ifndef LIQUID_WRAPPERS_H_
#define LIQUID_WRAPPERS_H_

#include "config.h"
#ifdef HAVE_LIQUID

#include <complex>
#include <vector>

#include "liquid/liquid.h"

namespace liquid {

class FIRFilter {
 public:
  FIRFilter(int len, float fc, float As = 60.0f, float mu = 0.0f);
  ~FIRFilter();
  void push(std::complex<float> s);
  std::complex<float> execute();

 private:
  firfilt_crcf object_;
};

class NCO {
 public:
  explicit NCO(liquid_ncotype type, float freq);
  ~NCO();
  std::complex<float> MixDown(std::complex<float> s);
  std::complex<float> MixUp(std::complex<float> s);
  void MixBlockDown(std::complex<float>* x, std::complex<float>* y,
      int n);
  void Step();
  void set_pll_bandwidth(float);
  void StepPLL(float dphi);
  float frequency();

 private:
  nco_crcf object_;
};

class Resampler {
 public:
  explicit Resampler(float ratio, int length);
  ~Resampler();
  unsigned int execute(std::complex<float> in, std::complex<float>* out);
  void set_rate(float rate);

 private:
  resamp_crcf object_;
  std::complex<float> outbuffer_[2];
};

}  // namespace liquid

#endif  // HAVE_LIQUID

#endif  // LIQUID_WRAPPERS_H_
