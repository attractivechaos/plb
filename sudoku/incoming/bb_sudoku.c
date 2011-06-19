/****************************************************************\ 
**  BB_Sudoku  Bit Based Sudoku Solver                          ** 
\****************************************************************/ 

/****************************************************************\ 
**  (c) Copyright Brian Turner, 2008-2009.  This file may be    ** 
**      freely used, modified, and copied for personal,         ** 
**      educational, or non-commercial purposes provided this   ** 
**      notice remains attached.                                ** 
\****************************************************************/ 

/****************************************************************\ 
** The solver source code has now been broken into 3 files:     ** 
**   - bb_sudoku.cpp         : Driver code which handles things ** 
**                               like decoding arguments,       ** 
**                               loading puzzles, timings,      ** 
**                               and outputs.                   ** 
**   - bb_sudoku_solver.cpp  : The actual solver code           ** 
**   - bb_sudoku_tables.h    : Various tables used for solving  ** 
**   - random.h              : Various Random Number Generators ** 
**                                                              ** 
** I do use a nonstandard method for including the solver code, ** 
**   but it makes compiling from the command line easier and    ** 
**   does not need a makefile or anything.                      ** 
**                                                              ** 
**                                                              ** 
** Compiling:                                                   ** 
**                                                              ** 
**   This code has been compiled with both Visual C (Windows),  ** 
**   and gcc (Linux).  The following shows options used:        ** 
**                                                              ** 
**   Visual C (tested under Windows XP)                         ** 
**     Build under a Win32 Console Application, turn off the    ** 
**     precomiled headers, and use the Release configuration    ** 
**     for full optimizations.                                  ** 
**     NOTE: Under XP, there is a Windows bug when using the    ** 
**           timer with AMDs that can cause negative times.     ** 
**                                                              ** 
**   GCC (tested under Ubuntu 8.10)                             ** 
**     Use the following command line to compile the code:      ** 
**                                                              ** 
**       gcc -O3 -lrt -xc -obb_sudoku.out bb_sudoku.cpp         ** 
**                                                              ** 
**         -O3  = preform full optimizations                    ** 
**         -lrt = needed for linking of timing routines         ** 
**         -xc  = compile as a C program                        ** 
**         -oBB_Sudoku.out = name of the output file            ** 
**         BB_Sudoku.cpp   = name of file to compile            ** 
**                                                              ** 
\****************************************************************/ 

#define CALC_STATSx 

int bb_solver (unsigned *puzzle); 
void bb_init(void); 

/* 
// masks for methodologies to use 
#define USE_GUESSES     0x01  // use backtracking and guesses 
#define USE_LOCK_CAND   0x02  // use locked candidates 
#define USE_SUBSETS     0x04  // use subset searching 
#define USE_FISHIES     0x08  // use fishies (x-wing, swordfish, jellyfish) 
#define USE_1_STEP      0x10  // use one step commonalitytest 
#define USE_2_STEP      0x20  // use two step commonality test 

// masks for variations 
#define GT_LT_PUZZLE   0x100  // Greater Than and Less Than puzzle 
#define KILLER_PUZZLE  0x200  // Killer Sudoku puzzle 
*/ 

// V2B and B2V stand for "Binary to Value" and "Value to 
//   Binary", and provide a quick lookup between bit positions 
//   and values.  This provides an easy method to convert 
//   between values and bit positions for single values. 
// 
// NSet looks up the number of bits set (potential values) 
//   for numbers 0 to 511 (9 bits).  NSetX provides a lookup 
//   to the xth bit in the value 

// Values to Bit positions lookup 
static const unsigned int V2B[33] = {0x0000, 0x00000001, 0x00000002, 0x00000004, 0x00000008, 
                                0x00000010, 0x00000020, 0x00000040, 0x00000080, 
                                0x00000100, 0x00000200, 0x00000400, 0x00000800, 
                                0x00001000, 0x00002000, 0x00004000, 0x00008000, 
                                0x00010000, 0x00020000, 0x00040000, 0x00080000, 
                                0x00100000, 0x00200000, 0x00400000, 0x00800000, 
                                0x01000000, 0x02000000, 0x04000000, 0x08000000, 
                                0x10000000, 0x20000000, 0x40000000, 0x80000000}; 

// Bits to Value lookup 
static int B2V[512]; 

// Number of bits set in the number, up to 9 bits, along with 
//   a lookup for getting the xth bit of the value 
static int NSet[512], NSetX[512][10]; 


// Groups and P2Group define the groups for a standard 
//   3x3 grid and a look from a position (0-80) to the 
//   three groups it is a part of. 

