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
#include "wdsp.h"

#include <complex>
#include <numeric>

namespace wdsp {

NCO::NCO(double frequency) : frequency_(frequency), phase_(0.0) {
}

void NCO::Step() {
  phase_ += frequency_;
}

std::complex<float> NCO::MixUp(std::complex<float> sample_in) const {
  return {real(sample_in) * cosf(phase_) - imag(sample_in) * sinf(phase_),
          imag(sample_in) * cosf(phase_) + real(sample_in) * sinf(phase_)};
}

DCRemover::DCRemover(size_t length) : buffer_(length), index_(0),
                                      is_filled_(false) {
}

void DCRemover::push(float sample) {
  if (buffer_.size() > 0) {
    buffer_[index_] = sample;
    index_ = (index_ + 1) % buffer_.size();

    if (index_ == 0)
      is_filled_ = true;
  }
}

float DCRemover::execute(float sample) {
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

}  // namespace wdsp
