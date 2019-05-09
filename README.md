# deinvert

deinvert is a voice inversion scrambler and descrambler. It supports simple
inversion as well as split-band inversion.

## Prerequisites
By default, deinvert requires liquid-dsp, libsndfile, and GNU autotools. On
Ubuntu, these can be installed like so:

    sudo apt install libsndfile1-dev libliquid-dev automake
    sudo ldconfig

On macOS I recommend using [homebrew](https://brew.sh/):

    xcode-select --install
    brew install libsndfile liquid-dsp automake

But deinvert can be compiled without liquid-dsp using the configure
option `--without-liquid`; filtering will be disabled and the result will not
sound as good. It can also be compiled
without libsndfile using the configure option `--without-sndfile`; WAV
support will be disabled, only raw input/output will work.

## Compiling

    ./autogen.sh
    ./configure [--without-liquid] [--without-sndfile]
    make

## Usage

Note that since scrambling and descrambling are the same operation this
tool also works as a scrambler.

If no input and output file is given, deinvert reads raw 16-bit PCM via stdin
and outputs in the same format via stdout. The inversion carrier defaults to
2632 Hz.

### Simple inversion, WAV input

(De)scrambling a WAV file with setting 4:

    ./src/deinvert -i input.wav -o output.wav -p 4

### Split-band inversion

(De)scrambling split-band inversion with a bandwidth of 3500 Hz, split at 1200 Hz:

    ./src/deinvert -i input.wav -o output.wav -f 3500 -s 1200

### Invert a live signal from RTL-SDR

Descrambling a live FM channel at 27 Megahertz from an RTL-SDR, setting 4:

    rtl_fm -M fm -f 27.0M -s 12k -g 50 -l 70 | ./src/deinvert -r 12000 -p 4 |\
      play -r 12k -c 1 -t .s16 -

### Invert a live signal from Gqrx (requires netcat)

1. Set Gqrx to demodulate the audio (for example, narrow FM).
2. Go to the Audio window and click on the three dots button "...".
3. Go to Network and set host to localhost and port to e.g. 12345.
4. In the Audio window, enable UDP.
5. Run this command in a terminal window:

    nc -u -l localhost 12345 | ./src/deinvert -r 48000 | play -r 48k -c 1 -t .s16 -


### Full options

    ./src/deinvert [OPTIONS]

    -f, --frequency FREQ   Frequency of the inversion carrier, in Hertz.

    -h, --help             Display this usage help.

    -i, --input-file FILE  Use an audio file as input. All formats
                           supported by libsndfile should work.

    -o, --output-file FILE Write output to a WAV file instead of stdout. An
                           existing file will be overwritten.

    -p, --preset NUM       Scrambler frequency preset (1-8), referring to
                           the set of common carrier frequencies used by
                           e.g. the Selectone ST-20B scrambler.

    -q, --quality NUM      Filter quality, from 0 (worst quality, low CPU
                           usage) to 3 (best quality, higher CPU usage). The
                           default is 2.

    -r, --samplerate RATE  Sampling rate of raw input audio, in Hertz.

    -s, --split-frequency  Split point for split-band inversion, in Hertz.

    -v, --version          Display version string.

## Inversion carrier presets

| No | Frequency |
| -- | --------- |
|  1 | 2632 Hz   |
|  2 | 2718 Hz   |
|  3 | 2868 Hz   |
|  4 | 3023 Hz   |
|  5 | 3196 Hz   |
|  6 | 3339 Hz   |
|  7 | 3495 Hz   |
|  8 | 3729 Hz   |

## Troubleshooting

### Can't find liquid-dsp on macOS

If you've installed liquid-dsp yet `configure` can't find it, it's possible that
XCode command line tools aren't installed. Run this command to fix it:

    xcode-select --install

### Can't find liquid-dsp on Linux

Try running this in the terminal:

    sudo ldconfig

### I hear a high-pitched beep in the result

This could be because the transmission has a CTCSS subtone.
It can be fixed by running a low-pass filter after deinvert. The cut-off
frequency should be around 250 Hz less than the inversion carrier frequency. For
example, if the inversion carrier is 3023 Hz, the `play` command could be
changed to:

    play -r 12000 -c 1 -t .s16 - sinc -2800

## Licensing

deinvert is released under the MIT license, which means it is copyrighted to Oona
Räisänen OH2EIQ yet you're free to use it provided that the copyright
information is not removed. See [LICENSE](LICENSE).
