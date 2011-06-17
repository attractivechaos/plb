# Changed command-line interface  -ac

# sudoku.py - Dancing links sudoku solver
# Copyright (c) 2006,2008 Antti Ajanki <antti.ajanki@iki.fi>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

import re
import sys
import time
import string
import cStringIO
import math
import dlx
from optparse import OptionParser

class SudokuError(Exception):
    pass

class Sudoku:
    def __init__(self):
        # Supported block sizes
        self.sizes = {4: [2, 2], 6: [2, 3], 8: [2, 4], 9: [3, 3],
                      10: [2,5], 12:[3, 4], 15: [3, 5], 16: [4, 4],
                      20: [4, 5], 24: [4, 6], 25: [5, 5], 28: [4, 7],
                      30: [5, 6], 32: [4, 8], 35: [5, 7], 36: [6, 6]}
        # Translation table used while transforming sudoku string to
        # integers
        self.chartable = string.maketrans(
            '.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ&',
            '\x00' + ''.join(map(chr, xrange(0,37))))
        # Characters for printing out sudoku board
        self.sudokuchars = ['.', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
                            'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                            'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                            'W', 'X', 'Y', 'Z', '&']
        # Sudoku board is saved in self.sarray in row major order
        self.sarray = [0]*81
        self.blockrows = 3
        self.blockcols = 3

    def __str__(self):
        """Returns string presentation of the sudoku board."""
        r = self.blockrows
        c = self.blockcols
        ret = cStringIO.StringIO()
        for i, x in enumerate(self.sarray):
            ret.write(self.sudokuchars[x])

            if (i+1) == len(self.sarray):
                pass
            elif (i+1) % (c*r*r) == 0:
                ret.write('\n' + '+'.join(['-'*c]*r) + '\n')
            elif (i+1) % (c*r) == 0:
                ret.write('\n')
            elif (i+1) % c == 0:
                ret.write('|')
        return ret.getvalue()

    def load(self, s):
        """Reads a sudoku puzzle from string s."""
        lines = s.strip('\n').split('\n')

        # Is this one long line or several short ones?
        if len(lines) == 1:
            sudokustring = lines[0]
            boardsize = int(math.sqrt(len(sudokustring)))
            if boardsize*boardsize != len(sudokustring):
                raise SudokuError('Unsupported sudoku string length')
        else:
            # Heuristics to find puzzle size:
            # - Leave out everything execpt characters [.0-9A-Z&]
            # - Find most frequent line length
            # - Leave out shorter or longer lines
            # - Is the puzzle a square shaped?
            # - Is the size one of the known sizes in self.size?
            # - Map characters to integers in range 0-36
            # - Check that givens are on the right range
            sudokulines = []
            hist = {}
            for line in lines:
                line = re.sub(r'[^.0-9A-Z&]', '', line)
                if len(line) == 0: continue
                sudokulines.append(line)
                hist[len(line)] = hist.get(len(line), 0)
            boardsize = max(hist)

            sudokulines = [x for x in sudokulines if len(x) == boardsize]
            if len(sudokulines) != boardsize:
                raise SudokuError('Given sudoku doesn\'t seem to be square (width: %d, height: %d)!' \
                                  % (boardsize, len(sudokulines)))
            sudokustring = ''.join(sudokulines)

        try:
            self.blockrows, self.blockcols = self.sizes[boardsize]
        except KeyError:
            raise SudokuError('Unsupported sudoku width: %d' % boardsize)

        # Translate sudokustring to integer array
        self.sarray = map(ord, string.translate(sudokustring, self.chartable))
        # Check that givens are consistent with puzzle size
        maxgiven = max(self.sarray)
        if maxgiven > boardsize:
            raise SudokuError('Too large given (%s) for board size %dx%d!' \
                              % (self.sudokuchars[maxgiven], boardsize, boardsize))

    def solve(self, all):
        """Solves sudoku puzzle previously loaded by load()."""
        br = self.blockrows
        bc = self.blockcols
        puzzlerows = br*bc         # number of rows in the puzzle
        puzzlecols = puzzlerows    # number of columns in the puzzle
        digits = puzzlerows        # max digit
        bl = puzzlerows*puzzlecols # board length

        # Construct full DLX matrix and list of rows corresponding to
        # givens
        givenrows = []
        mydlx = []
        dlxones = []
        for r in xrange(0, puzzlerows):
            for c in xrange(0, puzzlecols):
                for d in xrange(0, digits):
                    dlxrow = bl*r + puzzlerows*c + d + 1

                    if self.sarray[puzzlecols*r+c]-1 == d:
                        givenrows.append(dlxrow)

                    # Calculate the number of the box. Boxes are
                    # numbered from left to right and from top to
                    # bottom.
                    box = br*(r/br)+c/bc
                    dlxones.append([dlxrow,
                                    [puzzlecols*r+c+1,      # <r,c> constrain
                                     bl+digits*r+d+1,       # <d,r>
                                     2*bl+digits*c+d+1,     # <d,c>
                                     3*bl+digits*box+d+1]]) # <d,b>
        dlxsolver = dlx.DLXMatrix()
        self.nsolutions = 0
        if all:
            dlxsolver.solve(dlxones, givenrows, self.printsolution)
        else:
            solution = dlxsolver.solve(dlxones, givenrows)
            if solution is not None:
                self.printsolution(solution)

    def printsolution(self, solution):
        br = self.blockrows
        bc = self.blockcols
        puzzlerows = br*bc         # number of rows in the puzzle
        puzzlecols = puzzlerows    # number of columns in the puzzle
        digits = puzzlerows        # max digit
        bl = puzzlerows*puzzlecols # board length

        self.nsolutions += 1
        # turn solution into sudoku board
        for s in solution:
            d = (s-1) % digits + 1
            c = ((s-1)/digits) % puzzlecols
            r = (s-1)/bl
            self.sarray[puzzlecols*r+c] = d
        print 'Solution:\n'
        print str(self)
        print ''

def main():
	allSolution = True
	sdk = Sudoku()
	for line in sys.stdin:
		sdk.load(line)
		sdk.solve(allSolution)

if __name__ == '__main__':
    main()
