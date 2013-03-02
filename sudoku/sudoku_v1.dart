import 'dart:core';
import 'dart:io';

class Sudoku {
	var _R, _C;
	Sudoku() {
		_R = new List(324);
		_C = new List(729);
		for (int i = 0, r = 0; i < 9; ++i)
			for (int j = 0; j < 9; ++j)
				for (int k = 0; k < 9; ++k)
					_C[r++] = <int>[ 9 * i + j, (i~/3*3 + j~/3) * 9 + k + 81, 9 * i + k + 162, 9 * j + k + 243 ];
		for (int c = 0; c < 324; ++c) _R[c] = new List<int>();
		for (int r = 0; r < 729; ++r)
			for (int c2 = 0; c2 < 4; ++c2)
				_R[_C[r][c2]].add(r);
	}
	_update(sr, sc, r, v) {
		int min = 10, min_c = 0;
		for (int c2 = 0; c2 < 4; ++c2) sc[_C[r][c2]] += v<<7;
		for (int c2 = 0; c2 < 4; ++c2) {
			int r2, rr, cc2, c = _C[r][c2];
			if (v > 0) {
				for (r2 = 0; r2 < 9; ++r2) {
					if (sr[rr = _R[c][r2]]++ != 0) continue;
					for (cc2 = 0; cc2 < 4; ++cc2) {
						var cc = _C[rr][cc2];
						if (--sc[cc] < min) { min = sc[cc]; min_c = cc; }
					}
				}
			} else { // revert
				for (r2 = 0; r2 < 9; ++r2) {
					if (--sr[rr = _R[c][r2]] != 0) continue;
					var p = _C[rr];
					++sc[p[0]]; ++sc[p[1]]; ++sc[p[2]]; ++sc[p[3]];
				}
			}
		}
		return min<<16 | min_c; // return the col that has been modified and with the minimal available choices
	}
	solve(s_) {
		int j, r, c, r2, min, hints = 0;
		var sr = new List<int>(729), sc = new List<int>(324);
		var cr = new List<int>(81), cc = new List<int>(81), out = new List<int>(81), ret = [];
		for (r = 0; r < 729; ++r) sr[r] = 0;
		for (c = 0; c < 324; ++c) sc[c] = 9;
		for (int i = 0; i < 81; ++i) {
			int a = s_.charCodeAt(i) >= 49 && s_.charCodeAt(i) <= 57? s_.charCodeAt(i) - 49 : -1;
			if (a >= 0) _update(sr, sc, i * 9 + a, 1);
			if (a >= 0) ++hints;
			cr[i] = cc[i] = -1; out[i] = a + 1;
		}
		for (int i = 0, dir = 1, cand = 10<<16|0;;) {
			while (i >= 0 && i < 81 - hints) {
				if (dir == 1) {
					min = cand>>16; cc[i] = cand&0xffff;
					if (min > 1) {
						for (c = 0; c < 324; ++c) {
							if (sc[c] < min) {
								min = sc[c]; cc[i] = c;
								if (min <= 1) break;
							}
						}
					}
					if (min == 0 || min == 10) cr[i--] = dir = -1;
				}
				c = cc[i];
				if (dir == -1 && cr[i] >= 0) _update(sr, sc, _R[c][cr[i]], -1);
				for (r2 = cr[i] + 1; r2 < 9; ++r2)
					if (sr[_R[c][r2]] == 0) break;
				if (r2 < 9) {
					cand = _update(sr, sc, _R[c][r2], 1);
					cr[i++] = r2; dir = 1;
				} else cr[i--] = dir = -1;
			}
			if (i < 0) break;
			var y = new List<int>(81);
			for (j = 0; j < 81; ++j) y[j] = out[j];
			for (j = 0; j < i; ++j) { r = _R[cc[j]][cr[j]]; y[r~/9] = r%9 + 1; }
			ret.add(y);
			--i; dir = -1;
		}
		return ret;
	}
}

main()
{
	List<String> argv = new Options().arguments;
	var fp = new File(argv[0]);
	var lines = fp.readAsLinesSync(Encoding.ASCII);
	Sudoku s = new Sudoku();
	for (int i = 0; i < lines.length; ++i) {
		var ret = s.solve(lines[i]);
		print(ret[0]);
	};
}
