/* Concatenated two files and renamed the file. -ac */

/*****************
 * File game.cpp *
 *****************/

/***************************************************************************
A Fast Simple Sudoku Solver by Mladen Dobrichev

For commercial users the actions allowed to this file are limited to erasing.
No limitations to other users, but citing somewhere my name is welcome.
There are parts of code taken from bb_sudoku by Brian Turner. Thank you, Brian!

The entry point is int solve(char* in, char* out, int mode).

Some counterintuitive compilation hints:
- no global optimizations, only for speed
- no inlining
- no interprocedural optimizations
- __cdecl (default) calling convention
- no Profile Guided Optimizations

Therminology:
"Game" is the puzzle along with the whole context during processing.
Puzzle consist of 81 "Cells".
Each cell is member of three "Groups" - its "Row", "Column", and "Square".
The rest of the cells within the same row, column, and square are "Affected Cells".
Segment is a combination of first/second/third three rows or columns.
"Triplets" are first/second/third subsequent 3 cells within the rows and columns.
"Affected Triplets" for a given triplet are the rest of the triplets in the same row/col and square.
***************************************************************************/

//No includes
//No global variables
//No runtime integer division or multiplication
//Runtime Library function calls are limited to memcpy (hidden, when assigning the game struct)

//There is an Intel Compiler specific pragma "unroll" which causes significant improvement in the SetDigit function.
//It is critical this loop to be unrolled to obtain good performance. Splitting the loop into chunks
//may cause other compilers to unroll them. Last resort is manual unrolling.
//#define __INTEL_COMPILER
#ifndef __INTEL_COMPILER
#define MANUAL_UNROLL
#endif //__INTEL_COMPILER

//A quick way to experiment how data representation affects the performance
typedef unsigned int cellIndex;
typedef char cellDigit;
typedef unsigned short bitmap;

//Use constants and tabular functions whenever possible

//Convert a number to a mask with only the appropriate bit set
const bitmap Digit2Bitmap[17] =
{
	0x00000000,
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000
};

//Convert mask to a number (1 to 9) if only the appropriate bit is set.
//Convert zero to 9, allowing cellDigit to char conversion using the same table.
//Convert all the rest to zero.
const cellDigit Bitmap2Digit[512] =
{
        9,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//I am using hardware w/o SSE4 and POPCNT instruction
const int BitCount[512] =
{
        0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,5,6,6,7,6,7,7,8,6,7,7,8,7,8,8,9
};

/* Zero based cell enumeration
 9 10 11  12 13 14  15 16 17    Group

00 01 02  03 04 05  06 07 08	0
09 10 11  12 13 14  15 16 17	1	18	19	20
18 19 20  21 22 23  24 25 26	2

27 28 29  30 31 32  33 34 35	3
36 37 38  39 40 41  42 43 44	4	21	22	23
45 46 47  48 49 50  51 52 53	5

54 55 56  57 58 59  60 61 62	6
63 64 65  66 67 68  69 70 71	7	24	25	26
72 73 74  75 76 77  78 79 80	8
*/

//Cells' groups - row, column, square
const cellIndex affectedGroups[81][3] =
{
	{0, 9,18},{0,10,18},{0,11,18},{0,12,19},{0,13,19},{0,14,19},{0,15,20},{0,16,20},{0,17,20},
	{1, 9,18},{1,10,18},{1,11,18},{1,12,19},{1,13,19},{1,14,19},{1,15,20},{1,16,20},{1,17,20},
	{2, 9,18},{2,10,18},{2,11,18},{2,12,19},{2,13,19},{2,14,19},{2,15,20},{2,16,20},{2,17,20},
	{3, 9,21},{3,10,21},{3,11,21},{3,12,22},{3,13,22},{3,14,22},{3,15,23},{3,16,23},{3,17,23},
	{4, 9,21},{4,10,21},{4,11,21},{4,12,22},{4,13,22},{4,14,22},{4,15,23},{4,16,23},{4,17,23},
	{5, 9,21},{5,10,21},{5,11,21},{5,12,22},{5,13,22},{5,14,22},{5,15,23},{5,16,23},{5,17,23},
	{6, 9,24},{6,10,24},{6,11,24},{6,12,25},{6,13,25},{6,14,25},{6,15,26},{6,16,26},{6,17,26},
	{7, 9,24},{7,10,24},{7,11,24},{7,12,25},{7,13,25},{7,14,25},{7,15,26},{7,16,26},{7,17,26},
	{8, 9,24},{8,10,24},{8,11,24},{8,12,25},{8,13,25},{8,14,25},{8,15,26},{8,16,26},{8,17,26}
};

//The cell indexes in each of the 9 rows, 9 columns, and 9 squares
const cellIndex cellsInGroup[27][9] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8},{ 9,10,11,12,13,14,15,16,17},{18,19,20,21,22,23,24,25,26},
	{27,28,29,30,31,32,33,34,35},{36,37,38,39,40,41,42,43,44},{45,46,47,48,49,50,51,52,53},
	{54,55,56,57,58,59,60,61,62},{63,64,65,66,67,68,69,70,71},{72,73,74,75,76,77,78,79,80},
	{ 0, 9,18,27,36,45,54,63,72},{ 1,10,19,28,37,46,55,64,73},{ 2,11,20,29,38,47,56,65,74},
	{ 3,12,21,30,39,48,57,66,75},{ 4,13,22,31,40,49,58,67,76},{ 5,14,23,32,41,50,59,68,77},
	{ 6,15,24,33,42,51,60,69,78},{ 7,16,25,34,43,52,61,70,79},{ 8,17,26,35,44,53,62,71,80},
	{ 0, 1, 2, 9,10,11,18,19,20},{ 3, 4, 5,12,13,14,21,22,23},{ 6, 7, 8,15,16,17,24,25,26},
	{27,28,29,36,37,38,45,46,47},{30,31,32,39,40,41,48,49,50},{33,34,35,42,43,44,51,52,53},
	{54,55,56,63,64,65,72,73,74},{57,58,59,66,67,68,75,76,77},{60,61,62,69,70,71,78,79,80}
};

