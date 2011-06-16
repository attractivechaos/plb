function sd_genmat() {
	var i, j, r, c, c2, C = [], R = []
	for (i = r = 0; i < 9; ++i)
		for (j = 0; j < 9; ++j)
			for (k = 0; k < 9; ++k)
				C[r++] = [ 9 * i + j, (Math.floor(i/3)*3 + Math.floor(j/3)) * 9 + k + 81, 9 * i + k + 162, 9 * j + k + 243 ]
	for (c = 0; c < 324; ++c) R[c] = []
	for (r = 0; r < 729; ++r)
		for (c2 = 0; c2 < 4; ++c2)
			R[C[r][c2]].push(r)
	return [R, C];
}

function sd_update(R, C, sr, sc, r, v) {
	for (var c2 = 0; c2 < 4; ++c2) {
		var c = C[r][c2];
		sc[c] += v;
		for (var r2 = 0; r2 < 9; ++r2) sr[R[c][r2]] += v;
	}
}

function sd_solve(R, C, _s) {
	var i, j, r, c, r2, dir, c0, hints = 0;
	var sr = [], sc = [], cr = [], cc = [], out = [], ret = [];
	for (r = 0; r < 729; ++r) sr[r] = 0;
	for (c = 0; c < 324; ++c) sc[c] = 0;
	for (i = 0; i < 81; ++i) {
		var a = _s.charAt(i) >= '1' && _s.charAt(i) <= '9'? _s.charCodeAt(i) - 49 : -1;
		if (a >= 0) sd_update(R, C, sr, sc, i * 9 + a, 1);
		if (a >= 0) ++hints;
		cr[i] = cc[i] = -1, out[i] = a + 1;
	}
	for (i = c0 = 0, dir = 1;;) {
		while (i >= 0 && i < 81 - hints) {
			if (dir == 1) {
				var min = 10, n;
				for (var j = 0; j < 324; ++j) {
					var p;
					c = j + c0 < 324? j + c0 : j + c0 - 324
					if (sc[c]) continue;
					for (r2 = n = 0, p = R[c]; r2 < 9; ++r2)
						if (sr[p[r2]] == 0) ++n;
					if (n < min) min = n, cc[i] = c, c0 = c + 1;
					if (min <= 1) break;
				}
				if (min == 0 || min == 10) cr[i--] = dir = -1;
			}
			c = cc[i];
			if (dir == -1 && cr[i] >= 0) sd_update(R, C, sr, sc, R[c][cr[i]], -1);
			for (r2 = cr[i] + 1; r2 < 9; ++r2)
				if (sr[R[c][r2]] == 0) break;
			if (r2 < 9) {
				sd_update(R, C, sr, sc, R[c][r2], 1);
				cr[i++] = r2; dir = 1;
			} else cr[i--] = dir = -1;
		}
		if (i < 0) break;
		var y = []
		for (j = 0; j < 81; ++j) y[j] = out[j]
		for (j = 0; j < i; ++j) r = R[cc[j]][cr[j]], y[Math.floor(r/9)] = r%9 + 1;
		ret.push(y)
		--i; dir = -1;
	}
	return ret;
}

var l, e = sd_genmat()
while ((l = readline()) != null) {
	if (l.length >= 81) {
		var r = sd_solve(e[0], e[1], l)
		for (var i = 0; i < r.length; ++i) print(r[i].join(''))
		print()
	}
}
