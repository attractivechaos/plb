while (<>) {
	chomp;
	print "$1\t$2\t$3\n" if /([a-zA-Z][a-zA-Z0-9]*):\/\/([^ \/]+)(\/[^ ]*)?/
}