//Determining the digit of a cell forbids the same digit to be placed within the affected cells.
//8 cells in the row + 8 in the column + rest 4 in the square = total of 20.
//Below is the tabular function with all 20 affected cell indexes for each of the 81 cells.
const cellIndex affectedCells[81][20] =
{
	{ 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,18,19,20,27,36,45,54,63,72},
	{ 0, 2, 3, 4, 5, 6, 7, 8, 9,10,11,18,19,20,28,37,46,55,64,73},
	{ 0, 1, 3, 4, 5, 6, 7, 8, 9,10,11,18,19,20,29,38,47,56,65,74},
	{ 0, 1, 2, 4, 5, 6, 7, 8,12,13,14,21,22,23,30,39,48,57,66,75},
	{ 0, 1, 2, 3, 5, 6, 7, 8,12,13,14,21,22,23,31,40,49,58,67,76},
	{ 0, 1, 2, 3, 4, 6, 7, 8,12,13,14,21,22,23,32,41,50,59,68,77},
	{ 0, 1, 2, 3, 4, 5, 7, 8,15,16,17,24,25,26,33,42,51,60,69,78},
	{ 0, 1, 2, 3, 4, 5, 6, 8,15,16,17,24,25,26,34,43,52,61,70,79},
	{ 0, 1, 2, 3, 4, 5, 6, 7,15,16,17,24,25,26,35,44,53,62,71,80},
	{ 0, 1, 2,10,11,12,13,14,15,16,17,18,19,20,27,36,45,54,63,72},
	{ 0, 1, 2, 9,11,12,13,14,15,16,17,18,19,20,28,37,46,55,64,73},
	{ 0, 1, 2, 9,10,12,13,14,15,16,17,18,19,20,29,38,47,56,65,74},
	{ 3, 4, 5, 9,10,11,13,14,15,16,17,21,22,23,30,39,48,57,66,75},
	{ 3, 4, 5, 9,10,11,12,14,15,16,17,21,22,23,31,40,49,58,67,76},
	{ 3, 4, 5, 9,10,11,12,13,15,16,17,21,22,23,32,41,50,59,68,77},
	{ 6, 7, 8, 9,10,11,12,13,14,16,17,24,25,26,33,42,51,60,69,78},
	{ 6, 7, 8, 9,10,11,12,13,14,15,17,24,25,26,34,43,52,61,70,79},
	{ 6, 7, 8, 9,10,11,12,13,14,15,16,24,25,26,35,44,53,62,71,80},
	{ 0, 1, 2, 9,10,11,19,20,21,22,23,24,25,26,27,36,45,54,63,72},
	{ 0, 1, 2, 9,10,11,18,20,21,22,23,24,25,26,28,37,46,55,64,73},
	{ 0, 1, 2, 9,10,11,18,19,21,22,23,24,25,26,29,38,47,56,65,74},
	{ 3, 4, 5,12,13,14,18,19,20,22,23,24,25,26,30,39,48,57,66,75},
	{ 3, 4, 5,12,13,14,18,19,20,21,23,24,25,26,31,40,49,58,67,76},
	{ 3, 4, 5,12,13,14,18,19,20,21,22,24,25,26,32,41,50,59,68,77},
	{ 6, 7, 8,15,16,17,18,19,20,21,22,23,25,26,33,42,51,60,69,78},
	{ 6, 7, 8,15,16,17,18,19,20,21,22,23,24,26,34,43,52,61,70,79},
	{ 6, 7, 8,15,16,17,18,19,20,21,22,23,24,25,35,44,53,62,71,80},
	{ 0, 9,18,28,29,30,31,32,33,34,35,36,37,38,45,46,47,54,63,72},
	{ 1,10,19,27,29,30,31,32,33,34,35,36,37,38,45,46,47,55,64,73},
	{ 2,11,20,27,28,30,31,32,33,34,35,36,37,38,45,46,47,56,65,74},
	{ 3,12,21,27,28,29,31,32,33,34,35,39,40,41,48,49,50,57,66,75},
	{ 4,13,22,27,28,29,30,32,33,34,35,39,40,41,48,49,50,58,67,76},
	{ 5,14,23,27,28,29,30,31,33,34,35,39,40,41,48,49,50,59,68,77},
	{ 6,15,24,27,28,29,30,31,32,34,35,42,43,44,51,52,53,60,69,78},
	{ 7,16,25,27,28,29,30,31,32,33,35,42,43,44,51,52,53,61,70,79},
	{ 8,17,26,27,28,29,30,31,32,33,34,42,43,44,51,52,53,62,71,80},
	{ 0, 9,18,27,28,29,37,38,39,40,41,42,43,44,45,46,47,54,63,72},
	{ 1,10,19,27,28,29,36,38,39,40,41,42,43,44,45,46,47,55,64,73},
	{ 2,11,20,27,28,29,36,37,39,40,41,42,43,44,45,46,47,56,65,74},
	{ 3,12,21,30,31,32,36,37,38,40,41,42,43,44,48,49,50,57,66,75},
	{ 4,13,22,30,31,32,36,37,38,39,41,42,43,44,48,49,50,58,67,76},
	{ 5,14,23,30,31,32,36,37,38,39,40,42,43,44,48,49,50,59,68,77},
	{ 6,15,24,33,34,35,36,37,38,39,40,41,43,44,51,52,53,60,69,78},
	{ 7,16,25,33,34,35,36,37,38,39,40,41,42,44,51,52,53,61,70,79},
	{ 8,17,26,33,34,35,36,37,38,39,40,41,42,43,51,52,53,62,71,80},
	{ 0, 9,18,27,28,29,36,37,38,46,47,48,49,50,51,52,53,54,63,72},
	{ 1,10,19,27,28,29,36,37,38,45,47,48,49,50,51,52,53,55,64,73},
	{ 2,11,20,27,28,29,36,37,38,45,46,48,49,50,51,52,53,56,65,74},
	{ 3,12,21,30,31,32,39,40,41,45,46,47,49,50,51,52,53,57,66,75},
	{ 4,13,22,30,31,32,39,40,41,45,46,47,48,50,51,52,53,58,67,76},
	{ 5,14,23,30,31,32,39,40,41,45,46,47,48,49,51,52,53,59,68,77},
	{ 6,15,24,33,34,35,42,43,44,45,46,47,48,49,50,52,53,60,69,78},
	{ 7,16,25,33,34,35,42,43,44,45,46,47,48,49,50,51,53,61,70,79},
	{ 8,17,26,33,34,35,42,43,44,45,46,47,48,49,50,51,52,62,71,80},
	{ 0, 9,18,27,36,45,55,56,57,58,59,60,61,62,63,64,65,72,73,74},
	{ 1,10,19,28,37,46,54,56,57,58,59,60,61,62,63,64,65,72,73,74},
	{ 2,11,20,29,38,47,54,55,57,58,59,60,61,62,63,64,65,72,73,74},
	{ 3,12,21,30,39,48,54,55,56,58,59,60,61,62,66,67,68,75,76,77},
	{ 4,13,22,31,40,49,54,55,56,57,59,60,61,62,66,67,68,75,76,77},
	{ 5,14,23,32,41,50,54,55,56,57,58,60,61,62,66,67,68,75,76,77},
	{ 6,15,24,33,42,51,54,55,56,57,58,59,61,62,69,70,71,78,79,80},
	{ 7,16,25,34,43,52,54,55,56,57,58,59,60,62,69,70,71,78,79,80},
	{ 8,17,26,35,44,53,54,55,56,57,58,59,60,61,69,70,71,78,79,80},
	{ 0, 9,18,27,36,45,54,55,56,64,65,66,67,68,69,70,71,72,73,74},
	{ 1,10,19,28,37,46,54,55,56,63,65,66,67,68,69,70,71,72,73,74},
	{ 2,11,20,29,38,47,54,55,56,63,64,66,67,68,69,70,71,72,73,74},
	{ 3,12,21,30,39,48,57,58,59,63,64,65,67,68,69,70,71,75,76,77},
	{ 4,13,22,31,40,49,57,58,59,63,64,65,66,68,69,70,71,75,76,77},
	{ 5,14,23,32,41,50,57,58,59,63,64,65,66,67,69,70,71,75,76,77},
	{ 6,15,24,33,42,51,60,61,62,63,64,65,66,67,68,70,71,78,79,80},
	{ 7,16,25,34,43,52,60,61,62,63,64,65,66,67,68,69,71,78,79,80},
	{ 8,17,26,35,44,53,60,61,62,63,64,65,66,67,68,69,70,78,79,80},
	{ 0, 9,18,27,36,45,54,55,56,63,64,65,73,74,75,76,77,78,79,80},
	{ 1,10,19,28,37,46,54,55,56,63,64,65,72,74,75,76,77,78,79,80},
	{ 2,11,20,29,38,47,54,55,56,63,64,65,72,73,75,76,77,78,79,80},
	{ 3,12,21,30,39,48,57,58,59,66,67,68,72,73,74,76,77,78,79,80},
	{ 4,13,22,31,40,49,57,58,59,66,67,68,72,73,74,75,77,78,79,80},
	{ 5,14,23,32,41,50,57,58,59,66,67,68,72,73,74,75,76,78,79,80},
	{ 6,15,24,33,42,51,60,61,62,69,70,71,72,73,74,75,76,77,79,80},
	{ 7,16,25,34,43,52,60,61,62,69,70,71,72,73,74,75,76,77,78,80},
	{ 8,17,26,35,44,53,60,61,62,69,70,71,72,73,74,75,76,77,78,79}
};

