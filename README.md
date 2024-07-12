# deinvert

deinvert is a voice inversion scrambler and descrambler. It supports simple
inversion as well as split-band inversion.

NOTE: Frequency inversion is NOT encryption! It's very easy to decode. This
program is a toy and should never be used for security purposes.

See the [wiki](https://github.com/windytan/deinvert/wiki) for detailed usage
instructions and examples.

## Prerequisites

deinvert requires liquid-dsp, libsndfile, and meson.

On Ubuntu, these can be installed like so:

    sudo apt install libsndfile1-dev libliquid-dev meson

On older Debians:

    sudo apt-get install python3-pip ninja-build build-essential libsndfile1-dev libliquid-dev
    pip3 install --user meson
    sudo ldconfig

On macOS I recommend using [homebrew](https://brew.sh/):

    xcode-select --install
    brew install libsndfile liquid-dsp meson

## Compiling

    meson setup build
    cd build
    meson compile

## Usage

Note that since scrambling and descrambling are the same operation this
tool also works as a scrambler.

If no input and output file is given, deinvert reads raw 16-bit PCM via stdin
and outputs in the same format via stdout. The inversion carrier defaults to
2632 Hz.

### Simple inversion, WAV input

(De)scrambling a WAV file with setting 4:

    ./build/deinvert -i input.wav -o output.wav -p 4

### Split-band inversion

(De)scrambling split-band inversion with a bandwidth of 3500 Hz, split at 1200 Hz:

    ./build/deinvert -i input.wav -o output.wav -f 3500 -s 1200

### Invert a live signal from RTL-SDR

Descrambling a live FM channel at 27 Megahertz from an RTL-SDR, setting 4:

    rtl_fm -M fm -f 27.0M -s 12k -g 50 -l 70 | ./build/deinvert -r 12000 -p 4 |\
      play -r 12k -c 1 -t .s16 -

### Invert a live signal from Gqrx (requires netcat)

1. Set Gqrx to demodulate the audio (for example, narrow FM).
2. Go to the Audio window and click on the three dots button "...".
3. Go to Network and set host to localhost and port to e.g. 12345.
4. In the Audio window, enable UDP.
5. Run this command in a terminal window:

    nc -u -l localhost 12345 | ./build/deinvert -r 48000 | play -r 48k -c 1 -t .s16 -


### Full options

    ./build/deinvert [OPTIONS]

    -f, --frequency FREQ   Frequency of the inversion carrier, in Hertz.

    -h, --help             Display this usage help.

    -i, --input-file FILE  Use an audio file as input. All formats
                           supported by libsndfile should work.

    -o, --output-file FILE Write output to a WAV file instead of stdout. An
                           existing file will be overwritten.
                           The input sample rate will be used.

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

### I can't understand the speech even after deinverting

In this case, the sample is probably not frequency inversion scrambled.
It's very rare to encounter frequency inversion scrambling nowadays. See the
[wiki](https://github.com/windytan/deinvert/wiki) for details.

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
