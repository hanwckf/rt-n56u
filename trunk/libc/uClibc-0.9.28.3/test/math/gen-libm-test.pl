#!/usr/bin/perl -w
# Copyright (C) 1999 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Andreas Jaeger <aj@suse.de>, 1999.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

# This file needs to be tidied up
# Note that functions and tests share the same namespace.

# Information about tests are stored in: %results
# $results{$test}{"kind"} is either "fct" or "test" and flags whether this
# is a maximal error of a function or a single test.
# $results{$test}{"type"} is the result type, e.g. normal or complex.
# $results{$test}{"has_ulps"} is set if deltas exist.
# $results{$test}{"has_fails"} is set if exptected failures exist.
# In the following description $type and $float are:
# - $type is either "normal", "real" (for the real part of a complex number)
#   or "imag" (for the imaginary part # of a complex number).
# - $float is either of float, ifloat, double, idouble, ldouble, ildouble;
#   It represents the underlying floating point type (float, double or long
#   double) and if inline functions (the leading i stands for inline)
#   are used.
# $results{$test}{$type}{"fail"}{$float} is defined and has a 1 if
# the test is expected to fail
# $results{$test}{$type}{"ulp"}{$float} is defined and has a delta as value


use Getopt::Std;

use strict;

use vars qw ($input $output);
use vars qw (%results);
use vars qw (@tests @functions);
use vars qw ($count);
use vars qw (%beautify @all_floats);
use vars qw ($output_dir $ulps_file);

# all_floats is sorted and contains all recognised float types
@all_floats = ('double', 'float', 'idouble',
	       'ifloat', 'ildouble', 'ldouble');

%beautify =
  ( "minus_zero" => "-0",
    "plus_zero" => "+0",
    "minus_infty" => "-inf",
    "plus_infty" => "inf",
    "nan_value" => "NaN",
    "M_El" => "e",
    "M_E2l" => "e^2",
    "M_E3l" => "e^3",
    "M_LOG10El", "log10(e)",
    "M_PIl" => "pi",
    "M_PI_34l" => "3/4 pi",
    "M_PI_2l" => "pi/2",
    "M_PI_4l" => "pi/4",
    "M_PI_6l" => "pi/6",
    "M_PI_34_LOG10El" => "3/4 pi*log10(e)",
    "M_PI_LOG10El" => "pi*log10(e)",
    "M_PI2_LOG10El" => "pi/2*log10(e)",
    "M_PI4_LOG10El" => "pi/4*log10(e)",
    "M_LOG_SQRT_PIl" => "log(sqrt(pi))",
    "M_LOG_2_SQRT_PIl" => "log(2*sqrt(pi))",
    "M_2_SQRT_PIl" => "2 sqrt (pi)",
    "M_SQRT_PIl" => "sqrt (pi)",
    "INVALID_EXCEPTION" => "invalid exception",
    "DIVIDE_BY_ZERO_EXCEPTION" => "division by zero exception",
    "INVALID_EXCEPTION_OK" => "invalid exception allowed",
    "DIVIDE_BY_ZERO_EXCEPTION_OK" => "division by zero exception allowed",
    "EXCEPTIONS_OK" => "exceptions allowed",
    "IGNORE_ZERO_INF_SIGN" => "sign of zero/inf not specified",
"INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN" => "invalid exception and sign of zero/inf not specified"
  );


# get Options
# Options:
# u: ulps-file
# h: help
# o: output-directory
# n: generate new ulps file
use vars qw($opt_u $opt_h $opt_o $opt_n);
getopts('u:o:nh');

$ulps_file = 'libm-test-ulps';
$output_dir = '';

if ($opt_h) {
  print "Usage: gen-libm-test.pl [OPTIONS]\n";
  print " -h         print this help, then exit\n";
  print " -o DIR     directory where generated files will be placed\n";
  print " -n         only generate sorted file NewUlps from libm-test-ulps\n";
  print " -u FILE    input file with ulps\n";
  exit 0;
}

$ulps_file = $opt_u if ($opt_u);
$output_dir = $opt_o if ($opt_o);

$input = "libm-test.inc";
$output = "${output_dir}libm-test.c";