// In_Groups defines all the cells included in all the groups associated with 
//   with the index cell.  It includes 8 for the 8, 8 for the column, and 4 for 
//   the remaining cells in the box that are not in the row or column. 
static const unsigned char In_Groups[81][20] = 
  {{  1,  2,  3,  4,  5,  6,  7,  8,  9, 18, 27, 36, 45, 54, 63, 72, 10, 11, 19, 20 }, 
   {  0,  2,  3,  4,  5,  6,  7,  8, 10, 19, 28, 37, 46, 55, 64, 73,  9, 11, 18, 20 }, 
   {  0,  1,  3,  4,  5,  6,  7,  8, 11, 20, 29, 38, 47, 56, 65, 74,  9, 10, 18, 19 }, 
   {  0,  1,  2,  4,  5,  6,  7,  8, 12, 21, 30, 39, 48, 57, 66, 75, 13, 14, 22, 23 }, 
   {  0,  1,  2,  3,  5,  6,  7,  8, 13, 22, 31, 40, 49, 58, 67, 76, 12, 14, 21, 23 }, 
   {  0,  1,  2,  3,  4,  6,  7,  8, 14, 23, 32, 41, 50, 59, 68, 77, 12, 13, 21, 22 }, 
   {  0,  1,  2,  3,  4,  5,  7,  8, 15, 24, 33, 42, 51, 60, 69, 78, 16, 17, 25, 26 }, 
   {  0,  1,  2,  3,  4,  5,  6,  8, 16, 25, 34, 43, 52, 61, 70, 79, 15, 17, 24, 26 }, 
   {  0,  1,  2,  3,  4,  5,  6,  7, 17, 26, 35, 44, 53, 62, 71, 80, 15, 16, 24, 25 }, 
   { 10, 11, 12, 13, 14, 15, 16, 17,  0, 18, 27, 36, 45, 54, 63, 72,  1,  2, 19, 20 }, 
   {  9, 11, 12, 13, 14, 15, 16, 17,  1, 19, 28, 37, 46, 55, 64, 73,  0,  2, 18, 20 }, 
   {  9, 10, 12, 13, 14, 15, 16, 17,  2, 20, 29, 38, 47, 56, 65, 74,  0,  1, 18, 19 }, 
   {  9, 10, 11, 13, 14, 15, 16, 17,  3, 21, 30, 39, 48, 57, 66, 75,  4,  5, 22, 23 }, 
   {  9, 10, 11, 12, 14, 15, 16, 17,  4, 22, 31, 40, 49, 58, 67, 76,  3,  5, 21, 23 }, 
   {  9, 10, 11, 12, 13, 15, 16, 17,  5, 23, 32, 41, 50, 59, 68, 77,  3,  4, 21, 22 }, 
   {  9, 10, 11, 12, 13, 14, 16, 17,  6, 24, 33, 42, 51, 60, 69, 78,  7,  8, 25, 26 }, 
   {  9, 10, 11, 12, 13, 14, 15, 17,  7, 25, 34, 43, 52, 61, 70, 79,  6,  8, 24, 26 }, 
   {  9, 10, 11, 12, 13, 14, 15, 16,  8, 26, 35, 44, 53, 62, 71, 80,  6,  7, 24, 25 }, 
   { 19, 20, 21, 22, 23, 24, 25, 26,  0,  9, 27, 36, 45, 54, 63, 72,  1,  2, 10, 11 }, 
   { 18, 20, 21, 22, 23, 24, 25, 26,  1, 10, 28, 37, 46, 55, 64, 73,  0,  2,  9, 11 }, 
   { 18, 19, 21, 22, 23, 24, 25, 26,  2, 11, 29, 38, 47, 56, 65, 74,  0,  1,  9, 10 }, 
   { 18, 19, 20, 22, 23, 24, 25, 26,  3, 12, 30, 39, 48, 57, 66, 75,  4,  5, 13, 14 }, 
   { 18, 19, 20, 21, 23, 24, 25, 26,  4, 13, 31, 40, 49, 58, 67, 76,  3,  5, 12, 14 }, 
   { 18, 19, 20, 21, 22, 24, 25, 26,  5, 14, 32, 41, 50, 59, 68, 77,  3,  4, 12, 13 }, 
   { 18, 19, 20, 21, 22, 23, 25, 26,  6, 15, 33, 42, 51, 60, 69, 78,  7,  8, 16, 17 }, 
   { 18, 19, 20, 21, 22, 23, 24, 26,  7, 16, 34, 43, 52, 61, 70, 79,  6,  8, 15, 17 }, 
   { 18, 19, 20, 21, 22, 23, 24, 25,  8, 17, 35, 44, 53, 62, 71, 80,  6,  7, 15, 16 }, 
   { 28, 29, 30, 31, 32, 33, 34, 35,  0,  9, 18, 36, 45, 54, 63, 72, 37, 38, 46, 47 }, 
   { 27, 29, 30, 31, 32, 33, 34, 35,  1, 10, 19, 37, 46, 55, 64, 73, 36, 38, 45, 47 }, 
   { 27, 28, 30, 31, 32, 33, 34, 35,  2, 11, 20, 38, 47, 56, 65, 74, 36, 37, 45, 46 }, 
   { 27, 28, 29, 31, 32, 33, 34, 35,  3, 12, 21, 39, 48, 57, 66, 75, 40, 41, 49, 50 }, 
   { 27, 28, 29, 30, 32, 33, 34, 35,  4, 13, 22, 40, 49, 58, 67, 76, 39, 41, 48, 50 }, 
   { 27, 28, 29, 30, 31, 33, 34, 35,  5, 14, 23, 41, 50, 59, 68, 77, 39, 40, 48, 49 }, 
   { 27, 28, 29, 30, 31, 32, 34, 35,  6, 15, 24, 42, 51, 60, 69, 78, 43, 44, 52, 53 }, 
   { 27, 28, 29, 30, 31, 32, 33, 35,  7, 16, 25, 43, 52, 61, 70, 79, 42, 44, 51, 53 }, 
   { 27, 28, 29, 30, 31, 32, 33, 34,  8, 17, 26, 44, 53, 62, 71, 80, 42, 43, 51, 52 }, 
   { 37, 38, 39, 40, 41, 42, 43, 44,  0,  9, 18, 27, 45, 54, 63, 72, 28, 29, 46, 47 }, 
   { 36, 38, 39, 40, 41, 42, 43, 44,  1, 10, 19, 28, 46, 55, 64, 73, 27, 29, 45, 47 }, 
   { 36, 37, 39, 40, 41, 42, 43, 44,  2, 11, 20, 29, 47, 56, 65, 74, 27, 28, 45, 46 }, 
   { 36, 37, 38, 40, 41, 42, 43, 44,  3, 12, 21, 30, 48, 57, 66, 75, 31, 32, 49, 50 }, 
   { 36, 37, 38, 39, 41, 42, 43, 44,  4, 13, 22, 31, 49, 58, 67, 76, 30, 32, 48, 50 }, 
   { 36, 37, 38, 39, 40, 42, 43, 44,  5, 14, 23, 32, 50, 59, 68, 77, 30, 31, 48, 49 }, 
   { 36, 37, 38, 39, 40, 41, 43, 44,  6, 15, 24, 33, 51, 60, 69, 78, 34, 35, 52, 53 }, 
   { 36, 37, 38, 39, 40, 41, 42, 44,  7, 16, 25, 34, 52, 61, 70, 79, 33, 35, 51, 53 }, 
   { 36, 37, 38, 39, 40, 41, 42, 43,  8, 17, 26, 35, 53, 62, 71, 80, 33, 34, 51, 52 }, 
   { 46, 47, 48, 49, 50, 51, 52, 53,  0,  9, 18, 27, 36, 54, 63, 72, 28, 29, 37, 38 }, 
   { 45, 47, 48, 49, 50, 51, 52, 53,  1, 10, 19, 28, 37, 55, 64, 73, 27, 29, 36, 38 }, 
   { 45, 46, 48, 49, 50, 51, 52, 53,  2, 11, 20, 29, 38, 56, 65, 74, 27, 28, 36, 37 }, 
   { 45, 46, 47, 49, 50, 51, 52, 53,  3, 12, 21, 30, 39, 57, 66, 75, 31, 32, 40, 41 }, 
   { 45, 46, 47, 48, 50, 51, 52, 53,  4, 13, 22, 31, 40, 58, 67, 76, 30, 32, 39, 41 }, 
   { 45, 46, 47, 48, 49, 51, 52, 53,  5, 14, 23, 32, 41, 59, 68, 77, 30, 31, 39, 40 }, 
   { 45, 46, 47, 48, 49, 50, 52, 53,  6, 15, 24, 33, 42, 60, 69, 78, 34, 35, 43, 44 }, 
   { 45, 46, 47, 48, 49, 50, 51, 53,  7, 16, 25, 34, 43, 61, 70, 79, 33, 35, 42, 44 }, 
   { 45, 46, 47, 48, 49, 50, 51, 52,  8, 17, 26, 35, 44, 62, 71, 80, 33, 34, 42, 43 }, 
   { 55, 56, 57, 58, 59, 60, 61, 62,  0,  9, 18, 27, 36, 45, 63, 72, 64, 65, 73, 74 }, 
   { 54, 56, 57, 58, 59, 60, 61, 62,  1, 10, 19, 28, 37, 46, 64, 73, 63, 65, 72, 74 }, 
   { 54, 55, 57, 58, 59, 60, 61, 62,  2, 11, 20, 29, 38, 47, 65, 74, 63, 64, 72, 73 }, 
   { 54, 55, 56, 58, 59, 60, 61, 62,  3, 12, 21, 30, 39, 48, 66, 75, 67, 68, 76, 77 }, 
   { 54, 55, 56, 57, 59, 60, 61, 62,  4, 13, 22, 31, 40, 49, 67, 76, 66, 68, 75, 77 }, 
   { 54, 55, 56, 57, 58, 60, 61, 62,  5, 14, 23, 32, 41, 50, 68, 77, 66, 67, 75, 76 }, 
   { 54, 55, 56, 57, 58, 59, 61, 62,  6, 15, 24, 33, 42, 51, 69, 78, 70, 71, 79, 80 }, 
   { 54, 55, 56, 57, 58, 59, 60, 62,  7, 16, 25, 34, 43, 52, 70, 79, 69, 71, 78, 80 }, 
   { 54, 55, 56, 57, 58, 59, 60, 61,  8, 17, 26, 35, 44, 53, 71, 80, 69, 70, 78, 79 }, 
   { 64, 65, 66, 67, 68, 69, 70, 71,  0,  9, 18, 27, 36, 45, 54, 72, 55, 56, 73, 74 }, 
   { 63, 65, 66, 67, 68, 69, 70, 71,  1, 10, 19, 28, 37, 46, 55, 73, 54, 56, 72, 74 }, 
   { 63, 64, 66, 67, 68, 69, 70, 71,  2, 11, 20, 29, 38, 47, 56, 74, 54, 55, 72, 73 }, 
   { 63, 64, 65, 67, 68, 69, 70, 71,  3, 12, 21, 30, 39, 48, 57, 75, 58, 59, 76, 77 }, 
   { 63, 64, 65, 66, 68, 69, 70, 71,  4, 13, 22, 31, 40, 49, 58, 76, 57, 59, 75, 77 }, 
   { 63, 64, 65, 66, 67, 69, 70, 71,  5, 14, 23, 32, 41, 50, 59, 77, 57, 58, 75, 76 }, 
   { 63, 64, 65, 66, 67, 68, 70, 71,  6, 15, 24, 33, 42, 51, 60, 78, 61, 62, 79, 80 }, 
   { 63, 64, 65, 66, 67, 68, 69, 71,  7, 16, 25, 34, 43, 52, 61, 79, 60, 62, 78, 80 }, 
   { 63, 64, 65, 66, 67, 68, 69, 70,  8, 17, 26, 35, 44, 53, 62, 80, 60, 61, 78, 79 }, 
   { 73, 74, 75, 76, 77, 78, 79, 80,  0,  9, 18, 27, 36, 45, 54, 63, 55, 56, 64, 65 }, 
   { 72, 74, 75, 76, 77, 78, 79, 80,  1, 10, 19, 28, 37, 46, 55, 64, 54, 56, 63, 65 }, 
   { 72, 73, 75, 76, 77, 78, 79, 80,  2, 11, 20, 29, 38, 47, 56, 65, 54, 55, 63, 64 }, 
   { 72, 73, 74, 76, 77, 78, 79, 80,  3, 12, 21, 30, 39, 48, 57, 66, 58, 59, 67, 68 }, 
   { 72, 73, 74, 75, 77, 78, 79, 80,  4, 13, 22, 31, 40, 49, 58, 67, 57, 59, 66, 68 }, 
   { 72, 73, 74, 75, 76, 78, 79, 80,  5, 14, 23, 32, 41, 50, 59, 68, 57, 58, 66, 67 }, 
   { 72, 73, 74, 75, 76, 77, 79, 80,  6, 15, 24, 33, 42, 51, 60, 69, 61, 62, 70, 71 }, 
   { 72, 73, 74, 75, 76, 77, 78, 80,  7, 16, 25, 34, 43, 52, 61, 70, 60, 62, 69, 71 }, 
   { 72, 73, 74, 75, 76, 77, 78, 79,  8, 17, 26, 35, 44, 53, 62, 71, 60, 61, 69, 70 }}; 

