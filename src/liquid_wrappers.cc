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

  object_ = firfilt_rrrf_create_kaiser(len, fc, As, mu);
  firfilt_rrrf_set_scale(object_, 2.0f * fc);
}

FIRFilter::~FIRFilter() {
  firfilt_rrrf_destroy(object_);
}

void FIRFilter::push(float s) {
  firfilt_rrrf_push(object_, s);
}

float FIRFilter::execute() {
  float result;
  firfilt_rrrf_execute(object_, &result);
  return result;
}

NCO::NCO(liquid_ncotype type, float freq) :
    object_(nco_crcf_create(type)) {
  nco_crcf_set_frequency(object_, freq);
}

NCO::~NCO() {
  nco_crcf_destroy(object_);
}

std::complex<float> NCO::MixUp(std::complex<float> s) {
  std::complex<float> result;
  nco_crcf_mix_up(object_, s, &result);
  return result;
}

void NCO::Step() {
  nco_crcf_step(object_);
}

float NCO::frequency() {
  return nco_crcf_get_frequency(object_);
}

}  // namespace liquid

#endif
