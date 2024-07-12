use strict;
use warnings;
use IPC::Cmd qw(can_run);
use Carp;

# deinvert tests

my $binary                   = "../build/deinvert";
my $print_even_if_successful = 1;

my $test_file   = "test.wav";
my $output_file = "output.wav";

my $has_failures = 0;

can_run('sox') or croak 'sox is not installed!';

exit main();

sub main {
  system("uname -rms");

  testSimpleInversion();

  print $has_failures ? "Tests did not pass\n" : "All passed\n";

  return $has_failures;
}

sub testSimpleInversion {
  for my $test_frequency ( 500, 600, 700 ) {
    checkThatFrequencyInvertsAsItShould($test_frequency);
  }
  return;
}

sub checkThatFrequencyInvertsAsItShould {
  my ($test_frequency) = @_;
  my $inversion_carrier = 2632;
  generateTestSoundWithSimpleBeep($test_frequency);
  deinvertTestFileWithOptions( "-f " . $inversion_carrier );
  my $measured_frequency = findFrequencyOfOutputFile();
  my $expected_frequency =
    calculateExpectedInvertedFrequency( $test_frequency, $inversion_carrier );

  my $result = abs( $expected_frequency - $measured_frequency ) < 2;
  check( $result,
        "Carrier "
      . $inversion_carrier . " Hz: "
      . $test_frequency
      . " Hz  becomes "
      . $measured_frequency
      . ", should be ~"
      . $expected_frequency );

  return;
}

sub generateTestSoundWithSimpleBeep {
  my ($test_frequency) = @_;
  unlink($test_file);
  system( "sox -n -c 1 -e signed -b 16 -r 48k $test_file "
      . "synth sin $test_frequency trim 0 5 vol 0.5 fade 0.2" );
  return;
}

sub calculateExpectedInvertedFrequency {
  my ( $original_frequency, $inversion_carrier ) = @_;
  return $inversion_carrier - $original_frequency;
}

sub deinvertTestFileWithOptions {
  my ($options) = @_;
  system( $binary. " -i $test_file -o $output_file " . $options );
  return;
}

sub findFrequencyOfOutputFile {
  my $detected_frequency = 0;

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
      ( $dft[ $maxbin - 1 ]->[1] - 2 * $dft[$maxbin]->[1] + $dft[ $maxbin + 1 ]->[1] );
  }

  return $dft[$maxbin]->[0] + ( $dft[ $maxbin + 1 ]->[0] - $dft[$maxbin]->[0] ) * $delta;
}

# bool is expected to be true, otherwise fail with message
sub check {
  my ( $bool, $message ) = @_;
  if ( !$bool || $print_even_if_successful ) {
    print( ( $bool ? "[ OK ] " : "[FAIL] " ) . $message . "\n" );

    $has_failures = 1 if ( !$bool );
  }

  return;
}