// Defines the groups used in a standard 3x3 grid 
static const unsigned char Group[27][9] = 
  {{  0,  1,  2,  3,  4,  5,  6,  7,  8 }, {  9, 10, 11, 12, 13, 14, 15, 16, 17 }, { 18, 19, 20, 21, 22, 23, 24, 25, 26 }, 
   { 27, 28, 29, 30, 31, 32, 33, 34, 35 }, { 36, 37, 38, 39, 40, 41, 42, 43, 44 }, { 45, 46, 47, 48, 49, 50, 51, 52, 53 }, 
   { 54, 55, 56, 57, 58, 59, 60, 61, 62 }, { 63, 64, 65, 66, 67, 68, 69, 70, 71 }, { 72, 73, 74, 75, 76, 77, 78, 79, 80 }, 

   {  0,  9, 18, 27, 36, 45, 54, 63, 72 }, {  1, 10, 19, 28, 37, 46, 55, 64, 73 }, {  2, 11, 20, 29, 38, 47, 56, 65, 74 }, 
   {  3, 12, 21, 30, 39, 48, 57, 66, 75 }, {  4, 13, 22, 31, 40, 49, 58, 67, 76 }, {  5, 14, 23, 32, 41, 50, 59, 68, 77 }, 
   {  6, 15, 24, 33, 42, 51, 60, 69, 78 }, {  7, 16, 25, 34, 43, 52, 61, 70, 79 }, {  8, 17, 26, 35, 44, 53, 62, 71, 80 }, 

   {  0,  1,  2,  9, 10, 11, 18, 19, 20 }, {  3,  4,  5, 12, 13, 14, 21, 22, 23 }, {  6,  7,  8, 15, 16, 17, 24, 25, 26 }, 
   { 27, 28, 29, 36, 37, 38, 45, 46, 47 }, { 30, 31, 32, 39, 40, 41, 48, 49, 50 }, { 33, 34, 35, 42, 43, 44, 51, 52, 53 }, 
   { 54, 55, 56, 63, 64, 65, 72, 73, 74 }, { 57, 58, 59, 66, 67, 68, 75, 76, 77 }, { 60, 61, 62, 69, 70, 71, 78, 79, 80 }}; 

