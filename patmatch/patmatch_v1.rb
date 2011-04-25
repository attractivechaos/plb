if ARGV.length >= 1
	# FIXME: Will precompiling the regexp be faster?
	STDIN.each do |line|
		puts line if line[0..-2] =~ /#{ARGV[0]}/
	end
else
	puts "Usage: ruby patmatch patter < in.file"
end

