// I do not know if my implementation is buggy or javascript is buggy.
// For v8, it prints two extra lines
if (arguments[0]) {
	pat = new RegExp(arguments[0])
	while ((l = readline()) != null)
		if (pat.exec(l.slice(0, -1))) // for v8, exec() is slightly faster than match()
			print(l)
} else print("Usage: d8 patmatch.js -- pattern < in.file");