// Defines the 3 groups each position is part of 
static const unsigned char C2Grp[81][3] = 
  {{ 0,  9, 18}, { 0, 10, 18}, { 0, 11, 18}, { 0, 12, 19}, { 0, 13, 19}, { 0, 14, 19}, { 0, 15, 20}, { 0, 16, 20}, { 0, 17, 20}, 
   { 1,  9, 18}, { 1, 10, 18}, { 1, 11, 18}, { 1, 12, 19}, { 1, 13, 19}, { 1, 14, 19}, { 1, 15, 20}, { 1, 16, 20}, { 1, 17, 20}, 
   { 2,  9, 18}, { 2, 10, 18}, { 2, 11, 18}, { 2, 12, 19}, { 2, 13, 19}, { 2, 14, 19}, { 2, 15, 20}, { 2, 16, 20}, { 2, 17, 20}, 
   { 3,  9, 21}, { 3, 10, 21}, { 3, 11, 21}, { 3, 12, 22}, { 3, 13, 22}, { 3, 14, 22}, { 3, 15, 23}, { 3, 16, 23}, { 3, 17, 23}, 
   { 4,  9, 21}, { 4, 10, 21}, { 4, 11, 21}, { 4, 12, 22}, { 4, 13, 22}, { 4, 14, 22}, { 4, 15, 23}, { 4, 16, 23}, { 4, 17, 23}, 
   { 5,  9, 21}, { 5, 10, 21}, { 5, 11, 21}, { 5, 12, 22}, { 5, 13, 22}, { 5, 14, 22}, { 5, 15, 23}, { 5, 16, 23}, { 5, 17, 23}, 
   { 6,  9, 24}, { 6, 10, 24}, { 6, 11, 24}, { 6, 12, 25}, { 6, 13, 25}, { 6, 14, 25}, { 6, 15, 26}, { 6, 16, 26}, { 6, 17, 26}, 
   { 7,  9, 24}, { 7, 10, 24}, { 7, 11, 24}, { 7, 12, 25}, { 7, 13, 25}, { 7, 14, 25}, { 7, 15, 26}, { 7, 16, 26}, { 7, 17, 26}, 
   { 8,  9, 24}, { 8, 10, 24}, { 8, 11, 24}, { 8, 12, 25}, { 8, 13, 25}, { 8, 14, 25}, { 8, 15, 26}, { 8, 16, 26}, { 8, 17, 26}}; 


