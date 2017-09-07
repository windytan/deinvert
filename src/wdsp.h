#ifndef WDSP_H_
#define WDSP_H_

#include <complex>

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

}  // namespace wdsp
#endif  // WDSP_H_
