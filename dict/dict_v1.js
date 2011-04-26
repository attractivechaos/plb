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

var h = {}, n = 0, max = 0;
while ((l = readln()) != null) {
	if (h[l]) {
		++h[l];
		if (max < h[l]) max = h[l];
	} else h[l] = 1, ++n;
}
print(n, max)