// Defines the interaction of segments used for Lock Candidate searches 
static const unsigned char LC_Segment[9][4] = 
  {{1, 2, 3, 6}, {0, 2, 4, 7}, {0, 1, 5, 8}, {4, 5, 0, 6}, {3, 5, 1, 7}, {3, 4, 2, 8}, {7, 8, 0, 3}, {6, 8, 1, 4}, {6, 7, 2, 5}}; 

static struct Grid_Type 
{ int          CellsLeft;  // Cells left to solve 
  unsigned int Grid[81];   // Possibilities left for each cell 
  unsigned int Grp[27];    // Values found in each group 
} G[64], *Gp; 

//register struct Grid_Type *Gx; 

// Solution Grid, which will contain the answer after the puzzle is solved 
static unsigned int SolGrid[81]; 

// Naked Singles FIFO stack - new singles found are stored here before 
static char  SingleCnt = 0; 
static char  SinglePos[128]; 
static unsigned int SingleVal[128]; 
#define PushSingle(p, b) { SinglePos[SingleCnt] = p; SingleVal[SingleCnt] = b; SingleCnt++; Changed = 1; } 


// Changed Flags 
static int  Changed;          // Flag to indicate Grid has changed 
static char ChangedGroup[27]; // Marks which groups have changed 
static char ChangedLC[27]; 
static int  SingleGroup[27]; 

#define MarkChanged(x) { unsigned char const *ucp = C2Grp[x]; ChangedGroup[*ucp] = ChangedGroup[*(ucp+1)] = ChangedGroup[*(ucp+2)] = Changed = 1; } 
// original 
//#define MarkChanged(x) { ChangedGroup[C2Grp[x][0]] = ChangedGroup[C2Grp[x][1]] = ChangedGroup[C2Grp[x][2]] = Changed = 1; } 
// Seems to work as well. But not for everyone 
//#define MarkChanged(x) ChangedGroup[C2Grp[x][0]] = Changed = 1; 


// Guess structure 
static char GuessPos[128]; 
static int  GuessVal[128]; 
static int  OneStepP[81], OneStepI; 


// Key global variables used throughout the solving 
static int    PuzSolCnt;        // Solutions for a single puzzle 
static int    No_Sol;           // Flag to indicate there is no solution possible 
static int    PIdx;             // Position Index, used for guessing and backtracking 


// Debug stats 
#ifdef CALC_STATS 
static int  SCnt  = 0,  HCnt  = 0, GCnt  = 0, LCCnt  = 0, SSCnt  = 0, FCnt  = 0, OneCnt  = 0, TwoCnt  = 0; 
static long TSCnt = 0,  THCnt = 0, TGCnt = 0, TLCCnt = 0, TSSCnt = 0, TFCnt = 0, TOneCnt = 0, TTwoCnt = 0; 
#endif 
static int  MaxDepth = 0, Givens = 0; 


// forward procedure declarations 
static void InitGrid (); 
static void ProcessInitSingles (void); 
static void ProcessSingles (void); 
static void FindHiddenSingles (void); 
static void FindLockedCandidates (void); 
static void MakeGuess (void); 



/****************\ 
**  InitTables  *************************************************\ 
**                                                              ** 
**    populates the B2V and NSet tables.  They can            ** 
**    be initialized when defined, but if the grid sizes are    ** 
**    increased, the size of these tables will also increase    ** 
**    exponentially (9x9=512 items, 25x25=33,554,432 items).    ** 
**                                                              ** 
\****************************************************************/ 
void bb_init(void) 
{ int i, v; 

//  RND_Init(); 
  for (i = 0; i < 512; i++) 
  { B2V[i] = NSet[i] = 0; 
    for (v = i; v; NSet[i]++) 
    { NSetX[i][NSet[i]] = v & -v; 
      v = v ^ NSetX[i][NSet[i]]; 
    } 
  } 
  for (i = 1; i <= 9; i++) 
    B2V[1 << (i-1)] = i; 
} 


