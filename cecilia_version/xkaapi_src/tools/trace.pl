#!/usr/bin/env perl

use strict;
use warnings;

print <<HEAD;
\\documentclass[a4paper,11pt]{article}
\\usepackage[utf8x]{inputenc}
\\usepackage{tikz}
\\begin{document}
\\begin{center}
\\begin{tikzpicture}[yscale=0.2]
HEAD

while(<STDIN>) {
  if (/Frame\s+(\d+)\s+(\S+)\s+start\s+:\s+(\d+)\s+end\s+:\s+(\d+)\s+duration/) {
    my $frame = $1;
    my $color;
    if ($2 eq 'resized!') {
      $color = 'red';  
    } else {
      $color = 'blue';
    }
    my $line = $frame +1;
    my $s = $3 / 200;
    my $e = $4 / 200;
    print "\\draw [fill=$color!60,draw=$color!30] ($s, $frame) rectangle ($e,$line);\n";
  }
}


print <<END;
\\end{tikzpicture}
\\end{center}
\\end{document}
END
