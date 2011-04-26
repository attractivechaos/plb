my %h;
my $max = 0;
while (<>) {
	my $p = \$h{$_};
	if ($$p) { ++$$p; }
	else { $$p = 1; }
	$max = $$p if ($$p > $max);
}
print scalar(keys %h), "\t$max\n";