/************\ 
**  Solver  *****************************************************\ 
**                                                              ** 
**    Solver runs the sudoku solver.  Input puzzle is in the    ** 
**    buffer array, and somewhat controlled by a number of      ** 
**    globals (see the globals at the top of the main program   ** 
**    for globals and meanings).                                ** 
**                                                              ** 
\****************************************************************/ 
int bb_solver(unsigned *puzzle) 
{ register unsigned i; 

  PuzSolCnt = 0; 

  InitGrid(); 

  for (i = 0; i < 81; i++) 
     if (puzzle[i]) 
        PushSingle((char)(i), V2B[puzzle[i]]); 

  // Loop through the puzzle solving routines until finished 
  while (Changed) 
  { // If No Solution possible, jump straight to the backtrack routine 
    if (!No_Sol) 
    { // Check if any Singles to be propogated 
      if (SingleCnt) 
      { 
#ifdef CALC_STATS 
        SCnt++; 
#endif 
        if (SingleCnt > 2)               // If multiple singles 
          ProcessInitSingles();          //   process them all at once 
        if (SingleCnt)                   // otherwise 
          ProcessSingles();              //   process them one at a time 
        if (!Gp->CellsLeft) 
        { if (!No_Sol) 
          { PuzSolCnt++; 
            if (PuzSolCnt > 1) break; 
          } 
          No_Sol = Changed = 1; 
          continue; 
        } 
      } 

      // If nothing has changed, apply the next solver 
      if (Changed) 
      { 
#ifdef CALC_STATS 
        HCnt++; 
#endif 
        FindHiddenSingles(); if (SingleCnt) continue; 
//        if (use_methods & USE_LOCK_CAND) 
          { 
#ifdef CALC_STATS 
         LCCnt++; 
#endif 
         FindLockedCandidates(); if (Changed) continue; } 
      } 
    } 

    //If nothing new found, just make a guess 
#ifdef CALC_STATS 
   GCnt++; 
#endif 
   MakeGuess(); 
    if (No_Sol) break; 
//    if (!initp && (MaxDepth < PIdx)) MaxDepth = PIdx; 
//    if (PIdx > 62) { printf ("Max Depth exceeded, recompile for more depth.\n\n"); exit(0); } 
  } 

#ifdef CALC_STATS 
  TSCnt   += SCnt;    // Update Stats 
  THCnt   += HCnt; 
  TGCnt   += GCnt; 
  TLCCnt  += LCCnt; 
  TSSCnt  += SSCnt; 
  TFCnt   += FCnt; 
  TOneCnt += OneCnt; 
  TTwoCnt += TwoCnt; 
#endif 
  return PuzSolCnt; 
} 





/**************\ 
**  InitGrid  ***************************************************\ 
**                                                              ** 
**    InitGrid takes the text string stored in the buffer and   ** 
**    populates the solution grid.  It assumes the text is      ** 
**    properly formatted.                                       ** 
**                                                              ** 
\****************************************************************/ 
static void InitGrid (void) 
{ register char i; 

  // Initialize some key variables 
  Gp = G; 
  Gp->CellsLeft = 81; 
  PIdx = 0; 
  SingleCnt = 0; 
  Changed = 1; 
  No_Sol = 0; 
  OneStepI = 0; 
  Givens = 0; 
#ifdef CALC_STATS 
  SCnt = HCnt = GCnt = LCCnt = SSCnt = FCnt = OneCnt = TwoCnt = 0; 
#endif 

  // Loop through the buffer and set the singles to be set 
  for (i = 0; i < 81; i++) 
  { Gp->Grid[i] = 0x1FF; 
    SolGrid[i] = OneStepP[i] = 0; 
  } 

  // Clear the Groups Found values 
  for (i = 0; i < 27; i++) 
    Gp->Grp[i] = ChangedGroup[i] = ChangedLC[i] = SingleGroup[i] = 0; 
} 



/************************\ 
**  ProcessInitSingles  *****************************************\ 
**                                                              ** 
**    ProcessInitSingles takes a naked single and marks each    ** 
**    cell in the 3 associated groups as not allowing that      ** 
**    number.  It also marks the groups as changed so we know   ** 
**    to check for hidden singles in that group.                ** 
**                                                              ** 
**    This routines marks all the groups first, then marks the  ** 
**    cells for each changed groups.                            ** 
**                                                              ** 
\****************************************************************/ 
static void ProcessInitSingles (void) 
{ register unsigned char const *ucp; 
  register int i, t, g, t2, j; 
  unsigned int b; 
      
   while (SingleCnt > 2)  { 
      for (i = 0; i < SingleCnt; i++){ 
         t = SinglePos[i];                     // Get local copy of position 
         b = SingleVal[i];                     // Get local copy of the value 

         if (Gp->Grid[t] == 0) continue;   // Check if we already processed this position 
         if (!(Gp->Grid[t] & b)) {          // Check for error conditions 
            No_Sol = 1; SingleCnt = Changed = 0; 
            return; 
         }  
         SolGrid[t] = b; // Store the single in the solution grid 
         Gp->CellsLeft--;                  // mark one less empty space 
         Gp->Grid[t] = 0;                  // mark this position processed 
         ucp = C2Grp[t]; 
         for (g = 0; g < 3; g++) {              // loop through all 3 groups 
            if (Gp->Grp[*(ucp++)] & b) { 
               No_Sol = 1; SingleCnt = Changed = 0; 
               return; 
            }  
            Gp->Grp[C2Grp[t][g]] |= b;      // mark the value as found in the group 
            SingleGroup[C2Grp[t][g]]  = 1; 
         } 
      } 

      SingleCnt = 0; 
      for (i = 0; i < 27; i++) 
         if (SingleGroup[i]) { 
            SingleGroup[i] = 0; 
            for (j = 0; j < 9; j++) { 
               t2 = Group[i][j];                 // get temp copy of position 
               b = Gp->Grp[i]; 
               if (Gp->Grid[t2] & b) {        // check if removing a possibility 
                  Gp->Grid[t2] = Gp->Grid[t2] & ~b;   // remove possibility 
                  if (Gp->Grid[t2] == 0) {                 // check for error (no possibility) 
                     No_Sol = 1; SingleCnt = 0; Changed = 0; 
                     return; 
                  }  
                  if (B2V[Gp->Grid[t2]])                  // Check if a naked single is found 
                  PushSingle(t2, Gp->Grid[t2]);          
                  MarkChanged(t2);                            // Mark groups as changed 
               } 
            } 
         } 
   } 
} 