/*
Segment enumeration
0 0 0     3 4 5
1 1 1     3 4 5
2 2 2     3 4 5

Triplets within a segment enumeration
000 111 222
333 444 555
666 777 888
*/

const cellIndex affectedTriplets[9][4] =
  {{1,2,3,6},{0,2,4,7},{0,1,5,8},{4,5,0,6},{3,5,1,7},{3,4,2,8},{7,8,0,3},{6,8,1,4},{6,7,2,5}};

//6 segments * 9 triplets * 12 affected cells
const cellIndex tripletAffectedCells[6][9][12] =
{
	{
	{ 3, 4, 5, 6, 7, 8, 9,10,11,18,19,20},{ 0, 1, 2, 6, 7, 8,12,13,14,21,22,23},{ 0, 1, 2, 3, 4, 5,15,16,17,24,25,26},
	{12,13,14,15,16,17, 0, 1, 2,18,19,20},{ 9,10,11,15,16,17, 3, 4, 5,21,22,23},{ 9,10,11,12,13,14, 6, 7, 8,24,25,26},
	{21,22,23,24,25,26, 0, 1, 2, 9,10,11},{18,19,20,24,25,26, 3, 4, 5,12,13,14},{18,19,20,21,22,23, 6, 7, 8,15,16,17}
	},{
	{30,31,32,33,34,35,36,37,38,45,46,47},{27,28,29,33,34,35,39,40,41,48,49,50},{27,28,29,30,31,32,42,43,44,51,52,53},
	{39,40,41,42,43,44,27,28,29,45,46,47},{36,37,38,42,43,44,30,31,32,48,49,50},{36,37,38,39,40,41,33,34,35,51,52,53},
	{48,49,50,51,52,53,27,28,29,36,37,38},{45,46,47,51,52,53,30,31,32,39,40,41},{45,46,47,48,49,50,33,34,35,42,43,44}
	},{
	{57,58,59,60,61,62,63,64,65,72,73,74},{54,55,56,60,61,62,66,67,68,75,76,77},{54,55,56,57,58,59,69,70,71,78,79,80},
	{66,67,68,69,70,71,54,55,56,72,73,74},{63,64,65,69,70,71,57,58,59,75,76,77},{63,64,65,66,67,68,60,61,62,78,79,80},
	{75,76,77,78,79,80,54,55,56,63,64,65},{72,73,74,78,79,80,57,58,59,66,67,68},{72,73,74,75,76,77,60,61,62,69,70,71}
	},{
	{27,36,45,54,63,72, 1,10,19, 2,11,20},{ 0, 9,18,54,63,72,28,37,46,29,38,47},{ 0, 9,18,27,36,45,55,64,73,56,65,74},
	{28,37,46,55,64,73, 0, 9,18, 2,11,20},{ 1,10,19,55,64,73,27,36,45,29,38,47},{ 1,10,19,28,37,46,54,63,72,56,65,74},
	{29,38,47,56,65,74, 0, 9,18, 1,10,19},{ 2,11,20,56,65,74,27,36,45,28,37,46},{ 2,11,20,29,38,47,54,63,72,55,64,73}
	},{
	{30,39,48,57,66,75, 4,13,22, 5,14,23},{ 3,12,21,57,66,75,31,40,49,32,41,50},{ 3,12,21,30,39,48,58,67,76,59,68,77},
	{31,40,49,58,67,76, 3,12,21, 5,14,23},{ 4,13,22,58,67,76,30,39,48,32,41,50},{ 4,13,22,31,40,49,57,66,75,59,68,77},
	{32,41,50,59,68,77, 3,12,21, 4,13,22},{ 5,14,23,59,68,77,30,39,48,31,40,49},{ 5,14,23,32,41,50,57,66,75,58,67,76}
	},{
	{33,42,51,60,69,78, 7,16,25, 8,17,26},{ 6,15,24,60,69,78,34,43,52,35,44,53},{ 6,15,24,33,42,51,61,70,79,62,71,80},
	{34,43,52,61,70,79, 6,15,24, 8,17,26},{ 7,16,25,61,70,79,33,42,51,35,44,53},{ 7,16,25,34,43,52,60,69,78,62,71,80},
	{35,44,53,62,71,80, 6,15,24, 7,16,25},{ 8,17,26,62,71,80,33,42,51,34,43,52},{ 8,17,26,35,44,53,60,69,78,61,70,79}
	}
};

