deinvert is a descrambler for voice inversion scrambling.

## Prerequisites
deinvert requires [liquid-dsp](https://github.com/jgaeddert/liquid-dsp) and
libsndfile.

But it can be compiled without liquid-dsp using the configure
option `--without-liquid`; filtering will be disabled. And it can be compiled
without libsndfile using the configure option `--without-sndfile`; WAV
support will be disabled.

## Compiling

    ./autogen.sh
    ./configure [--without-liquid] [--without-sndfile]
    make

## Usage

By default, deinvert reads raw 16-bit, 44.1 kHz PCM via stdin and outputs in the
same format via stdout. The inversion carrier defaults to 2632 Hz.

    deinvert [OPTIONS]

    -f, --frequency FREQ   Frequency of the inversion carrier, in Hertz.

    -h, --help             Display this usage help.

    -i, --input-file FILE  Use an audio file as input. All formats
                           supported by libsndfile should work.

    -n, --nofilter         Disable filtering (faster).

    -o, --output-file FILE Write output to a WAV file instead of stdout. An
                           existing file will be overwritten.

    -p, --preset NUM       Scrambler frequency preset (1-8), referring to
                           the set of common carrier frequencies used by
                           e.g. the Selectone ST-20B scrambler.

    -r, --samplerate RATE  Sampling rate of raw input (and output) audio, in
                           Hertz.

    -v, --version          Display version string.

## Troubleshooting

### Can't find liquid-dsp on macOS

If you've installed liquid-dsp yet `configure` can't find it, it's possible that
XCode command line tools aren't installed. Run this command to fix it:

    xcode-select --install

### Can't find liquid-dsp on Linux

Try running this in the terminal:

    sudo ldconfig

## Licensing

deinvert is released under the MIT license, which means it is copyrighted to Oona
Räisänen OH2EIQ yet you're free to use it provided that the copyright
information is not removed. See [LICENSE](LICENSE).