/********************\ 
**  ProcessSingles  *********************************************\ 
**                                                              ** 
**    ProcessSingles takes a naked single and marks each cell   ** 
**    in the 3 associated groups as not allowing that number.   ** 
**    It also marks the groups as changed so we know to check   ** 
**    for hidden singles in that group.                         ** 
**                                                              ** 
**    This routines marks cells changed as each single is       ** 
**    processed.                                                ** 
**                                                              ** 
\****************************************************************/ 
static void ProcessSingles (void) 
{ int i, t, g, t2; 
register  unsigned int b; 

  for (i = 0; i < SingleCnt; i++) 
  { t = SinglePos[i];                     // Get local copy of position 
    b = SingleVal[i];                     // Get local copy of the value 

    if (Gp->Grid[t] == 0) continue;   // Check if we already processed this position 
    if (!(Gp->Grid[t] & b))           // Check for error conditions 
    { No_Sol = 1; SingleCnt = Changed = 0; return; } 
    SolGrid[t] = b; // Store the single in the solution grid 
    Gp->CellsLeft--;                  // mark one less empty space 
    Gp->Grid[t] = 0;                  // mark this position processed 

    for (g = 0; g < 3; g++)               // loop through all 3 groups 
      Gp->Grp[C2Grp[t][g]] |= b;      // mark the value as found in the group 
    for (g = 0; g < 20; g++)              // loop through each cell in the groups 
    { t2 = (int)In_Groups[t][g];          // get temp copy of position 
      if (Gp->Grid[t2] & b)           // check if removing a possibility 
      { Gp->Grid[t2] = Gp->Grid[t2] ^ b;    // remove possibility 
        if (Gp->Grid[t2] == 0)                  // check for error (no possibility) 
           { No_Sol = 1; SingleCnt = 0; Changed = 0; return; } 
        if (B2V[Gp->Grid[t2]])                  // Check if a naked single is found 
          PushSingle(t2, Gp->Grid[t2]); 
        MarkChanged(t2);                            // Mark groups as changed 
      } 
    } 
  } 
  SingleCnt = 0;                          // Clear the single count 
} 




/***********************\ 
**  FindHiddenSingles  ******************************************\ 
**                                                              ** 
**    FindHiddenSingles checks each grouping that has changed   ** 
**    to see if they contain any hidden singles.  If one is     ** 
**    found, the routine adds it to the queue and exits.        ** 
**                                                              ** 
\****************************************************************/ 
static void FindHiddenSingles (void) 
{ unsigned int t1, t2, t3, b, t; 
  int i, j; 
  unsigned char g; 

  for (i = 0; i < 27; i++) 
    if (ChangedGroup[i]) 
    { ChangedLC[i] = 1; 
      t1 = t2 = t3 = 0; 
      for (j = 0; j < 9; j++) 
      { b  = Gp->Grid[Group[i][j]]; 
        t2 = t2 | (t1 & b); 
        t1 = t1 | b; 
      } 
      if ((t1 | Gp->Grp[i]) != 0x01FF) 
        { No_Sol = 1; SingleCnt = 0; Changed = 0; return; } 
      t3 = t2 ^ t1; 
      if (t3) 
      { for (j = 0; j < 9; j++) 
       { g = Group[i][j]; 
          if (t3 & Gp->Grid[g]) 
          { t = t3 & Gp->Grid[g]; 
            if (!B2V[t]) { No_Sol = 1; SingleCnt = 0; Changed = 0; return; } 
            PushSingle((char)g, t); 
            if (t3 == t) return; 
            t3 = t3 ^ t; 
          } 
      } 
      } 
      ChangedGroup[i] = 0; 
    } 
  Changed = 0; 
} 




