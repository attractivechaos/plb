die("Usage: patmatch.pl regexp in.file\n") if (@ARGV == 0);
my $re = shift(@ARGV);
while (<>) {
  print if /$re/o;
}
