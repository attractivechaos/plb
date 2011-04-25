// This program segfaults on simple input and complains "Error: std.format"
// given the "howto" file, even if I clean the text.

import std.stdio, std.regexp;

int main(string[] args) {
	if (args.length < 2) {
		writefln("Usage: patmatch pattern < in.file");
		return 1;
	}
	char[] buf;
	auto re = RegExp(args[1]);
	while ((buf = readln()) != null)
		if (re.find(buf[0 .. buf.length-1]) > 0)
			writef(buf);
	return 0;
}
