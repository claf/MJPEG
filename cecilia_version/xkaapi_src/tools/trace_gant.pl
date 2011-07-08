#!/usr/bin/env perl

use strict;
use warnings;

print <<HEAD;
\\documentclass[a4paper,11pt]{article}
\\usepackage[utf8x]{inputenc}
\\usepackage{tikz}
\\newcommand\\ganttline[4]{% line, tag, start end
\\node at (0,#1) [anchor=base east] {#2};
\\fill[blue] (#3/20,#1-.2) rectangle (#4/20,#1+.2);}
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
    print "\\ganttline{$frame}{$2}{$s}{$e}\n";
  }
}


print <<END;
\\end{tikzpicture}
\\end{center}
\\end{document}
END