//6 segments * 9 triplets * 3 cells in triplet
const cellIndex tripletCells[6][9][3] =
{
	{{cellsInGroup[ 0][0],cellsInGroup[ 0][1],cellsInGroup[ 0][2]},{cellsInGroup[ 0][3],cellsInGroup[ 0][4],cellsInGroup[ 0][5]},{cellsInGroup[ 0][6],cellsInGroup[ 0][7],cellsInGroup[ 0][8]},
	 {cellsInGroup[ 1][0],cellsInGroup[ 1][1],cellsInGroup[ 1][2]},{cellsInGroup[ 1][3],cellsInGroup[ 1][4],cellsInGroup[ 1][5]},{cellsInGroup[ 1][6],cellsInGroup[ 1][7],cellsInGroup[ 1][8]},
	 {cellsInGroup[ 2][0],cellsInGroup[ 2][1],cellsInGroup[ 2][2]},{cellsInGroup[ 2][3],cellsInGroup[ 2][4],cellsInGroup[ 2][5]},{cellsInGroup[ 2][6],cellsInGroup[ 2][7],cellsInGroup[ 2][8]}},
	{{cellsInGroup[ 3][0],cellsInGroup[ 3][1],cellsInGroup[ 3][2]},{cellsInGroup[ 3][3],cellsInGroup[ 3][4],cellsInGroup[ 3][5]},{cellsInGroup[ 3][6],cellsInGroup[ 3][7],cellsInGroup[ 3][8]},
	 {cellsInGroup[ 4][0],cellsInGroup[ 4][1],cellsInGroup[ 4][2]},{cellsInGroup[ 4][3],cellsInGroup[ 4][4],cellsInGroup[ 4][5]},{cellsInGroup[ 4][6],cellsInGroup[ 4][7],cellsInGroup[ 4][8]},
	 {cellsInGroup[ 5][0],cellsInGroup[ 5][1],cellsInGroup[ 5][2]},{cellsInGroup[ 5][3],cellsInGroup[ 5][4],cellsInGroup[ 5][5]},{cellsInGroup[ 5][6],cellsInGroup[ 5][7],cellsInGroup[ 5][8]}},
	{{cellsInGroup[ 6][0],cellsInGroup[ 6][1],cellsInGroup[ 6][2]},{cellsInGroup[ 6][3],cellsInGroup[ 6][4],cellsInGroup[ 6][5]},{cellsInGroup[ 6][6],cellsInGroup[ 6][7],cellsInGroup[ 6][8]},
	 {cellsInGroup[ 7][0],cellsInGroup[ 7][1],cellsInGroup[ 7][2]},{cellsInGroup[ 7][3],cellsInGroup[ 7][4],cellsInGroup[ 7][5]},{cellsInGroup[ 7][6],cellsInGroup[ 7][7],cellsInGroup[ 7][8]},
	 {cellsInGroup[ 8][0],cellsInGroup[ 8][1],cellsInGroup[ 8][2]},{cellsInGroup[ 8][3],cellsInGroup[ 8][4],cellsInGroup[ 8][5]},{cellsInGroup[ 8][6],cellsInGroup[ 8][7],cellsInGroup[ 8][8]}},
	{{cellsInGroup[ 9][0],cellsInGroup[ 9][1],cellsInGroup[ 9][2]},{cellsInGroup[ 9][3],cellsInGroup[ 9][4],cellsInGroup[ 9][5]},{cellsInGroup[ 9][6],cellsInGroup[ 9][7],cellsInGroup[ 9][8]},
	 {cellsInGroup[10][0],cellsInGroup[10][1],cellsInGroup[10][2]},{cellsInGroup[10][3],cellsInGroup[10][4],cellsInGroup[10][5]},{cellsInGroup[10][6],cellsInGroup[10][7],cellsInGroup[10][8]},
	 {cellsInGroup[11][0],cellsInGroup[11][1],cellsInGroup[11][2]},{cellsInGroup[11][3],cellsInGroup[11][4],cellsInGroup[11][5]},{cellsInGroup[11][6],cellsInGroup[11][7],cellsInGroup[11][8]}},
	{{cellsInGroup[12][0],cellsInGroup[12][1],cellsInGroup[12][2]},{cellsInGroup[12][3],cellsInGroup[12][4],cellsInGroup[12][5]},{cellsInGroup[12][6],cellsInGroup[12][7],cellsInGroup[12][8]},
	 {cellsInGroup[13][0],cellsInGroup[13][1],cellsInGroup[13][2]},{cellsInGroup[13][3],cellsInGroup[13][4],cellsInGroup[13][5]},{cellsInGroup[13][6],cellsInGroup[13][7],cellsInGroup[13][8]},
	 {cellsInGroup[14][0],cellsInGroup[14][1],cellsInGroup[14][2]},{cellsInGroup[14][3],cellsInGroup[14][4],cellsInGroup[14][5]},{cellsInGroup[14][6],cellsInGroup[14][7],cellsInGroup[14][8]}},
	{{cellsInGroup[15][0],cellsInGroup[15][1],cellsInGroup[15][2]},{cellsInGroup[15][3],cellsInGroup[15][4],cellsInGroup[15][5]},{cellsInGroup[15][6],cellsInGroup[15][7],cellsInGroup[15][8]},
	 {cellsInGroup[16][0],cellsInGroup[16][1],cellsInGroup[16][2]},{cellsInGroup[16][3],cellsInGroup[16][4],cellsInGroup[16][5]},{cellsInGroup[16][6],cellsInGroup[16][7],cellsInGroup[16][8]},
	 {cellsInGroup[17][0],cellsInGroup[17][1],cellsInGroup[17][2]},{cellsInGroup[17][3],cellsInGroup[17][4],cellsInGroup[17][5]},{cellsInGroup[17][6],cellsInGroup[17][7],cellsInGroup[17][8]}}
};

