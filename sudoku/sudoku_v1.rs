struct Sudoku {
	r: [[int, ..9], ..324],
	c: [[int, ..4], ..729]
}

pub impl Sudoku {
	pub fn new() -> Sudoku {
		let mut s = Sudoku { r: [[0, ..9], ..324], c: [[0, ..4], ..729] };
		let mut nr = [0, ..324], r = 0;
		for int::range(0, 9) |i| {
			for int::range(0, 9) |j| {
				for int::range(0, 9) |k| {
					s.c[r][0] = 9 * i + j;
					s.c[r][1] = (i/3*3 + j/3) * 9 + k + 81;
					s.c[r][2] = 9 * i + k + 162;
					s.c[r][3] = 9 * j + k + 243;
					r += 1;
				}
			}
		}
		for int::range(0, 729) |r| {
			for int::range(0, 4) |c2| {
				let k = s.c[r][c2];
				s.r[k][nr[k]] = r;
				nr[k] += 1;
			}
		}
		return s;
	}
	fn update(&self, sr: &mut [int], sc: &mut [int], r: int, v: int) -> int {
		let mut min = 10, min_c = 0;
		for int::range(0, 4) |c2| {
			sc[self.c[r][c2]] += v<<7;
		}
		for int::range(0, 4) |c2| {
			let c = self.c[r][c2];
			if v > 0 { // move forward
				for int::range(0, 9) |r2| {
					let rr = self.r[c][r2];
					sr[rr] += 1;
					if sr[rr] == 1 {
						for int::range(0, 4) |cc2| {
							let cc = self.c[rr][cc2];
							sc[cc] -= 1;
							if (sc[cc] < min) {
								min = sc[cc]; min_c = cc;
							}
						}
					}
				}
			} else {
				for int::range(0, 9) |r2| {
					let rr = self.r[c][r2];
					sr[rr] -= 1;
					if sr[rr] == 0 {
						let p = self.c[rr];
						sc[p[0]] += 1; sc[p[1]] += 1; sc[p[2]] += 1; sc[p[3]] += 1;
					}
				}
			}
		}
		return min<<16 | min_c;
	}
	pub fn solve(&self, inp: &str) -> ~[~str] {
		let mut sc = ~[9, ..324], sr = ~[0, ..729];
		let mut cr = [-1, ..81], cc = [-1, ..81];
		let mut s = [0, ..81], s8 = [48u8, ..81];
		let mut hints = 0;
		for int::range(0, 81) |i| {
			let c = inp[i] as char;
			s[i] = -1;
			if c >= '1' && c <= '9' {
				s[i] = (c - '1') as int;
				self.update(sr, sc, i * 9 + s[i], 1);
				hints += 1;
				s8[i] = c as u8;
			}
		}

		let mut ret: ~[~str] = ~[];
		let mut i = 0, dir = 1, cand = 10<<16|0;
		loop {
			while i >= 0 && i < 81 - hints {
				if dir == 1 {
					let mut min = cand>>16;
					cc[i] = cand & 0xffff;
					if min > 1 {
						for int::range(0, 324) |c| {
							if sc[c] < min {
								min = sc[c]; cc[i] = c;
								if min <= 1 { break; }
							}
						}
					}
					if min == 0 || min == 10 {
						cr[i] = -1; dir = -1; i -= 1;
					}
				}
				let c = cc[i];
				if dir == -1 && cr[i] >= 0 {
					self.update(sr, sc, self.r[c][cr[i]], -1);
				}
				let mut tmp = 9;
				for int::range(cr[i] + 1, 9) |r2| {
					if sr[self.r[c][r2]] == 0 {
						tmp = r2;
						break;
					}
				}
				if tmp < 9 {
					cand = self.update(sr, sc, self.r[c][tmp], 1);
					cr[i] = tmp; dir = 1; i += 1;
				} else {
					cr[i] = -1; dir = -1; i -= 1;
				}
			}
			if i < 0 { break; }
			for int::range(0, i) |j| {
				let r = self.r[cc[j]][cr[j]];
				s8[r/9] = (r%9 + '1' as int) as u8;
			}
			ret.push(str::from_bytes(s8));
			i -= 1; dir = -1;
		}
		return ret;
	}
}

fn main() {
	let s = Sudoku::new();
	for io::stdin().each_line |line| {
		let r = s.solve(line);
		for vec::each(r) |l| {
			io::println(*l);
		}
		io::println("");
	}
}