/**************************\ 
**  FindLockedCandidates  ***************************************\ 
**                                                              ** 
**    FindLockedCandidates will scan through the grid to find   ** 
**    any locked candidates.                                    ** 
**                                                              ** 
\****************************************************************/ 
static void FindLockedCandidates (void) 
{ unsigned int Seg[9], b; 
  int i, j, k, x; 
  int s, s1, s2, gp; 
  
  for (i = 0; i < 18; i += 3) 
    if (ChangedLC[i] || ChangedLC[i+1] || ChangedLC[i+2]) 
    { register unsigned *gpg = Gp->Grid; 
     ChangedLC[i] = ChangedLC[i+1] = ChangedLC[i+2] = 0; 
      Seg[0] = gpg[Group[i  ][0]] | gpg[Group[i  ][1]] | gpg[Group[i  ][2]]; 
      Seg[1] = gpg[Group[i  ][3]] | gpg[Group[i  ][4]] | gpg[Group[i  ][5]]; 
      Seg[2] = gpg[Group[i  ][6]] | gpg[Group[i  ][7]] | gpg[Group[i  ][8]]; 
      Seg[3] = gpg[Group[i+1][0]] | gpg[Group[i+1][1]] | gpg[Group[i+1][2]]; 
      Seg[4] = gpg[Group[i+1][3]] | gpg[Group[i+1][4]] | gpg[Group[i+1][5]]; 
      Seg[5] = gpg[Group[i+1][6]] | gpg[Group[i+1][7]] | gpg[Group[i+1][8]]; 
      Seg[6] = gpg[Group[i+2][0]] | gpg[Group[i+2][1]] | gpg[Group[i+2][2]]; 
      Seg[7] = gpg[Group[i+2][3]] | gpg[Group[i+2][4]] | gpg[Group[i+2][5]]; 
      Seg[8] = gpg[Group[i+2][6]] | gpg[Group[i+2][7]] | gpg[Group[i+2][8]]; 
      for (j = 0; j < 9; j++) 
      { b = Seg[j] & ((Seg[LC_Segment[j][0]] | Seg[LC_Segment[j][1]]) ^ (Seg[LC_Segment[j][2]] | Seg[LC_Segment[j][3]])); 
        if (b) 
        { 
          for (k = 0; k < 4; k++) 
          { s = LC_Segment[j][k]; 
            s1 = i+s/3;  s2 = (s%3)*3; 
            for (x = 0; x < 3; x++) 
            { gp = Group[s1][s2+x]; 
              if (Gp->Grid[gp] & b) 
              { Gp->Grid[gp] = Gp->Grid[gp] & ~b; 
                MarkChanged(gp); 
                if (!(Gp->Grid[gp])) 
                  { No_Sol = 1; SingleCnt = 0; Changed = 0; return; } 
                if (B2V[Gp->Grid[gp]]) 
                  PushSingle(gp, Gp->Grid[gp]); 
              } 
            } 
          } 
          return; 
        } 
      } 
    } 
} 




/***************\ 
**  MakeGuess  **************************************************\ 
**                                                              ** 
**    MakeGuess handles the guessing and backtracking used      ** 
**    when solving puzzles.                                     ** 
**                                                              ** 
\****************************************************************/ 
static void MakeGuess (void) 
{ 
  register char gpp = GuessPos[PIdx]; 
  register int ns; 
  static int LastPos = 0; 
  int i; 
  //unsigned 
     int v; 
  int Found = 0; 
  char mt; 

  Changed = 1; 
  SingleCnt = 0; 

  // Check to see where the next guess should be 
  /* 
  if (No_Sol) 
    while (No_Sol) 
    { if (PIdx == 0) return; 
      //Gp2 = Gp - 1; 
      (Gp - 1)->Grid[GuessPos[PIdx]] ^= GuessVal[PIdx]; 
      if ((Gp - 1)->Grid[GuessPos[PIdx]]) 
      { if (B2V[(Gp - 1)->Grid[GuessPos[PIdx]]]) 
          PushSingle(GuessPos[PIdx], (Gp - 1)->Grid[GuessPos[PIdx]]); 
        No_Sol = 0; 
        MarkChanged(GuessPos[PIdx]); 
        PIdx--; Gp--; 
        return; 
      } 
    } 
  */ 
  if (No_Sol)  { 
//  register char gpp = GuessPos[PIdx]; 
    while (No_Sol) 
    { if (PIdx == 0) return; 
      //Gp2 = Gp - 1; 
      (Gp - 1)->Grid[gpp] ^= GuessVal[PIdx]; 
      if ((Gp - 1)->Grid[gpp]) 
      { if (B2V[(Gp - 1)->Grid[gpp]]) 
          PushSingle(gpp, (Gp - 1)->Grid[gpp]); 
        No_Sol = 0; 
        MarkChanged(gpp); 
        PIdx--; Gp--; 
        return; 
      } 
    } 
  } 
  // Populate the grid for the next guess 
  G[PIdx+1] = G[PIdx]; 
  PIdx++; Gp++; 
      
  // Find next spot to check 
  if (!Found) 
  { mt = 99; 
    i = LastPos; 
    do 
    { ns = NSet[Gp->Grid[i]]; 
     if ((ns < mt) && (ns > 1)) 
      { GuessPos[PIdx] = i; 
        mt = ns; 
        if (mt == 2) break;  // if 2, then its the best it can get 
      } 
      if (++i >= 81) i = 0; 
    } while (i != LastPos); 
    LastPos = i; 
  } 

  // Mark the next guess in the position 
  No_Sol = 1; 
  v = Gp->Grid[GuessPos[PIdx]]; 
  if (v) 
  { 
     v = v & -v; 
    GuessVal[PIdx] = v; 
    PushSingle(GuessPos[PIdx], v); 
    Changed = 1; 
    No_Sol = 0; 
  } 
}

#include <stdio.h>
#include <string.h>

int main()
{
	char buf[1024];
	unsigned grid[81], i;
	bb_init();
	while(fgets(buf, 1024, stdin) != 0) {
		if (strlen(buf) < 81) continue;
		for (i = 0; i < 81; ++i)
			grid[i] = buf[i] >= '1' && buf[i] <= '9'? buf[i] - '0' : 0;
		printf("%d\n", bb_solver(grid));
		//for (i = 0; i < 81; ++i) putchar(SolGrid[i] + '0');
		//putchar('\n');
	}
	return 0;
}