//game mode flags
#define MODE_SOLVING			0	//unused, keep solving
#define MODE_GUESS				1	//unused, may help distinguishing wrong guess from wrong initial conditions
#define MODE_WRONG_GUESS		2	//inconsistency found, discard guess or return error
#define MODE_STOP_PROCESSING	4	//solved or errored
#define MODE_SOLVED				8	//done
#define MODE_NO_OUTPUT			16	//only check for solution, do not write the result
#define MODE_ANY_SOLUTION		32	//stop on first solution
#define MODE_MULTIPLE_SOLUTONS	64	//second solution found

//The whole game context, cloned before guessing, and copied back after successful guess.
struct game
{
	bitmap cellPossibilities[81];	//0==known, initial 511=0x01FF
	int mode;						//combination of the game mode flags, initial 0
	int cellsLeft;					//initial 81
	int firstUnknownCell;			//initial 0
	cellDigit *cellDigits;			//pointer to a common array of the digits found, uninitialized
	bitmap groupKnownDigits[27];	//initial 0
};

//A template used for game structure initialization.
const game defaultGame =
{
	{ //cellPossibilities
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511
	},
	MODE_SOLVING,
	81, //cellsLeft
	0, //firstUnknownCell
	0, //&cellDigits
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} //groupKnownDigits
};

//Set a known digit at a known cell.
//Update (clear) possibilities at affected cells and call recursively
//when a single possibility is found.
//Update some other context
static void setDigit(game* g, const cellIndex i, const bitmap bm)
{
	//setting the same digit for second time may corrupt the rest of the data
	if(g->cellPossibilities[i] == 0) { //puzzle is overdetermined
		return;
	}
	const cellIndex *ag = affectedGroups[i];
	const cellIndex *ac = affectedCells[i];
	bitmap *gkd = g->groupKnownDigits;
	bitmap *cp = g->cellPossibilities;

	//if the digit we are setting has been previously set for one of
	//the 3 groups the cell is member of, we are in wrong way
	if(bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]])) {
		g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
		return;
	}
	cp[i] = 0; //clear the mask to exclude this cell from further processing

	//if we want the final solution, store the digit for printing
	if(0 == (g->mode & MODE_NO_OUTPUT))
		g->cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.

	if(0 == --g->cellsLeft) {//solved
		g->mode |= (MODE_SOLVED | MODE_STOP_PROCESSING);
		return;
	}

	//set the digit as solved for the all 3 groups
	gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;

	//clear the known digit from the possibilities of all related cells
