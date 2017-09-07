#include "wdsp.h"

#include <complex>

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

}  // namespace wdsp
