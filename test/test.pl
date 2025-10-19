#!/usr/bin/perl

# deinvert tests
# - Requires sox
# - Will create some WAV files in the current directory

use strict;
use warnings;
use IPC::Cmd qw(can_run);
use Carp;

my $binary                   = "../build/deinvert";
my $print_even_if_successful = 1;

my $test_file   = "test.wav";
my $output_file = "output.wav";

my $has_failures = 0;

can_run('sox') or croak 'sox is not installed!';

exit main();

sub main {
    system( "uname", "-rms" );

    testExeRunnable();

    # Can't continue if the exe is not runnable
    if ($has_failures) {
        return $has_failures;
    }

    testSimpleInversion();
    testSplitBandInversion();

    print $has_failures ? "\nTests did not pass\n" : "\nAll tests passed\n";

    return $has_failures;
}

sub testExeRunnable {
    check( -x $binary, "$binary should exist" );
}

# Run tests for the simple inversion case
# testSimpleInversion() -> void
sub testSimpleInversion {

    printf "\nSIMPLE %3s %10s %10s %10s %10s %10s\n", "Q", "Carrier",
      "Original",
      "Measured", "Expected",
      "Margin_hz";

    for my $quality ( 1, 3 ) {
        for my $inversion_carrier ( 2632, 3729 ) {
            for my $test_frequency ( 500, 600, 700 ) {
                checkSimpleInversionFor( $test_frequency, $inversion_carrier,
                    $quality );
            }
        }
    }
    return;
}

sub testSplitBandInversion {
    printf "\nSPLIT  %3s %10s %10s %10s %10s %10s %10s\n", "Q", "Carrier",
      "Split",
      "Original",
      "Measured", "Expected",
      "Margin_hz";

    my $split_point = 900;

    for my $quality ( 1, 3 ) {
        for my $inversion_carrier ( 2632, 3729 ) {
            for my $test_frequency ( 500, 600, 1100, 1200 ) {
                checkSplitBandInversionFor( $test_frequency, $inversion_carrier,
                    $split_point, $quality );
            }
        }
    }
    return;
}

# Generate test sound, run deinvert (simple inversion), measure output frequency
# checkSimpleInversionFor(test_frequency, inversion_carrier) -> void
sub checkSimpleInversionFor {
    my ( $test_frequency, $inversion_carrier, $quality ) = @_;
    generateTestSoundWithSimpleBeep($test_frequency);
    invertTestFileWithOptions( "-f", $inversion_carrier, "-q", $quality );
    my $measured_frequency = findFrequencyOfOutputFile();
    my $expected_frequency =
      calculateExpectedInvertedFrequency( $test_frequency, $inversion_carrier );

    my $frequency_tolerance_hz = 2;

    my $result = abs( $expected_frequency - $measured_frequency ) <
      $frequency_tolerance_hz;

    my $test_info = sprintf( "%3d %10d %10.2f %10.2f %10.2f %10.2f",
        $quality, $inversion_carrier, $test_frequency, $measured_frequency,
        $expected_frequency, $frequency_tolerance_hz );

    check( $result, $test_info );

    return;
}

# checkSplitBandInversionFor(test_frequency, inversion_carrier, split_point) -> void
sub checkSplitBandInversionFor {
    my ( $test_frequency, $inversion_carrier, $split_point, $quality ) = @_;
    generateTestSoundWithSimpleBeep($test_frequency);
    invertTestFileWithOptions( "-f", $inversion_carrier, "-s", $split_point,
        "-q", $quality );
    my $measured_frequency = findFrequencyOfOutputFile();
    my $expected_frequency = calculateExpectedInvertedFrequency(
          $test_frequency, $test_frequency < $split_point
        ? $split_point
        : $split_point + $inversion_carrier
    );

    my $frequency_tolerance_hz = 2;

    my $result = abs( $expected_frequency - $measured_frequency ) <
      $frequency_tolerance_hz;

    my $test_info = sprintf( "%3d %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f",
        $quality,            $inversion_carrier,  $split_point, $test_frequency,
        $measured_frequency, $expected_frequency, $frequency_tolerance_hz );

    check( $result, $test_info );

    return;
}

# generateTestSoundWithSimpleBeep(test_frequency) -> void
sub generateTestSoundWithSimpleBeep {
    my ($test_frequency) = @_;
    unlink($test_file);
    system( "sox -n -c 1 -e signed -b 16 -r 48k $test_file "
          . "synth sin $test_frequency trim 0 5 vol 0.5 fade 0.2" );
    return;
}

# calculateExpectedInvertedFrequency(original_frequency, inversion_carrier) -> expected_frequency
sub calculateExpectedInvertedFrequency {
    my ( $original_frequency, $inversion_carrier ) = @_;
    return $inversion_carrier - $original_frequency;
}

# Run deinvert with given options (results are in $output_file)
# invertTestFileWithOptions(options) -> void
sub invertTestFileWithOptions {
    my @options = @_;
    system( $binary, "-i", $test_file, "-o", $output_file, @options );
    return;
}

# Use sox to find the dominant frequency in the output file
# findFrequencyOfOutputFile() -> frequency_hz
sub findFrequencyOfOutputFile {
    my @dft;
    my $maxbin = 0;
    for (qx!sox $output_file -n stat -freq 2>&1!) {
        if (/^([\d\.]+)\s+([\d\.]+)/) {
            push @dft, [ $1, $2 ];
            $maxbin = $#dft if ( $2 > $dft[$maxbin]->[1] );
            last            if ( @dft == 4096 );
        }
    }

    # Parabolic FFT peak interpolation
    my $delta = 0;
    if ( $maxbin > 0 && $maxbin < $#dft ) {
        $delta =
          0.5 * ( $dft[ $maxbin - 1 ]->[1] - $dft[ $maxbin + 1 ]->[1] ) /
          ( $dft[ $maxbin - 1 ]->[1] -
              2 * $dft[$maxbin]->[1] +
              $dft[ $maxbin + 1 ]->[1] );
    }

    return $dft[$maxbin]->[0] +
      ( $dft[ $maxbin + 1 ]->[0] - $dft[$maxbin]->[0] ) * $delta;
}

# bool is expected to be true, otherwise fail with message
# check(bool, message) -> void
sub check {
    my ( $bool, $message ) = @_;
    if ( !$bool || $print_even_if_successful ) {
        print( ( $bool ? "[ OK ] " : "[FAIL] " ) . $message . "\n" );

        $has_failures = 1 if ( !$bool );
    }

    return;
}
