/*
   zerodoku_v2.c
   Copyright 2008 Jim Schirle
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   For a copy of the GNU General Public License, see
   <http://www.gnu.org/licenses/>.
*/
#include <memory.h>     // memset
#include <stdio.h>      // printf, FILE, fopen, etc
#include <unistd.h>     // getopt

// modify BOX to be 2, 3, 4, or 5 for different size puzzles
#define BOX 3
#define UNIT (BOX * BOX)
#define GRID (UNIT * UNIT)
#define MASK ((1<<(UNIT+1))-2)
#define GET_POSSIBLE(x) ((~(p.r[s.r[x]]|p.c[s.c[x]]|p.b[s.b[x]])) & MASK)

// Try to bitcount with array, otherwise use function
#if (BOX < 4)
#define BITCOUNT(x) (bitCountArr[x])
#else
#define BITCOUNT(x) (bitCount(x))
#endif

// Stats to keep while solving
unsigned int puzlCnt = 0;
unsigned int solnCnt = 0;
unsigned int errCnt = 0;

// Options
unsigned int printSolutions = 1;
unsigned int maxSolutions = 1;

// The actual puzzle itself with conflicts for row/col/box
struct Puzzle {
   unsigned int board[GRID];
   unsigned int r[UNIT]; // conflicts
   unsigned int c[UNIT]; // conflicts
   unsigned int b[UNIT]; // conflicts
   int count;
}p;

// quick reference to groups for rows, cols and boxes
unsigned int g[UNIT*3][UNIT];

// possibilities for each square - derived from conflicts
unsigned int possib[GRID];

// used as a quick reference to access the proper conflicts in row/col/box
struct Offsets {
   unsigned int r[GRID];
   unsigned int c[GRID];
   unsigned int b[GRID];
}s;

// try a move in the bitboard - also sets bits in the conflict arrays
void set(int pos, unsigned int val) {
   p.board[pos] = val;
   p.r[s.r[pos]] |= val;
   p.c[s.c[pos]] |= val;
   p.b[s.b[pos]] |= val;
   p.count++;
}

// undo a move - this also clears bits in the conflict arrays
void clear(int pos) {
   int val = ~p.board[pos];
   p.board[pos] = 0;
   p.r[s.r[pos]] &= val;
   p.c[s.c[pos]] &= val;
   p.b[s.b[pos]] &= val;
   p.count--;
}

// bit count array - contains the count of 1 bits for each index in the array
int bitCountArr[1 << 10];

// bit count function for when bit count array is too small
int bitCount(unsigned int val) {
   val -= (val >> 1) & 0x55555555;
   val = (val & 0x33333333) + ((val >> 2) & 0x33333333);
   val = (val + (val >> 4)) & 0x0f0f0f0f;
   val += val >> 8;
   val += val >> 16;
   return val & 0x1f;
}

// outputs the puzzle/solution to screen as a single line
void printPuzzle() {
   int i;
   for (i = 0; i < GRID; i++) {
      int val = p.board[i] == 0 ? 0 : BITCOUNT((p.board[i] & -p.board[i]) - 1);
      putchar(val < 10 ? val + '0' : val + 'a' - 10);
   }
   putchar('\n');
}

// Search the board to find naked and hidden singles (or zeros)
int findSingles(int* pos, int* val) {
   int savePos = -1;
   int saveVal = -1;
   int saveCount = UNIT+1;
   int count;
   int i, j, x;
   for (i = 0; i < GRID; i++) {
      if (p.board[i] != 0) {
         possib[i] = 0;
         continue;
      }
      possib[i] = GET_POSSIBLE(i);
      count = BITCOUNT(possib[i]);
      if (count == 0) { // found naked zero - backtrack
         return 0;
      } else if (count == 1) { // found naked single
         *pos = i;
         *val = possib[i];
         return 1;
      } else if (count <= saveCount) { // find lowest number of pencil marks
         savePos = i;
         saveVal = possib[i];
         saveCount = count;
      }
   }
   // search for hidden zeros and singles
   for (i = 0; i < UNIT*3; i++) {
      if(p.r[i] == MASK)
         continue;
      int once = 0, twice = 0, all = 0;
      for (j = 0; j < UNIT; j++) {
         x = g[i][j];
         all |= (possib[x] | p.board[x]);
         twice |= (once & possib[x]);
         once |= possib[x];
      }
      if (all != MASK) // found hidden 0 - backtrack
         return 0;
      once &= ~twice;
      if (!once)
         continue;
      once &= -once;
      // found a hidden single
      for (j = 0; j < UNIT; j++) { // go back through row and get the single
         if (possib[*pos = g[i][j]] & once) {
            *val = once;
            return 1;
         }
      }
   }
   // give up and return the lowest number of pencil marks in a cell
   *pos = savePos;
   *val = saveVal;
   return (saveCount > UNIT) ? 0 : saveCount;
}

