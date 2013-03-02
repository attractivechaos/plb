die("Usage: patmatch.pl regexp in.file\n") if (@ARGV == 0);
my $re = shift(@ARGV);
while (<>) {
	chomp;
	print $_, "\n" if /$re/o;
}
