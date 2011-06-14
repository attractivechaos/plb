function sd_enumerate9() {
	var pos2box = [], mask_box = [], mask_col = [];
	var i, j, c, p = [], e = [];
	for (i = 0; i < 81; ++i) pos2box[i] = Math.floor(i/27)*3 + Math.floor(i%9/3); // translate position to the box index
	for (i = 0; i < 9; ++i) mask_box[i] = mask_col[i] = 0, p[i] = -1; // initialization
	for (i = c = 0;; ++c) { // each iteration generates a valid pattern
		while (i >= 0 && i < 9) { // loop until found one pattern or found no pattern
			var i9 = 9 * i;
			if (p[i] >= 0) mask_col[p[i]] = mask_box[pos2box[i9+p[i]]] = 0; // reset the mask at row i
			for (j = p[i] + 1; j < 9; ++j) // search for a valid position
				if (!mask_box[pos2box[i9 + j]] && !mask_col[j]) break; // stop if found
			if (j < 9) mask_col[j] = mask_box[pos2box[9*i+j]] = 1, p[i++] = j; // found a valid one
			else p[i--] = -1; // backtrack
		}
		if (i-- < 0) break; // no valid patterns any more
		e[c] = []
		for (j = 0; j < 9; ++j) { // write the pattern
			var x = 9 * j + p[j];
			e[c][x>>5] |= 1<<(x&31)
		}
	}
	return e;
}

function sd_solve(e, s) {
	var i, j, k, p = [], n = [], x, y, z = [], ret = [];
	for (i = 0; i < 9; ++i) { // generate all possible patterns for each number
		z[i] = []; p[i] = -3;
		x = [0, 0, 0]; y = [0, 0, 0];
		x.a = x.b = y.a = y.b = 0; // x keeps the pattern of the current number; y the other numbers
		for (j = 0; j < 81; ++j) { // set x and y
			if (s.charAt(j) < '1' || s.charAt(j) > '9') continue; // skip if not [1,9]
			if (s.charCodeAt(j) - 49 == i) x[j>>5] |= 1<<(j&31); // if the current number, set x
			else y[j>>5] |= 1<<(j&31); // if other numbers, set y
		}
		for (j = 0; j < 46656; ++j) { // traverse all possible patterns
			var ej = e[j];
			if ((ej[0]&x[0]) == x[0] && (ej[1]&x[1]) == x[1] && (ej[2]&x[2]) == x[2]
				&& !(ej[0]&y[0]) && !(ej[1]&y[1]) && !(ej[2]&y[2]))
			{
				z[i].push(ej[0]); z[i].push(ej[1]); z[i].push(ej[2]);
			}
		}
		n[i] = (z[i].length/3)<<8 | i;
	}
	n.sort(function(a,b){return a - b});
	for (i = 0; i < 9; ++i) n[i] &= 0xff;
	x = [0, 0, 0]
	for (i = 0;;) {
		while (i >= 0 && i < 9) {
			var zi = z[n[i]]; // n[i]&0xff is the number
			if (p[i] >= 0) x[0] &= ~zi[p[i]], x[1] &= ~zi[p[i]+1], x[2] &= ~zi[p[i]+2];
			for (j = p[i] + 3; j < zi.length; j += 3) // search for a compatible pattern
				if (!(x[0]&zi[j]) && !(x[1]&zi[j+1]) && !(x[2]&zi[j+2])) break;
			if (j < zi.length) x[0] |= zi[j], x[1] |= zi[j+1], x[2] |= zi[j+2], p[i++] = j; // found; move to the next
			else p[i--] = -3; // backtrack
		}
		if (i-- < 0) break; // no further solutions
		var y = []
		for (j = 0; j < 9; ++j) {
			for (k = 0; k < 32; ++k) if (z[n[j]][p[j]+0]>>k&1) y[k+0]  = n[j] + 1
			for (k = 0; k < 32; ++k) if (z[n[j]][p[j]+1]>>k&1) y[k+32] = n[j] + 1
			for (k = 0; k < 17; ++k) if (z[n[j]][p[j]+2]>>k&1) y[k+64] = n[j] + 1
		}
		ret.push(y)
	}
	return ret
}

var l, e = sd_enumerate9()
while ((l = readline()) != null) {
	if (l.length >= 81) {
		var r = sd_solve(e, l)
		for (var i = 0; i < r.length; ++i) print(r[i].join(''))
		print()
	}
}
