deinvert is a descrambler for voice inversion scrambling.

## Prerequisites
deinvert requires liquid-dsp and libsndfile. But it can be compiled without
liquid-dsp using the configure option `--without-liquid`; filtering
will be disabled.

## Compiling

    ./autogen.sh
    ./configure
    make

## Usage

By default, deinvert reads 16-bit, 44.1 kHz PCM via stdin and outputs in the
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

    -r, --samplerate RATE  Sampling rate of raw input audio, in Hertz.

    -v, --version          Display version string.