// iterate over the puzzle and find solution(s)
int backTrackSolve() {
   if (p.count == GRID) { // found a solution
      if (printSolutions)
         printPuzzle();
      return 1;
   }
   int i;
   int solnsFound = 0;
   int x = -1;
   int val = -1;
   int count = findSingles(&x, &val);
   if (count == 0) // backtrack
      return 0;
   else if (count == 1) { // found a single, so set it and recurse
      set(x, val);
      solnsFound = backTrackSolve();
      clear(x);
      return solnsFound;
   }
   // iterate through the possible moves in this square
   for (i = (val & -val); val; i = (val & -val)) {
      set(x, i);                        // try this move
      solnsFound += backTrackSolve();   // recurse
      if (solnsFound >= maxSolutions)
         return solnsFound;
      clear(x);                         // undo the move
      val &= ~i;
   }
   return solnsFound;
}

int main(int argc, char** argv) {
   char c;
   int i, opt;
   // process options
   while ((opt = getopt(argc, argv, "qs:")) != -1) {
      switch (opt) {
      case 'q':  // quiet!
         printSolutions = 0;
         break;
      case 's':  // max solutions
         maxSolutions = atoi(optarg);
         break;
      default: // '?'
         fprintf(stderr, "Usage: %s [-q] [-s solns]\n", argv[0]);
         fprintf(stderr, "\t-q        quiet - only prints stats\n");
         fprintf(stderr, "\t-s solns  changes max solutions (default=1)\n");
         return 1;
      }
   }
   // initialize lookup tables
   for (i = 0; i < GRID; i++) {
      int x = i / UNIT, y = i % UNIT;
      s.r[i] = x;
      s.c[i] = y;
      s.b[i] = x / BOX * BOX + y / BOX;
      g[x][y] = i;
      g[y+UNIT][x] = i;
      g[x+UNIT*2][y] = ((x/BOX*BOX)+(y/BOX))*UNIT+(x%BOX*BOX)+(y%BOX);
   }
   // initialize bitCounts
   for (i = 0; i < (1 << 10); i++)
      bitCountArr[i] = bitCount(i);

   // process file (or stdin)
   FILE* fp = stdin;
   if (optind < argc && (fp = fopen(argv[optind], "r")) == NULL) {
      fprintf(stderr, "%s: can't open %s\n", argv[0], argv[optind]);
      return 2;
   }
   do {
      i = 0;
      memset(&p, 0, sizeof(p)); // clear for new puzzle
      while ((c = tolower(getc(fp))) != EOF && i < GRID) {
         if (c > '0' && c <= '9')
            set(i++, 1<<(c - '0'));
         else if (c >= 'a' && c <= 'z')
            set(i++, 1<<(c - 'a' + 10));
         else if (c == '0' || c == '.' || c == '_')
            i++;
      }
      if (i < GRID)
         continue;
      puzlCnt++;
      int solns = backTrackSolve();
      if (solns == 0) {
         errCnt++;
         if (printSolutions)
            printPuzzle();
      }
      solnCnt += solns;
   } while (c != EOF);
   printf("%d puzzles w/ %d solutions\n", puzlCnt, solnCnt);
   if (errCnt != 0)
      printf("Errors: %d\n", errCnt);
   return 0;
}
