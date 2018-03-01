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
#ifndef WDSP_H_
#define WDSP_H_

#include <complex>
#include <vector>

namespace wdsp {

class NCO {
 public:
  explicit NCO(double frequency);
  void Step();
  std::complex<float> MixUp(std::complex<float> sample_in) const;

 private:
  double frequency_;
  double phase_;
};

class DCRemover {
 public:
  explicit DCRemover(size_t length);
  void push(float sample);
  float execute(float sample);

 private:
  std::vector<float> buffer_;
  size_t index_;
  bool is_filled_;
};

}  // namespace wdsp
#endif  // WDSP_H_
