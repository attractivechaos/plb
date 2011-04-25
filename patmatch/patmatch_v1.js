// I do not know if my implementation is buggy or v8 is buggy: it prints two extra lines.

// reference http://stackoverflow.com/questions/4508897/console-input-function-for-rhino
var readln = (typeof readline === 'function') ? (readline) : (function() {
     importPackage(java.io);
     importPackage(java.lang);
     var stdin = new BufferedReader(new InputStreamReader(System['in']));

     return function() {
         return String(stdin.readLine());  // Read line, 
     };                                    // force to JavaScript String
 }());
if (arguments[0]) {
	pat = new RegExp(arguments[0])
	while ((l = readln()) != null)
		if (pat.exec(l.slice(0, -1))) // for v8, exec() is slightly faster than match()
			print(l)
} else print("Usage: d8 patmatch.js -- pattern < in.file");

