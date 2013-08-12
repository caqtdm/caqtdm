#!/usr/bin/perl
# change the line above if perl is located
# somewhere else on your system

# we will process only PostScript files;
# $isps is true for PostScript only
$isps = 0;

# read first line
$_ = <STDIN>;
# PostScript files start with '%!'
$isps = 1 if (/^%!/);

print;

# read next line
$_ = <STDIN>;

# structured PostScript files have comments at the
# beginning that start with '%%' and end with
# a line '%%EndComments';
# we skip them
if ($isps) {
  while (/^%%/ && !/%%EndComments/) {
    print;
    $_ = <STDIN>;
  }
  print;

# put our commands now
  print << "EOF";
%%BeginSetup
2 dict dup /Duplex true put dup /Tumble true put setpagedevice
%%EndSetup
EOF
}
else {
  print;
}

# read and print to the end of the file
while (<STDIN>) { print; }
close(STDOUT);

exit 0;

