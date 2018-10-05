// This program segfaults on simple input and complains "Error: std.format"
// given the "howto" file, even if I clean the text.

import std.stdio, std.regex;

int main(string[] args) {
	if (args.length < 2) {
		writefln("Usage: patmatch pattern < in.file");
		return 1;
	}
	
	auto re = regex(args[1]);
	foreach(line; stdin.byLine) {
		foreach(c; matchAll(line, re)) writeln(c.hit);
	}
	
	return 0;
}