#ifdef MANUAL_UNROLL
#define clp( ci ) {\
	bitmap *bbm = &cp[ac[ci]];\
	if(*bbm & bm) {\
		*bbm ^= bm; \
		if(Bitmap2Digit[*bbm]) { \
			setDigit(g, ac[ci], *bbm);\
			if(g->mode & MODE_STOP_PROCESSING) return;\
	}}}

	clp(0);clp(1);clp(2);clp(3);clp(4);clp(5);clp(6);clp(7);clp(8);clp(9);
	clp(10);clp(11);clp(12);clp(13);clp(14);clp(15);clp(16);clp(17);clp(18);clp(19);
#else //MANUAL_UNROLL
#ifdef __INTEL_COMPILER
#pragma unroll(20)
#endif //__INTEL_COMPILER
	for(int ci = 0; ci < 20; ci++) {
		bitmap *bbm = &cp[ac[ci]];
		if(*bbm & bm) { //skip already marked/solved cells
			*bbm ^= bm; //clear the appropriate possibility
			if(Bitmap2Digit[*bbm]) { //single digit?
				setDigit(g, ac[ci], *bbm);
				if(g->mode & MODE_STOP_PROCESSING)
					return;
			}
		}
	}
#endif //MANUAL_UNROLL
}

//Loop trough the groups, find the only cell containing particular digit in the group if any,
//then set the digit found.
//Since setting the digit changes the context, repeat until nothing found.
//On exit, the slower algorithms are expected to be run (i.e. guessing). 
static void checkForLastOccurenceInGroup(game* g)
{
	int repeat = 1;
	while (repeat)
	{
		//checks for group having some posible digit in only one cell
		repeat = 0;
		for (int gi = 0; gi < 27; gi++) {
			//if(g->groupKnownDigits[gi] == 511) continue;
			bitmap groupPoss = 0;
			bitmap duplicates = 0;
			const cellIndex *gc = cellsInGroup[gi];
			for (cellIndex ci = 0; ci < 9; ci++) {
				bitmap cellPoss = g->cellPossibilities[gc[ci]];
				duplicates |= (groupPoss & cellPoss); //this tricky code is taken from bb_sudoku by Brian Turner
				groupPoss |= cellPoss;
			}
			if((groupPoss ^ g->groupKnownDigits[gi]) != 511) { //no place for some of the unknown digits
				g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
				return;
			}
			bitmap uniques = groupPoss ^ duplicates;
			if(uniques == 0) continue;
			//clear the unique possibilities from the group and process the unique cells
			for (int ci = 0; ci < 9; ci++) {
				bitmap newPoss = g->cellPossibilities[gc[ci]] & uniques;
				if(newPoss == 0) continue;
				//one of the cells of interest found
				if (Bitmap2Digit[newPoss] == 0) { //error: the same cell introduced > 1 unique digits in the group
					g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
					return;
					}
				setDigit(g, gc[ci], newPoss);
				if(g->mode & MODE_STOP_PROCESSING)
					return;
				repeat = 1;
				//the benefit from the following optimization is questionable
				uniques ^= newPoss; //clear the already processed bit
				if (uniques == 0) break; //no more bits
			}
			//break;
		} //group loop
	} //while repeat
}

//The code is from bb_sudoku by Brian Turner.
static void FindLockedCandidates (game* g, int *found)
{
	*found = 0; //nothing found; after exiting continue with next algorithm.
	bitmap *gcp = g->cellPossibilities;
	for (int i = 0; i < 6; i++) {
		bitmap tripletPossibilities[9];
		for(int j = 0; j < 9; j++)
			tripletPossibilities[j] = gcp[tripletCells[i][j][0]] | gcp[tripletCells[i][j][1]] | gcp[tripletCells[i][j][2]];
		for (int j = 0; j < 9; j++) {
			bitmap b = (bitmap)(tripletPossibilities[j] & //found in the current triplet, and found in exactly one of the affected triplet pairs
				((tripletPossibilities[affectedTriplets[j][0]] | tripletPossibilities[affectedTriplets[j][1]]) ^ //row or column pair
				 (tripletPossibilities[affectedTriplets[j][2]] | tripletPossibilities[affectedTriplets[j][3]]))  //square pair
				);
			if (b) { //don't care which bit where came from
				for (int k = 0; k < 12; k++) { //6 from the row/col + 6 from the square
					int ci = tripletAffectedCells[i][j][k];
					bitmap *cp = &gcp[ci];
					if (*cp & b) { //there is something to clear
						*cp = (bitmap)(*cp & ~b);
						if (*cp == 0) { //no any possibility for this cell
							g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
							return;
						}
						if (Bitmap2Digit[*cp]) { //single possibility remained?
							setDigit(g, ci, *cp);
							if(g->mode & MODE_STOP_PROCESSING)
								return;
							*found = 1; //setting a digit is enough to force repeating of the whole checkings
						}
					}
				}
				//*found = 1; //clearing some bits is not enough to repeat the whole checkings
				//if(*found) return; //slower
				return; //don't ask why, but this works faster
			}
		} //for j
		//if(*found) return; //slower
	} //for i
}

//Perform all the solving algorithms, starting from faster ones
static void checkForWellKnownCells(game* g)
{
	int repeat = 1;
	while(repeat) {
		repeat = 0;
		checkForLastOccurenceInGroup(g); //the first algorithm performs internal repeating
		if(g->mode & MODE_STOP_PROCESSING)
			return;
		FindLockedCandidates(g, &repeat); //bb_sudoku by Brian Turner
		if(g->mode & MODE_STOP_PROCESSING)
			return;
		//if(repeat) continue; //something changed, try again the faster algorithms
		//...(g, &repeat);
		//if(g->mode & MODE_STOP_PROCESSING)
		//	return;
	}
}

