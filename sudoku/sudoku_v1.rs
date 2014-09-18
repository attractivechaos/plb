#![feature(phase)]
#[phase(plugin, link)] extern crate log;

struct Sudoku {
	r: [[u16, ..9], ..324],
	c: [[u16, ..4], ..729]
}

impl Sudoku {
	pub fn new() -> Sudoku {
		let mut s = Sudoku { r: [[0, ..9], ..324], c: [[0, ..4], ..729] };
		let mut nr = [0, ..324];
		let mut r = 0;
		for i in range(0, 9) {
			for j in range(0, 9) {
				for k in range(0, 9) {
					s.c[r][0] = 9 * i + j;
					s.c[r][1] = (i/3*3 + j/3) * 9 + k + 81;
					s.c[r][2] = 9 * i + k + 162;
					s.c[r][3] = 9 * j + k + 243;
					r += 1;
				}
			}
		}
		for r in range(0, 729) {
			for c2 in range(0, 4) {
				let k = s.c[r][c2];
				s.r[k as uint][nr[k as uint]] = r as u16;
				nr[k as uint] += 1;
			}
		}
		return s;
	}
	fn forward(&self, sr: &mut [i8], sc: &mut [u8], c: uint, min: &mut u8, min_c: &mut u16) {
		for &rr in self.r[c].iter() {
			// Take a pointer to avoid repeated bounds checks
			let srrr = &mut sr[rr as uint];
			*srrr += 1;
			if *srrr == 1 {
				for &cc in self.c[rr as uint].iter() {
					// Take a pointer to avoid repeated bounds checks
					let sccc = &mut sc[cc as uint];
					*sccc -= 1;
					if *sccc < *min {
						*min = *sccc; *min_c = cc;
					}
				}
			}
		}
	}
	fn revert(&self, sr: &mut [i8], sc: &mut [u8], c: uint) {
		for &rr in self.r[c].iter() {
			// Take a pointer to avoid repeated bounds checks
			let srrr = &mut sr[rr as uint];
			*srrr -= 1;
			if *srrr == 0 {
				for &i in self.c[rr as uint].iter() {
					sc[i as uint] += 1;
				}
			}
		}
	}
	fn update(&self, sr: &mut [i8], sc: &mut [u8], r: uint, v: int) -> int {
		let mut min = 10;
		let mut min_c = 0;
		for &i in self.c[r].iter() {
			sc[i as uint] += (v<<7) as u8;
		}
		for &c in self.c[r].iter() {
			if v > 0 { // move forward
				self.forward(sr, sc, c as uint, &mut min, &mut min_c)
			} else {
				self.revert(sr, sc, c as uint)
			}
		}
		return (min as int)<<16 | min_c as int;
	}
	pub fn solve(&self, inp: &str) -> Vec<String> {
		let mut sc = [9u8, ..324];
		let mut sr = [0i8, ..729];
		let mut cr = [-1i8, ..81];
		let mut cc = [-1i16, ..81];
		let mut s = [0u, ..81];
		let mut s8 = [48u8, ..81];
		let mut hints = 0;
		for (i, c) in inp.chars().enumerate().take(81) {
			s[i] = -1;
			if c >= '1' && c <= '9' {
				s[i] = c as uint - '1' as uint;
				self.update(sr, sc, (i * 9 + s[i]), 1);
				hints += 1;
				s8[i] = c as u8;
			}
		}

		let mut ret: Vec<String> = Vec::new();
		let mut i = 0i;
		let mut dir = 1i;
		let mut cand: int = 10<<16|0;
		loop {
			while i >= 0 && i < 81 - hints {
				if dir == 1 {
					let mut min = (cand>>16) as u8;
					cc[i as uint] = (cand & 0xffff) as i16;
					if min > 1 {
						for (c, &v) in sc.iter().enumerate() {
							if v < min {
								min = v; cc[i as uint] = c as i16;
								if min <= 1 { break; }
							}
						}
					}
					if min == 0 || min == 10 {
						cr[i as uint] = -1; dir = -1; i -= 1;
						assert!(i >= 0);
					}
				}
				let c = cc[i as uint];
				if dir == -1 && cr[i as uint] >= 0 {
					self.update(sr, sc, self.r[c as uint][cr[i as uint] as uint] as uint, -1);
				}
				let mut tmp = 9i8;
				for r2 in range(cr[i as uint] + 1, 9) {
					if sr[self.r[c as uint][r2 as uint] as uint] == 0 {
						tmp = r2;
						break;
					}
				}
				if tmp < 9 {
					cand = self.update(sr, sc, self.r[c as uint][tmp as uint] as uint, 1);
					cr[i as uint] = tmp; dir = 1; i += 1;
				} else {
					cr[i as uint] = -1; dir = -1; i -= 1;
				}
			}
			if i < 0 { break; }
			for j in range(0, i as uint) {
				let r = self.r[cc[j] as uint][cr[j] as uint];
				s8[r as uint/9] = (r%9 + '1' as u16) as u8;
			}
			ret.push(String::from_utf8_lossy(s8).to_string());
			i -= 1; dir = -1;
		}
		return ret;
	}
}

fn main() {
	let s = Sudoku::new();
	for e_line in std::io::stdin().lines() {
		match e_line {
			Ok(line) => {
				let r = s.solve(line.as_slice());
				for l in r.iter() {
					std::io::println(l.as_slice());
				}
				std::io::println("");
			},
			Err(e) => {
				error!("error reading: {}", e);
				return;
			}
		}
	}
}
