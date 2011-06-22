die("Usage: patmatch.pl regexp in.file\n") if (@ARGV == 0);

my $re = qr/$ARGV[0]/;

while (<>) {
  print if /$re/;
}