//Perform any algorithms for solving.
//Finally make a guess and call recursively until success (solved) or failure (no solution).
static void attempt(game* g)
{
	if(g->mode & MODE_STOP_PROCESSING) //initialization caused finalization
		return;
	checkForWellKnownCells(g);
	if(g->mode & MODE_STOP_PROCESSING) //some of the algorithms reached solution or discrepancy 
		return;

	//Prepare a guess
	//g->mode |= MODE_GUESS; //still unused

	//Findout an unsolved cell with less possibilities
	int nGuesses = 10;
	cellIndex chosenCell;
	for(int ci = g->firstUnknownCell, firstIsUnknown = 1; ci < 81; ci++) {
		bitmap cp = g->cellPossibilities[ci];
		if(0 == cp) continue; //skip solved cells
		if(firstIsUnknown) g->firstUnknownCell = ci, firstIsUnknown = 0; //the benefit is about zero
		int bc;
		if((bc = BitCount[cp]) < nGuesses) {
			chosenCell = ci;
			if(2 == (nGuesses = bc)) //guessing a cell with 2 possibilities is OK
				break;
		}
	}

	//Try all the possibilities for the choosen cell and return on first or second success.
	//The following loop is a good candidate for parallelization if we want ALL the
	//solutions to be counted, but this is not the case.
	bitmap bm = g->cellPossibilities[chosenCell];
	int mode = MODE_WRONG_GUESS | MODE_STOP_PROCESSING; //assume all guesses are wrong
	for(int i = 1; nGuesses; i <<= 1) {
		if(0 == (bm & i)) continue; //skip cleared possibilities
		nGuesses--;
		game gg = *g; //local copy the game
		setDigit(&gg, chosenCell, (bitmap)(bm & i));
		//modify the code below if you wish to count solutions
		if(0 == (gg.mode & MODE_STOP_PROCESSING))
			attempt(&gg);
		if(0 == (gg.mode & MODE_SOLVED)) continue; //there is nothing to do with this wrong guess
		if(g->mode & MODE_ANY_SOLUTION) {
			g->mode = gg.mode; //mark the game as solved
			return; //exit on first success
		}
		if((mode & MODE_SOLVED) //secondary success in this loop
			|| (gg.mode & MODE_MULTIPLE_SOLUTONS)) { //mutiple solutions in the depth
			g->mode = gg.mode | MODE_MULTIPLE_SOLUTONS; //mark the game as solved
			return; //stop on second solution
		}
		mode = gg.mode; //at least one solution
		//Note that the next guess will destroy the content of g->cellDigits.
	}
	g->mode |= mode;
}

//Convert the solution to ASCII and write to the buffer.
static void writeBack(const game* g, char* out)
{
	for(int i = 0; i < 81; i++)
		out[i] = (char)(Bitmap2Digit[(unsigned char)g->cellDigits[i]] + '0');
}

//Set the initially known digits.
//The context is updated on 2 passes.
static void init(game* g, const char* in)
{
	bitmap *gkd = g->groupKnownDigits;
	bitmap *cp = g->cellPossibilities;
	//First pass: set the digits w/o updating the affected cells.
	for(int i = 0; i < 81; i++) {
		if(in[i] == 0) continue;
		//...additional input checking here...
		//setDigit(g, i, Digit2Bitmap[in[i]]);
		//...check for error here if this is the goal...

		bitmap bm = Digit2Bitmap[in[i]];
		const cellIndex *ag = affectedGroups[i];
		//if the digit we are setting has been previously set for one of
		//the 3 groups the cell is member of, we are in wrong way
		if(bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]])) {
			g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
			return;
		}
		cp[i] = 0; //clear the mask to exclude this cell from further processing

		//if we need the final solution, store the digit for printing
		if(0 == (g->mode & MODE_NO_OUTPUT))
			g->cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.

		if(0 == --g->cellsLeft) {//solved
			g->mode |= (MODE_SOLVED | MODE_STOP_PROCESSING);
			return;
		}

		//set the digit as solved for the all 3 groups
		gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;
	}
	//Second pass: update the affected cells.
	//Overall performance improvement is about 9% compared to setDigit calls.
	for(int i = 0; i < 81; i++) {
		bitmap *bm = &cp[i];
		//if(*bm == 0) continue;
		const cellIndex *ag = affectedGroups[i];
		bitmap knowns = gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]];
		if(*bm & knowns) { //there are possibilities to be cleared
			*bm &= ~knowns;
			if(*bm == 0) { //no possibilities for this cell
				g->mode |= (MODE_WRONG_GUESS | MODE_STOP_PROCESSING);
				return;
			}
			if(Bitmap2Digit[*bm]) { //single digit?
				setDigit(g, i, *bm);
				if(g->mode & MODE_STOP_PROCESSING)
					return;
			}
		}
	}
}

