#!/usr/bin/perl

# Copyright (C) 2016 Martin Thomas <mtdev@hamtam.de>
# LGPL version 2 or later.

use strict;
use warnings;
use IO::Dir;
use File::Find qw(find);
use Encode::Guess;

my ($dir, $directory, $f, $w, $tmp);
my (@files, @dirs, $file, $filename);
my $header;
my $files;
my $encoding;
my @copyright;
my @copyrightout;
my @uniqcpr;
my @output;
my $i;

$encoding = ":encoding(UTF-8)";
$encoding = "";

$directory="./";

$header  = "Format: http://www.debian.org/doc/packaging-manuals/copyright-format/1.0/\n";
$header .= "Upstream-Name: uclibc-ng\n";
$header .= 'Upstream-Contact: Waldemar Brodkorb <wbx@uclibc-ng.org>'."\n";
$header .= "Source: git://uclibc-ng.org/git/uclibc-ng\n\n";

# my $emailregex='\b[[:alnum:]._%+-]+@[[:alnum:].-]+.[[:alpha:]]{2,6}\b';

sub list_dirs {
  my @dirs = @_;
  my @files;
  find({ wanted => sub { push @files, $_ } , no_chdir => 1 }, @dirs);
  return @files;
}

@files=list_dirs($directory);

foreach $file (@files) {
  if ( -f $file ){
#     $encoding = guess_encoding($file);
    open(my $fh, "< $encoding", $file)
      or die "Could not open file '$file' $!";
    while (my $row = <$fh>) {
      chomp $row;
      if ($row =~ m/[Cc]opyright / )
      {
        $row =~ s/^[\s\/\*#!;.\"\\]*//; #remove leading
        $row =~ s/\s+$//;               #remove trailing
        push @copyright, { file => $file, raw => $row};
        last;
      }
    }
    close $fh
  }
}

#sort raw
@copyrightout = sort { $a->{raw} cmp $b->{raw} } @copyright;

$tmp="";
$i=-1;
foreach (@copyrightout) {
  if ( $tmp eq $_->{'raw'} )
  {
    $output[$i]{"files"} .= "\n"."       $_->{'file'}";
  }
  else
  {
    ++$i;
    $output[$i]{"header"} .= "Copyright: $_->{'raw'}\n";
    $output[$i]{"header"} .= "License: GNU Lesser General Public License 2.1\n";
    $output[$i]{"files"} .= "Files: ".$_->{'file'};
  }
  $tmp=$_->{'raw'};
}

print "$header";
$i=0;
foreach (@output) {
  print "$output[$i]->{'files'}\n";
  print "$output[$i]->{'header'}\n";
  ++$i;
}
