#!/usr/bin/env perl

while (<STDIN>)
{
  if (/Frame(\d+)\s(\d+)\s(\d+)\sfetch/)
  {
    my $frame = $1 - 1;
    print "Frame$frame\t$2\t$3\tfetch\n";
  } elsif (/Frame(\d+)\s(\d+)\s+fetch/) {
    my $frame = $1 - 1;
    print "Frame$frame\t$2\tfetch\n";
  } else {
       print $_;
  }
}