//Attempt solving the game
//Parameters:
// in - the input buffer, char[81] with values from 0x00 to 0x09
// out - the output buffer, char[81] with values from '1' to '9'. May be NULL.
// mode - 0=find first solution and write to the buffer. Return 0 or 1.
//        1=search for second solution and do not write. Return 0, 1, or 2.
//Return values:
// -1 - invalid parameters.
//  0 - no solution.
//  1 - some solution found in mode=0 or there is only one solution in mode 1.
//  2 - second solution found. No write back.
//The in and out buffers may overlap or be the same buffer.
extern int solve(const char* in, char* out, const int mode=0)
{
	if(0 == in) return -1; //wrong parameters
	game g = defaultGame; //clone the empty game
	switch(mode) {
		case 0: //find first solution and write it back if the out buffer is not NULL
			g.mode |= MODE_ANY_SOLUTION;
			if(!out) g.mode |= MODE_NO_OUTPUT; //do not copy back the solution
			break;
		case 1: //search for second solution; do not write back the result
			g.mode |= MODE_NO_OUTPUT; //do not copy back the solution
			break;
		default:
			return -1; //wrong parameters
	}
	cellDigit cellDigits[81]; //0==9, uninitialized
	g.cellDigits = cellDigits;
	init(&g, in); //set all known digits
	attempt(&g); //do the job
	if (g.mode & MODE_SOLVED) {
		if(0 == (g.mode & MODE_NO_OUTPUT))
			writeBack(&g, out);
		if(g.mode & MODE_MULTIPLE_SOLUTONS)
			return 2; //2 or more solutions
		return 1; //success or exactly one solution
	}
	return 0; //no solution
}

/*******************
 * File sudoku.cpp *
 *******************/

/***************************************************************************
A Fast Simple Sudoku Solver by Mladen Dobrichev

This is a simple file created to demonstrate:
1. Usage of "solve" function defined in file game.cpp.
2. Parallel processing using OpemMP package. (/Qopenmp /Qopenmp-link:static)
The file does not share either preprocess directives nor global variables
with game.cpp, which is the actual solver.
It has been tested only with MS VS 2005.

For commercial users the actions allowed to this file are limited to erasing.
No limitations to other users, but citing somewhere my name is welcome.

Compile with: icl.exe
 /c /O2 /Ob1 /Oi /Ot /Oy /MT /GS- /arch:SSE2 /fp:fast /GR- /Fo"Release/" /W3 /Wp64 /Qopenmp /Qopenmp-link:static

Link with: xilink.exe
 /INCREMENTAL:NO /MANIFEST:NO /TLBID:1 /SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF
***************************************************************************/

//supress some warnings for fopen
#define _CRT_SECURE_NO_DEPRECATE 1

#include <stdio.h>
#include <time.h>

#define CHUNK_SIZE	2048 //this consumes stack

int main(int argc, char* argv[])
{
	FILE *src, *res = NULL;
	clock_t start, finish;
	char line[CHUNK_SIZE][84], obuf[CHUNK_SIZE][83], *ifname, *ofname = NULL;
	int nExamples = 0, mode = 0, nSolved = 0, pos, i, chunkSolved;

	switch(argc) {
		case 3:
			if(*argv[2] == '*')
				mode = 1;
			else
				ofname = argv[2];
		case 2:
			ifname = argv[1];
			break;
		default:
			printf("A Fast Simple Sudoku Solver by Ml. Dobrichev, v8.2, %s\n\nUsage:\n%s infile [outfile | *]\n\n", __TIMESTAMP__, argv[0]);
			printf("infile is an ASCII file; 81 characters per row.\nUse characters 1 to 9 for known digits, and any other for the rest.\n");
			printf("* checks for second solution.\n");
			return 0;
	}

	if((src = fopen(ifname, "r")) == NULL) {
		printf("Error opening the file %s\n", ifname );
		return 1;
	}
	if(ofname) {
		if((res = fopen(ofname, "w+")) == NULL) {
			printf("Error creating the file %s\n", ofname);
			fclose(src);
			return 1;
		}
		for(i = 0; i < CHUNK_SIZE; i++) {
			obuf[i][81] = 0x0a;
			obuf[i][82] = 0;
		}
	}

	finish = start = clock();
	while(finish != -1 && finish == (start = clock())); //start from the edge of the clock cycle

	pos = 0;
	while(fgets(line[pos], 84, src)) {
		nExamples++;
		pos++;
		if(pos == CHUNK_SIZE) {
			pos = 0;
			chunkSolved = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(static, 1) reduction(+:chunkSolved)
#endif //_OPENMP
			for(int i = 0; i < CHUNK_SIZE; i++) {
				for (char *t = line[i]; t < &line[i][81]; t++) {
					char cc = (char)(*t - '0');
					*t = ((cc >= 0 && cc <= 9) ? cc : (char)0);
				}
				obuf[i][0] = 0x0a;
				obuf[i][1] = 0;
				if(solve(line[i], obuf[i], mode) == 1) {
					chunkSolved++;
				}
			}
			nSolved += chunkSolved;
			if(res) {
				for(int i = 0; i < CHUNK_SIZE; i++) {
					fputs(obuf[i], res);
				}
			}
		}
	}
	if(pos) {
		chunkSolved = 0;
#ifdef _OPENMP
#pragma omp parallel for schedule(static, 1) reduction(+:chunkSolved)
#endif //_OPENMP
		for(int i = 0; i < pos; i++) {
			for (char *t = line[i]; t < &line[i][81]; t++) {
				char cc = (char)(*t - '0');
				*t = ((cc >= 0 && cc <= 9) ? cc : (char)0);
			}
			obuf[i][0] = 0x0a;
			obuf[i][1] = 0;
			if(solve(line[i], obuf[i], mode) == 1) {
				chunkSolved++;
			}
		}
		nSolved += chunkSolved;
		if(res) {
			for(int i = 0; i < pos; i++) {
				fputs(obuf[i], res);
			}
		}
	}

	finish = clock();

	fclose(src);
	if(res) fclose(res);

	printf( "\n%i of %i puzzles solved in %2.3f seconds (%3.2f microseconds/puzzle)\n",
		nSolved,
		nExamples,
		(double)(finish - start) / CLOCKS_PER_SEC,
		(double)(finish - start) / CLOCKS_PER_SEC * 1000000 / nExamples);
	return 0;
}