$count = 0;

&parse_ulps ($ulps_file);
&generate_testfile ($input, $output) unless ($opt_n);
&output_ulps ("${output_dir}libm-test-ulps.h", $ulps_file) unless ($opt_n);
&print_ulps_file ("${output_dir}NewUlps") if ($opt_n);

# Return a nicer representation
sub beautify {
  my ($arg) = @_;
  my ($tmp);

  if (exists $beautify{$arg}) {
    return $beautify{$arg};
  }
  if ($arg =~ /^-/) {
    $tmp = $arg;
    $tmp =~ s/^-//;
    if (exists $beautify{$tmp}) {
      return '-' . $beautify{$tmp};
    }
  }
  if ($arg =~ /[0-9]L$/) {
    $arg =~ s/L$//;
  }
  return $arg;
}

# Return a nicer representation of a complex number
sub build_complex_beautify {
  my ($r, $i) = @_;
  my ($str1, $str2);

  $str1 = &beautify ($r);
  $str2 = &beautify ($i);
  if ($str2 =~ /^-/) {
    $str2 =~ s/^-//;
    $str1 .= ' - ' . $str2;
  } else {
    $str1 .= ' + ' . $str2;
  }
  $str1 .= ' i';
  return $str1;
}

# Return name of a variable
sub get_variable {
  my ($number) = @_;

  return "x" if ($number == 1);
  return "y" if ($number == 2);
  return "z" if ($number == 3);
  # return x1,x2,...
  $number =-3;
  return "x$number";
}

# Add a new test to internal data structures and fill in the
# ulps, failures and exception information for the C line.
sub new_test {
  my ($test, $exception) = @_;
  my $rest;

  # Add ulp, xfail
  if (exists $results{$test}{'has_ulps'}) {
    $rest = ", DELTA$count";
  } else {
    $rest = ', 0';
  }
  if (exists $results{$test}{'has_fails'}) {
    $rest .= ", FAIL$count";
  } else {
    $rest .= ', 0';
  }
  if (defined $exception) {
    $rest .= ", $exception";
  } else {
    $rest .= ', 0';
  }
  $rest .= ");\n";
  # We must increment here to keep @tests and count in sync
  push @tests, $test;
  ++$count;
  return $rest;
}

# Treat some functions especially.
# Currently only sincos needs extra treatment.
sub special_functions {
  my ($file, $args) = @_;
  my (@args, $str, $test, $cline);

  @args = split /,\s*/, $args;

  unless ($args[0] =~ /sincos/) {
    die ("Don't know how to handle $args[0] extra.");
  }
  print $file "  FUNC (sincos) ($args[1], &sin_res, &cos_res);\n";

  $str = 'sincos (' . &beautify ($args[1]) . ', &sin_res, &cos_res)';
  # handle sin
  $test = $str . ' puts ' . &beautify ($args[2]) . ' in sin_res';
  if ($#args == 4) {
    $test .= " plus " . &beautify ($args[4]);
  }

  $cline = "  check_float (\"$test\", sin_res, $args[2]";
  $cline .= &new_test ($test, $args[4]);
  print $file $cline;

  # handle cos
  $test = $str . ' puts ' . &beautify ($args[3]) . ' in cos_res';
  $cline = "  check_float (\"$test\", cos_res, $args[3]";
  # only tests once for exception
  $cline .= &new_test ($test, undef);
  print $file $cline;
}

