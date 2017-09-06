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
#include "src/liquid_wrappers.h"

#include "config.h"
#ifdef HAVE_LIQUID

#include <cassert>
#include <complex>

#include "liquid/liquid.h"

namespace liquid {

FIRFilter::FIRFilter(int len, float fc, float As, float mu) {
  assert(fc >= 0.0f && fc <= 0.5f);
  assert(As > 0.0f);
  assert(mu >= -0.5f && mu <= 0.5f);

  object_ = firfilt_crcf_create_kaiser(len, fc, As, mu);
  firfilt_crcf_set_scale(object_, 2.0f * fc);
}

FIRFilter::~FIRFilter() {
  firfilt_crcf_destroy(object_);
}

void FIRFilter::push(std::complex<float> s) {
  firfilt_crcf_push(object_, s);
}

std::complex<float> FIRFilter::execute() {
  std::complex<float> result;
  firfilt_crcf_execute(object_, &result);
  return result;
}

NCO::NCO(liquid_ncotype type, float freq) :
    object_(nco_crcf_create(type)) {
  nco_crcf_set_frequency(object_, freq);
}

NCO::~NCO() {
  nco_crcf_destroy(object_);
}

std::complex<float> NCO::MixDown(std::complex<float> s) {
  std::complex<float> result;
  nco_crcf_mix_down(object_, s, &result);
  return result;
}

std::complex<float> NCO::MixUp(std::complex<float> s) {
  std::complex<float> result;
  nco_crcf_mix_up(object_, s, &result);
  return result;
}

void NCO::MixBlockDown(std::complex<float>* x, std::complex<float>* y,
    int n) {
  nco_crcf_mix_block_down(object_, x, y, n);
}

void NCO::Step() {
  nco_crcf_step(object_);
}

void NCO::set_pll_bandwidth(float bw) {
  nco_crcf_pll_set_bandwidth(object_, bw);
}

void NCO::StepPLL(float dphi) {
  nco_crcf_pll_step(object_, dphi);
}

float NCO::frequency() {
  return nco_crcf_get_frequency(object_);
}

Resampler::Resampler(float ratio, int length) :
    object_(resamp_crcf_create(ratio, length, 0.47f, 60.0f, 32)) {
  assert(ratio <= 2.0f);
}

Resampler::~Resampler() {
  resamp_crcf_destroy(object_);
}

void Resampler::set_rate(float rate) {
  resamp_crcf_set_rate(object_, rate);
}

unsigned int Resampler::execute(std::complex<float> in,
                                std::complex<float>* out) {
  unsigned int num_written;
  resamp_crcf_execute(object_, in, out, &num_written);

  return num_written;
}

}  // namespace liquid

#endif
