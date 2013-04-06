// xfail-pretty

// Copyright 2012 The Rust Project Developers. See the COPYRIGHT
// file at the top-level directory of this distribution and at
// http://rust-lang.org/COPYRIGHT.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

extern mod std;

use core::io::{ReaderUtil, WriterUtil};
use core::io;
use core::unstable::intrinsics::cttz16;

// internal type of sudoku grids
type grid = ~[~[u8]];

struct Sudoku {
    grid: grid
}

pub impl Sudoku {
    pub fn new(g: grid) -> Sudoku {
        return Sudoku { grid: g }
    }

    pub fn from_vec(vec: &[[u8, ..9], ..9]) -> Sudoku {
        let mut g = do vec::from_fn(9u) |i| {
            do vec::from_fn(9u) |j| { vec[i][j] }
        };
        return Sudoku::new(g)
    }

	pub fn from_str(inp: &str) -> Sudoku {
		let mut g: grid = ~[~[0u8, ..9], ..9];
		for int::range(0, 9) |row| {
			for int::range(0, 9) |col| {
				let i = row * 9 + col;
				let c = inp[i] as char;
				let mut a = 0u8;
				if c >= '1' && c <= '9' {
					a = (c - '0') as u8;
				}
				g[row][col] = a;
			}
		}
		return Sudoku::new(g);
	}

    pub fn write(&self, writer: @io::Writer) {
		let mut s8 = [0u8, ..81];
		for int::range(0, 9) |row| {
			for int::range(0, 9) |col| {
				s8[row*9+col] = self.grid[row][col] + '0' as u8;
			}
		}
		writer.write_str(str::from_bytes(s8) + "\n\n");
    }

    // solve sudoku grid
    pub fn solve(&mut self) {
        let mut work: ~[(u8, u8)] = ~[]; /* queue of uncolored fields */
        for u8::range(0u8, 9u8) |row| {
            for u8::range(0u8, 9u8) |col| {
                let color = self.grid[row][col];
                if color == 0u8 { work += ~[(row, col)]; }
            }
        }

        let mut ptr = 0u;
        let end = vec::len(work);
        while (ptr < end) {
            let (row, col) = work[ptr];
            // is there another color to try?
            if self.next_color(row, col, self.grid[row][col] + (1 as u8)) {
                //  yes: advance work list
                ptr = ptr + 1u;
            } else {
                // no: redo this field aft recoloring pred; unless there is none
                if ptr == 0u { fail!(~"No solution found for this sudoku"); }
                ptr = ptr - 1u;
            }
        }
    }

    fn next_color(&mut self, row: u8, col: u8, start_color: u8) -> bool {
        if start_color < 10u8 {
            // colors not yet used
            let mut avail = ~Colors::new(start_color);

            // drop colors already in use in neighbourhood
            self.drop_colors(avail, row, col);

            // find first remaining color that is available
            let next = avail.next();
            self.grid[row][col] = next;
            return 0u8 != next;
        }
        self.grid[row][col] = 0u8;
        return false;
    }

    // find colors available in neighbourhood of (row, col)
    fn drop_colors(&mut self, avail: &mut Colors, row: u8, col: u8) {
        for u8::range(0u8, 9u8) |idx| {
            avail.remove(self.grid[idx][col]); /* check same column fields */
            avail.remove(self.grid[row][idx]); /* check same row fields */
        }

        // check same block fields
        let row0 = (row / 3u8) * 3u8;
        let col0 = (col / 3u8) * 3u8;
        for u8::range(row0, row0 + 3u8) |alt_row| {
            for u8::range(col0, col0 + 3u8) |alt_col| { avail.remove(self.grid[alt_row][alt_col]); }
        }
    }
}

// Stores available colors as simple bitfield, bit 0 is always unset
struct Colors(u16);

static heads: u16 = (1u16 << 10) - 1; /* bits 9..0 */

impl Colors {
    fn new(start_color: u8) -> Colors {
        // Sets bits 9..start_color
        let tails = !0u16 << start_color;
        return Colors(heads & tails);
    }

    fn next(&self) -> u8 {
        let val = **self & heads;
        if (0u16 == val) {
            return 0u8;
        }
        else
        {
            return cttz16(val as i16) as u8;
        }
    }

    fn remove(&mut self, color: u8) {
        if color != 0u8 {
            let val  = **self;
            let mask = !(1u16 << color);
            *self    = Colors(val & mask);
        }
    }
}

fn main() {
	for io::stdin().each_line |line| {
		let mut s = Sudoku::from_str(line);
		s.solve();
		s.write(io::stdout());
	}
}