# Parse the arguments to TEST_x_y
sub parse_args {
  my ($file, $descr, $args) = @_;
  my (@args, $str, $descr_args, $descr_res, @descr);
  my ($current_arg, $cline, $i);
  my ($pre, $post, @special);
  my ($extra_var, $call, $c_call);

  if ($descr eq 'extra') {
    &special_functions ($file, $args);
    return;
  }
  ($descr_args, $descr_res) = split /_/,$descr, 2;

  @args = split /,\s*/, $args;

  $call = "$args[0] (";

  # Generate first the string that's shown to the user
  $current_arg = 1;
  $extra_var = 0;
  @descr = split //,$descr_args;
  for ($i = 0; $i <= $#descr; $i++) {
    if ($i >= 1) {
      $call .= ', ';
    }
    # FLOAT, int, long int, long long int
    if ($descr[$i] =~ /f|i|l|L/) {
      $call .= &beautify ($args[$current_arg]);
      ++$current_arg;
      next;
    }
    # &FLOAT, &int - argument is added here
    if ($descr[$i] =~ /F|I/) {
      ++$extra_var;
      $call .= '&' . &get_variable ($extra_var);
      next;
    }
    # complex
    if ($descr[$i] eq 'c') {
      $call .= &build_complex_beautify ($args[$current_arg], $args[$current_arg+1]);
      $current_arg += 2;
      next;
    }

    die ("$descr[$i] is unknown");
  }
  $call .= ')';
  $str = "$call == ";

  # Result
  @descr = split //,$descr_res;
  foreach (@descr) {
    if ($_ =~ /f|i|l|L/) {
      $str .= &beautify ($args[$current_arg]);
      ++$current_arg;
    } elsif ($_ eq 'c') {
      $str .= &build_complex_beautify ($args[$current_arg], $args[$current_arg+1]);
      $current_arg += 2;
    } elsif ($_ eq 'b') {
      # boolean
      $str .= ($args[$current_arg] == 0) ? "false" : "true";
      ++$current_arg;
    } elsif ($_ eq '1') {
      ++$current_arg;
    } else {
      die ("$_ is unknown");
    }
  }
  # consistency check
  if ($current_arg == $#args) {
    die ("wrong number of arguments")
      unless ($args[$current_arg] =~ /EXCEPTION|IGNORE_ZERO_INF_SIGN/);
  } elsif ($current_arg < $#args) {
    die ("wrong number of arguments");
  } elsif ($current_arg > ($#args+1)) {
    die ("wrong number of arguments");
  }


  # check for exceptions
  if ($current_arg <= $#args) {
    $str .= " plus " . &beautify ($args[$current_arg]);
  }

  # Put the C program line together
  # Reset some variables to start again
  $current_arg = 1;
  $extra_var = 0;
  if (substr($descr_res,0,1) eq 'f') {
    $cline = 'check_float'
  } elsif (substr($descr_res,0,1) eq 'b') {
    $cline = 'check_bool';
  } elsif (substr($descr_res,0,1) eq 'c') {
    $cline = 'check_complex';
  } elsif (substr($descr_res,0,1) eq 'i') {
    $cline = 'check_int';
  } elsif (substr($descr_res,0,1) eq 'l') {
    $cline = 'check_long';
  } elsif (substr($descr_res,0,1) eq 'L') {
    $cline = 'check_longlong';
  }
  # Special handling for some macros:
  $cline .= " (\"$str\", ";
  if ($args[0] =~ /fpclassify|isnormal|isfinite|signbit/) {
    $c_call = "$args[0] (";
  } else {
    $c_call = " FUNC($args[0]) (";
  }
  @descr = split //,$descr_args;
  for ($i=0; $i <= $#descr; $i++) {
    if ($i >= 1) {
      $c_call .= ', ';
    }
    # FLOAT, int, long int, long long int
    if ($descr[$i] =~ /f|i|l|L/) {
      $c_call .= $args[$current_arg];
      $current_arg++;
      next;
    }
    # &FLOAT, &int
    if ($descr[$i] =~ /F|I/) {
      ++$extra_var;
      $c_call .= '&' . &get_variable ($extra_var);
      next;
    }
    # complex
    if ($descr[$i] eq 'c') {
      $c_call .= "BUILD_COMPLEX ($args[$current_arg], $args[$current_arg+1])";
      $current_arg += 2;
      next;
    }
  }
  $c_call .= ')';
  $cline .= "$c_call, ";

  @descr = split //,$descr_res;
  foreach (@descr) {
    if ($_ =~ /b|f|i|l|L/ ) {
      $cline .= $args[$current_arg];
      $current_arg++;
    } elsif ($_ eq 'c') {
      $cline .= "BUILD_COMPLEX ($args[$current_arg], $args[$current_arg+1])";
      $current_arg += 2;
    } elsif ($_ eq '1') {
      push @special, $args[$current_arg];
      ++$current_arg;
    }
  }
  # Add ulp, xfail
  $cline .= &new_test ($str, ($current_arg <= $#args) ? $args[$current_arg] : undef);

  # special treatment for some functions
  if ($args[0] eq 'frexp') {
    if (defined $special[0] && $special[0] ne "IGNORE") {
      my ($str) = "$call sets x to $special[0]";
      $post = "  check_int (\"$str\", x, $special[0]";
      $post .= &new_test ($str, undef);
    }
  } elsif ($args[0] eq 'gamma' || $args[0] eq 'lgamma') {
    $pre = "  signgam = 0;\n";
    if (defined $special[0] && $special[0] ne "IGNORE") {
      my ($str) = "$call sets signgam to $special[0]";
      $post = "  check_int (\"$str\", signgam, $special[0]";
      $post .= &new_test ($str, undef);
    }
  } elsif ($args[0] eq 'modf') {
    if (defined $special[0] && $special[0] ne "IGNORE") {
      my ($str) = "$call sets x to $special[0]";
      $post = "  check_float (\"$str\", x, $special[0]";
      $post .= &new_test ($str, undef);
    }
  } elsif ($args[0] eq 'remquo') {
    if (defined $special[0] && $special[0] ne "IGNORE") {
      my ($str) = "$call sets x to $special[0]";
      $post = "  check_int (\"$str\", x, $special[0]";
      $post .= &new_test ($str, undef);
    }
  }

  print $file $pre if (defined $pre);

  print $file "  $cline";

  print $file $post if (defined $post);
}

# Generate libm-test.c
sub generate_testfile {
  my ($input, $output) = @_;
  my ($lasttext);
  my (@args, $i, $str);

  open INPUT, $input or die ("Can't open $input: $!");
  open OUTPUT, ">$output" or die ("Can't open $output: $!");

  # Replace the special macros
  while (<INPUT>) {

    # TEST_...
    if (/^\s*TEST_/) {
      my ($descr, $args);
      chop;
      ($descr, $args) = ($_ =~ /TEST_(\w+)\s*\((.*)\)/);
      &parse_args (\*OUTPUT, $descr, $args);
      next;
    }
    # START (function)
    if (/START/) {
      print OUTPUT "  init_max_error ();\n";
      next;
    }
    # END (function)
    if (/END/) {
      my ($fct, $line, $type);
      if (/complex/) {
	s/,\s*complex\s*//;
	$type = 'complex';
      } else {
	$type = 'normal';
      }
      ($fct) = ($_ =~ /END\s*\((.*)\)/);
      if ($type eq 'complex') {
	$line = "  print_complex_max_error (\"$fct\", ";
      } else {
	$line = "  print_max_error (\"$fct\", ";
      }
      if (exists $results{$fct}{'has_ulps'}) {
	$line .= "DELTA$fct";
      } else {
	$line .= '0';
      }
      if (exists $results{$fct}{'has_fails'}) {
	$line .= ", FAIL$fct";
      } else {
	$line .= ', 0';
      }
      $line .= ");\n";
      print OUTPUT $line;
      push @functions, $fct;
      next;
    }
    print OUTPUT;
  }
  close INPUT;
  close OUTPUT;
}



# Parse ulps file
sub parse_ulps {
  my ($file) = @_;
  my ($test, $type, $float, $eps, $kind);

  # $type has the following values:
  # "normal": No complex variable
  # "real": Real part of complex result
  # "imag": Imaginary part of complex result
  open ULP, $file  or die ("Can't open $file: $!");
  while (<ULP>) {
    chop;
    # ignore comments and empty lines
    next if /^#/;
    next if /^\s*$/;
    if (/^Test/) {
      if (/Real part of:/) {
	s/Real part of: //;
	$type = 'real';
      } elsif (/Imaginary part of:/) {
	s/Imaginary part of: //;
	$type = 'imag';
      } else {
	$type = 'normal';
      }
      s/^.+\"(.*)\".*$/$1/;
      $test = $_;
      $kind = 'test';
      next;
    }
    if (/^Function: /) {
      if (/Real part of/) {
	s/Real part of //;
	$type = 'real';
      } elsif (/Imaginary part of/) {
	s/Imaginary part of //;
	$type = 'imag';
      } else {
	$type = 'normal';
      }
      ($test) = ($_ =~ /^Function:\s*\"([a-zA-Z0-9_]+)\"/);
      $kind = 'fct';
      next;
    }
    if (/^i?(float|double|ldouble):/) {
      ($float, $eps) = split /\s*:\s*/,$_,2;

      if ($eps eq 'fail') {
	$results{$test}{$type}{'fail'}{$float} = 1;
	$results{$test}{'has_fails'} = 1;
      } elsif ($eps eq "0") {
	# ignore
	next;
      } else {
	$results{$test}{$type}{'ulp'}{$float} = $eps;
	$results{$test}{'has_ulps'} = 1;
      }
      if ($type =~ /^real|imag$/) {
	$results{$test}{'type'} = 'complex';
      } elsif ($type eq 'normal') {
	$results{$test}{'type'} = 'normal';
      }
      $results{$test}{'kind'} = $kind;
      next;
    }
    print "Skipping unknown entry: `$_'\n";
  }
  close ULP;
}


# Clean up a floating point number
sub clean_up_number {
  my ($number) = @_;

  # Remove trailing zeros
  $number =~ s/0+$//;
  $number =~ s/\.$//;
  return $number;
}

# Output a file which can be read in as ulps file.
sub print_ulps_file {
  my ($file) = @_;
  my ($test, $type, $float, $eps, $fct, $last_fct);

  $last_fct = '';
  open NEWULP, ">$file" or die ("Can't open $file: $!");
  print NEWULP "# Begin of automatic generation\n";
  # first the function calls
  foreach $test (sort keys %results) {
    next if ($results{$test}{'kind'} ne 'test');
    foreach $type ('real', 'imag', 'normal') {
      if (exists $results{$test}{$type}) {
	if (defined $results{$test}) {
	  ($fct) = ($test =~ /^(\w+)\s/);
	  if ($fct ne $last_fct) {
	    $last_fct = $fct;
	    print NEWULP "\n# $fct\n";
	  }
	}
	if ($type eq 'normal') {
	  print NEWULP "Test \"$test\":\n";
	} elsif ($type eq 'real') {
	  print NEWULP "Test \"Real part of: $test\":\n";
	} elsif ($type eq 'imag') {
	  print NEWULP "Test \"Imaginary part of: $test\":\n";
	}
	foreach $float (@all_floats) {
	  if (exists $results{$test}{$type}{'ulp'}{$float}) {
	    print NEWULP "$float: ",
	    &clean_up_number ($results{$test}{$type}{'ulp'}{$float}),
	    "\n";
	  }
	  if (exists $results{$test}{$type}{'fail'}{$float}) {
	    print NEWULP "$float: fail\n";
	  }
	}
      }
    }
  }
  print NEWULP "\n# Maximal error of functions:\n";

  foreach $fct (sort keys %results) {
    next if ($results{$fct}{'kind'} ne 'fct');
    foreach $type ('real', 'imag', 'normal') {
      if (exists $results{$fct}{$type}) {
	if ($type eq 'normal') {
	  print NEWULP "Function: \"$fct\":\n";
	} elsif ($type eq 'real') {
	  print NEWULP "Function: Real part of \"$fct\":\n";
	} elsif ($type eq 'imag') {
	  print NEWULP "Function: Imaginary part of \"$fct\":\n";
	}
	foreach $float (@all_floats) {
	  if (exists $results{$fct}{$type}{'ulp'}{$float}) {
	    print NEWULP "$float: ",
	    &clean_up_number ($results{$fct}{$type}{'ulp'}{$float}),
	    "\n";
	  }
	  if (exists $results{$fct}{$type}{'fail'}{$float}) {
	    print NEWULP "$float: fail\n";
	  }
	}
	print NEWULP "\n";
      }
    }
  }
  print NEWULP "# end of automatic generation\n";
  close NEWULP;
}

sub get_ulps {
  my ($test, $type, $float) = @_;

  if ($type eq 'complex') {
    my ($res);
    # Return 0 instead of BUILD_COMPLEX (0,0)
    if (!exists $results{$test}{'real'}{'ulp'}{$float} &&
	!exists $results{$test}{'imag'}{'ulp'}{$float}) {
      return "0";
    }
    $res = 'BUILD_COMPLEX (';
    $res .= (exists $results{$test}{'real'}{'ulp'}{$float}
	     ? $results{$test}{'real'}{'ulp'}{$float} : "0");
    $res .= ', ';
    $res .= (exists $results{$test}{'imag'}{'ulp'}{$float}
	     ? $results{$test}{'imag'}{'ulp'}{$float} : "0");
    $res .= ')';
    return $res;
  }
  return (exists $results{$test}{'normal'}{'ulp'}{$float}
	  ? $results{$test}{'normal'}{'ulp'}{$float} : "0");
}

sub get_failure {
  my ($test, $type, $float) = @_;
  if ($type eq 'complex') {
    # return x,y
    my ($res);
    # Return 0 instead of BUILD_COMPLEX_INT (0,0)
    if (!exists $results{$test}{'real'}{'ulp'}{$float} &&
	!exists $results{$test}{'imag'}{'ulp'}{$float}) {
      return "0";
    }
    $res = 'BUILD_COMPLEX_INT (';
    $res .= (exists $results{$test}{'real'}{'fail'}{$float}
	     ? $results{$test}{'real'}{'fail'}{$float} : "0");
    $res .= ', ';
    $res .= (exists $results{$test}{'imag'}{'fail'}{$float}
	     ? $results{$test}{'imag'}{'fail'}{$float} : "0");
    $res .= ')';
    return $res;
  }
  return (exists $results{$test}{'normal'}{'fail'}{$float}
	  ? $results{$test}{'normal'}{'fail'}{$float} : "0");

}

# Output the defines for a single test
sub output_test {
  my ($file, $test, $name) = @_;
  my ($ldouble, $double, $float, $ildouble, $idouble, $ifloat);
  my ($type);

  # Do we have ulps/failures?
  if (!exists $results{$test}{'type'}) {
    return;
  }
  $type = $results{$test}{'type'};
  if (exists $results{$test}{'has_ulps'}) {
    # XXX use all_floats (change order!)
    $ldouble = &get_ulps ($test, $type, "ldouble");
    $double = &get_ulps ($test, $type, "double");
    $float = &get_ulps ($test, $type, "float");
    $ildouble = &get_ulps ($test, $type, "ildouble");
    $idouble = &get_ulps ($test, $type, "idouble");
    $ifloat = &get_ulps ($test, $type, "ifloat");
    print $file "#define DELTA$name CHOOSE($ldouble, $double, $float, $ildouble, $idouble, $ifloat)\t/* $test  */\n";
  }

  if (exists $results{$test}{'has_fails'}) {
    $ldouble = &get_failure ($test, "ldouble");
    $double = &get_failure ($test, "double");
    $float = &get_failure ($test, "float");
    $ildouble = &get_failure ($test, "ildouble");
    $idouble = &get_failure ($test, "idouble");
    $ifloat = &get_failure ($test, "ifloat");
    print $file "#define FAIL$name CHOOSE($ldouble, $double, $float $ildouble, $idouble, $ifloat)\t/* $test  */\n";
  }
}

# Print include file
sub output_ulps {
  my ($file, $ulps_filename) = @_;
  my ($i, $fct);

  open ULP, ">$file" or die ("Can't open $file: $!");

  print ULP "/* This file is automatically generated\n";
  print ULP "   from $ulps_filename with gen-libm-test.pl.\n";
  print ULP "   Don't change it - change instead the master files.  */\n\n";

  print ULP "\n/* Maximal error of functions.  */\n";
  foreach $fct (@functions) {
    output_test (\*ULP, $fct, $fct);
  }

  print ULP "\n/* Error of single function calls.  */\n";
  for ($i = 0; $i < $count; $i++) {
    output_test (\*ULP, $tests[$i], $i);
  }
  close ULP;
}
