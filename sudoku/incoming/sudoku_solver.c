/************************************************************************************/
/*                                                                                  */
/* Author: Bill DuPree                                                              */
/* Name: sudoku_solver.c                                                            */
/* Language: C                                                                      */
/* Date: Feb. 25, 2006                                                              */
/* Copyright (C) Feb. 25, 2006, All rights reserved.                                */
/*                                                                                  */
/* This is a program that solves Su Doku (aka Sudoku, Number Place, etc.) puzzles   */
/* primarily using deductive logic. It will only resort to trial-and-error and      */
/* backtracking approaches upon exhausting all of its deductive moves.              */
/*                                                                                  */
/* Puzzles must be of the standard 9x9 variety using the (ASCII) characters '1'     */
/* through '9' for the puzzle solution set. Puzzles should be submitted as 81       */
/* character strings which, when read left-to-right will fill a 9x9 Sudoku grid     */
/* from left-to-right and top-to-bottom. In the puzzle specification, the           */
/* characters 1 - 9 represent the puzzle "givens" or clues. Any other non-blank     */
/* character represents an unsolved cell.                                           */
/*                                                                                  */
/* The puzzle solving algorithm is "home grown." I did not borrow any of the usual  */
/* techniques from the literature, e.g. Donald Knuth's "Dancing Links." Instead     */
/* I "rolled my own" from scratch. As such, its performance can only be blamed      */
/* on yours truly. Still, I feel it is quite fast. On a 333 MHz Pentium II Linux    */
/* box it solves typical medium force puzzles in approximately 800 microseconds or  */
/* about 1,200 puzzles per second, give or take. On an Athlon XP 3000 (Barton core) */
/* it solves about 6,600 puzzles per sec.                                           */
/*                                                                                  */
/* DESCRIPTION OF ALGORITHM:                                                        */
/*                                                                                  */
/* The puzzle algorithm initially assumes every unsolved cell can assume every      */
/* possible value. It then uses the placement of the givens to refine the choices   */
/* available to each cell. I call this the markup phase.                            */
/*                                                                                  */
/* After markup completes, the algorithm then looks for "singleton" cells with      */
/* values that, due to constraints imposed by the row, column, or 3x3 region, may   */
/* only assume one possible value. Once these cells are assigned values, the        */
/* algorithm returns to the markup phase to apply these changes to the remaining    */
/* candidate solutions. The markup/singleton phases alternate until either no more  */
/* changes occur, or the puzzle is solved. I call the markup/singleton elimination  */
/* loop the "Simple Solver" because in a large percentage of cases it solves the    */
/* puzzle.                                                                          */
/*                                                                                  */
/* If the simple solver portion of the algorithm doesn't produce a solution, then   */
/* more advanced deductive rules are applied. I've implemented two additional rules */
/* as part of the deductive puzzle solver. The first is subset elimination wherein  */
/* a row/column/region is scanned for X number of cells with X number of matching   */
/* candidate solutions. If such subsets are found in the row, column, or region,    */
/* then the candidates values from the subset may be eliminated from all other      */
/* unsolved cells within the row, column, or region, respectively.                  */
/*                                                                                  */
/* The second advanced deductive rule examines each region looking for candidate    */
/* values that exclusively align themselves along a single row or column, i.e. a    */
/* a vector. If such candidate values are found, then they may be eliminated from   */
/* the cells outside of the region that are part of the aligned row or column.      */
/*                                                                                  */
/* Note that each of the advanced deductive rules calls all preceeding rules, in    */
/* order, if that advanced rule has effected a change in puzzle markup.             */
/*                                                                                  */
/* Finally, if no solution is found after iteratively applying all deductive rules, */
/* then we begin trial-and-error using recursion for backtracking. A working copy   */
/* is created from our puzzle, and using this copy the first cell with the          */
/* smallest number of candidate solutions is chosen. One of the solutions values is */
/* assigned to that cell, and the solver algorithm is called using this working     */
/* copy as its starting point. Eventually, either a solution, or an impasse is      */
/* reached.                                                                         */
/*                                                                                  */
/* If we reach an impasse, the recursion unwinds and the next trial solution is     */
/* attempted. If a solution is found (at any point) the values for the solution are */
/* added to a list. Again, so long as we are examining all possibilities, the       */
/* recursion unwinds so that the next trial may be attempted. It is in this manner  */
/* that we enumerate puzzles with multiple solutions.                               */
/*                                                                                  */
/* Note that it is certainly possible to add to the list of applied deductive       */
/* rules. The techniques known as "X-Wing" and "Swordfish" come to mind. On the     */
/* other hand, adding these additional rules will, in all likelihood, slow the      */
/* solver down by adding to the computational burden while producing very few       */
/* results. I've seen the law of diminishing returns even in some of the existing   */
/* rules, e.g. in subset elimination I only look at two and three valued subsets    */
/* because taking it any further than that degraded performance.                    */
/*                                                                                  */
/* PROGRAM INVOCATION:                                                              */
/*                                                                                  */
/* This program is a console (or command line) based utility and has the following  */
/* usage:                                                                           */
/*                                                                                  */
/*      sudoku_solver {-p puzzle | -f <puzzle_file>} [-o <outfile>]                 */
/*              [-r <reject_file>] [-1][-a][-c][-g][-l][-m][-n][-s]                 */
/*                                                                                  */
/* where:                                                                           */
/*                                                                                  */
/*        -1      Search for first solution, otherwise all solutions are returned   */
/*        -a      Requests that the answer (solution) be printed                    */
/*        -c      Print a count of solutions for each puzzle                        */
/*        -d      Print the recursive trial depth required to solve the puzzle      */
/*        -e      Print a step-by-step explanation of the solution(s)               */
/*        -f      Takes an argument which specifes an input file                    */
/*                containing one or more unsolved puzzles (default: stdin)          */
/*        -G      Print the puzzle solution(s) in a 9x9 grid format                 */
/*        -g      Print the number of given clues                                   */
/*        -l      Print the recursive trial depth required to solve the puzzle      */
/*        -m      Print an octal mask for the puzzle givens                         */
/*        -n      Number each result                                                */
/*        -o      Specifies an output file for the solutions (default: stdout)      */
/*        -p      Takes an argument giving a single inline puzzle to be solved      */
/*        -r      Specifies an output file for unsolvable puzzles                   */
/*                (default: stderr)                                                 */
/*        -s      Print the puzzle's score or difficulty rating                     */
/*        -?      Print usage information                                           */
/*                                                                                  */
/* The return code is zero if all puzzles had unique solutions,                     */
/* (or have one or more solutions when -1 is specified) and non-zero                */
/* when no unique solution exists.                                                  */
/*                                                                                  */
/* PUZZLE SCORING                                                                   */
/*                                                                                  */
/* A word about puzzle scoring, i.e. rating a puzzle's difficulty, is in order.     */
/* Rating Sudoku puzzles is a rather subjective thing, and thus it is difficult to  */
/* really develop an objective puzzle rating system. I, however, have attempted     */
/* this feat (several times with varying degrees of success ;-) and I think the     */
/* heuristics I'm currently applying aren't too bad for rating the relative         */
/* difficulty of solving a puzzle.                                                  */
/*                                                                                  */
/* The following is a brief rundown of how it works. The initial puzzle markup is   */
/* a "free" operation, i.e. no points are scored for the first markup pass. I feel  */
/* this is appropriate because a person solving a puzzle will always have to do     */
/* their own eyeballing and scanning of the puzzle. Subsequent passes are           */
/* scored at one point per candidate eliminated because these passes indicate       */
/* that more deductive work is required. Secondly, the "reward" for solving a cell  */
/* is set to one point, and as long as the solution only requires simple markup     */
/* and elimination of singletons, this level of reward remains unchanged.           */
/*                                                                                  */
/* This reward changes, however, when advanced solving rules are required. Puzzles  */
/* that remain unsolved after the first pass through the simple solver phase have   */
/* a higher "reward", i.e. it is incremented by two. Thus, if subset or vector      */
/* elimination is required, all subsequently solved cells score higher bounties.    */
/* In addition, the successful application of these deductive techniques score      */
/* their own penalties.                                                             */
/*                                                                                  */
/* Finally, if a trial-and-error approach is called for, then the "reward" is       */
/* incremented by another five points. Thus, the total penalty for each level of    */
/* recursion is an additional seven points per solved cell, i.e.                    */
/* (recursive_depth * 7) + 1 points per solved cell. Trial solutions are also       */
/* penalized by a weighting factor that is based upon the number of unsolved cells  */
/* that remain upon reentry to the solver and the depth of recursion. (I've seen a  */
/* pathological puzzle from the "Minimum Sudoku" web site require 16 levels of      */
/* recursion and score a whopping 228,642 points using this scoring system!)        */
/*                                                                                  */
/* And that brings me to this topic: What do all these points mean?                 */
/*                                                                                  */
/* Well, who knows? This is still subjective, and the weighting system I've chosen  */
/* for point scoring is is largely arbitrary. But based upon feedback from a number */
/* of individuals, a rough scale of difficulty plays out as follows:                */
/*                                                                                  */
/*   DEGREE OF DIFFICULTY   |  SCORE                                                */
/* -------------------------+------------------------------------------             */
/*   TRIVIAL                |  80 points or less                                    */
/*   EASY                   |  81 - 150 points                                      */
/*   MEDIUM                 |  151 - 250 points                                     */
/*   HARD                   |  251 - 400 points                                     */
/*   VERY HARD              |  401 - 900 points                                     */
/*   DIABOLICAL             |  901 and up                                           */
/*                                                                                  */
/* Experience shows that puzzles in the HARD category, in a few cases, will         */
/* require a small amount of trial-and-error. The VERY HARD puzzles will likely     */
/* require trial-and-error, and in some cases more than one level of trial-and-     */
/* error. As for the DIABOLICAL puzzles--why waste your time? These are best left   */
/* to masochists, savants and automated solvers. YMMV.                              */
/*                                                                                  */
/* LICENSE:                                                                         */
/*                                                                                  */
/* This program is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by             */
/* the Free Software Foundation; either version 2 of the License, or                */
/* (at your option) any later version.                                              */
/*                                                                                  */
/* This program is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    */
/* GNU General Public License for more details.                                     */
/*                                                                                  */
/* You should have received a copy of the GNU General Public License                */
/* along with this program; if not, write to the Free Software                      */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       */
/*                                                                                  */
/* CONTACT:                                                                         */
/*                                                                                  */
/* Email: bdupree@techfinesse.com                                                   */
/* Post: Bill DuPree, 609 Wenonah Ave, Oak Park, IL 60304 USA                       */
/*                                                                                  */
/************************************************************************************/
/*                                                                                  */
/* CHANGE LOG:                                                                      */
/*                                                                                  */
/* Rev.	  Date        Init.	Description                                         */
/* -------------------------------------------------------------------------------- */
/* 1.00   2006-02-25  WD	Initial version.                                    */
/* 1.01   2006-03-13  WD	Fixed return code calc. Added signon message.       */
/* 1.10   2006-03-20  WD        Added explain option, add'l speed optimizations     */
/* 1.11   2006-03-23  WD        More simple speed optimizations, cleanup, bug fixes */
/*                                                                                  */
/************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define VERSION "1.11"

#define PUZZLE_ORDER 3
#define PUZZLE_DIM (PUZZLE_ORDER*PUZZLE_ORDER)
#define PUZZLE_CELLS (PUZZLE_DIM*PUZZLE_DIM)

/* Command line options */
#ifdef EXPLAIN
#define OPTIONS "?1acdef:Ggmno:p:r:s"
#else
#define OPTIONS "?1acdf:Ggmno:p:r:s"
#endif
extern char *optarg;
extern int optind, opterr, optopt;

static char *myname;    /* Name that we were invoked under */

static FILE *solnfile, *rejects;

/* This is the list of cell coordinates specified on a row basis */

static int const row[PUZZLE_DIM][PUZZLE_DIM] = {
 {  0,  1,  2,  3,  4,  5,  6,  7,  8 },
 {  9, 10, 11, 12, 13, 14, 15, 16, 17 },
 { 18, 19, 20, 21, 22, 23, 24, 25, 26 },
 { 27, 28, 29, 30, 31, 32, 33, 34, 35 },
 { 36, 37, 38, 39, 40, 41, 42, 43, 44 },
 { 45, 46, 47, 48, 49, 50, 51, 52, 53 },
 { 54, 55, 56, 57, 58, 59, 60, 61, 62 },
 { 63, 64, 65, 66, 67, 68, 69, 70, 71 },
 { 72, 73, 74, 75, 76, 77, 78, 79, 80 }};

/* This is the list of cell coordinates specified on a column basis */

static int const col[PUZZLE_DIM][PUZZLE_DIM] = {
 {  0,  9, 18, 27, 36, 45, 54, 63, 72 },
 {  1, 10, 19, 28, 37, 46, 55, 64, 73 },
 {  2, 11, 20, 29, 38, 47, 56, 65, 74 },
 {  3, 12, 21, 30, 39, 48, 57, 66, 75 },
 {  4, 13, 22, 31, 40, 49, 58, 67, 76 },
 {  5, 14, 23, 32, 41, 50, 59, 68, 77 },
 {  6, 15, 24, 33, 42, 51, 60, 69, 78 },
 {  7, 16, 25, 34, 43, 52, 61, 70, 79 },
 {  8, 17, 26, 35, 44, 53, 62, 71, 80 }};

/* This is the list of cell coordinates specified on a 3x3 region basis */

static int const region[PUZZLE_DIM][PUZZLE_DIM] = {
 {  0,  1,  2,  9, 10, 11, 18, 19, 20 },
 {  3,  4,  5, 12, 13, 14, 21, 22, 23 },
 {  6,  7,  8, 15, 16, 17, 24, 25, 26 },
 { 27, 28, 29, 36, 37, 38, 45, 46, 47 },
 { 30, 31, 32, 39, 40, 41, 48, 49, 50 },
 { 33, 34, 35, 42, 43, 44, 51, 52, 53 },
 { 54, 55, 56, 63, 64, 65, 72, 73, 74 },
 { 57, 58, 59, 66, 67, 68, 75, 76, 77 },
 { 60, 61, 62, 69, 70, 71, 78, 79, 80 }};

/* Flags for cellflags member */
#define GIVEN 1
#define FOUND 2
#define STUCK 3

/* Return codes for funcs that modify puzzle markup */
#define NOCHANGE 0
#define CHANGE   1

typedef struct grd {
	short cellflags[PUZZLE_CELLS];
        short solved[PUZZLE_CELLS];
	short cell[PUZZLE_CELLS];
        short tail, givens, exposed, maxlvl, inc, reward;
        unsigned int score, solncount;
        struct grd *next;
} grid;

typedef int (*return_soln)(grid *g);

static grid *soln_list = NULL;

typedef struct {
	short row, col, region;
} cellmap;

/* Array structure to help map cell index back to row, column, and region */
static cellmap const map[PUZZLE_CELLS] = {
   { 0, 0, 0 },
   { 0, 1, 0 },
   { 0, 2, 0 },
   { 0, 3, 1 },
   { 0, 4, 1 },
   { 0, 5, 1 },
   { 0, 6, 2 },
   { 0, 7, 2 },
   { 0, 8, 2 },
   { 1, 0, 0 },
   { 1, 1, 0 },
   { 1, 2, 0 },
   { 1, 3, 1 },
   { 1, 4, 1 },
   { 1, 5, 1 },
   { 1, 6, 2 },
   { 1, 7, 2 },
   { 1, 8, 2 },
   { 2, 0, 0 },
   { 2, 1, 0 },
   { 2, 2, 0 },
   { 2, 3, 1 },
   { 2, 4, 1 },
   { 2, 5, 1 },
   { 2, 6, 2 },
   { 2, 7, 2 },
   { 2, 8, 2 },
   { 3, 0, 3 },
   { 3, 1, 3 },
   { 3, 2, 3 },
   { 3, 3, 4 },
   { 3, 4, 4 },
   { 3, 5, 4 },
   { 3, 6, 5 },
   { 3, 7, 5 },
   { 3, 8, 5 },
   { 4, 0, 3 },
   { 4, 1, 3 },
   { 4, 2, 3 },
   { 4, 3, 4 },
   { 4, 4, 4 },
   { 4, 5, 4 },
   { 4, 6, 5 },
   { 4, 7, 5 },
   { 4, 8, 5 },
   { 5, 0, 3 },
   { 5, 1, 3 },
   { 5, 2, 3 },
   { 5, 3, 4 },
   { 5, 4, 4 },
   { 5, 5, 4 },
   { 5, 6, 5 },
   { 5, 7, 5 },
   { 5, 8, 5 },
   { 6, 0, 6 },
   { 6, 1, 6 },
   { 6, 2, 6 },
   { 6, 3, 7 },
   { 6, 4, 7 },
   { 6, 5, 7 },
   { 6, 6, 8 },
   { 6, 7, 8 },
   { 6, 8, 8 },
   { 7, 0, 6 },
   { 7, 1, 6 },
   { 7, 2, 6 },
   { 7, 3, 7 },
   { 7, 4, 7 },
   { 7, 5, 7 },
   { 7, 6, 8 },
   { 7, 7, 8 },
   { 7, 8, 8 },
   { 8, 0, 6 },
   { 8, 1, 6 },
   { 8, 2, 6 },
   { 8, 3, 7 },
   { 8, 4, 7 },
   { 8, 5, 7 },
   { 8, 6, 8 },
   { 8, 7, 8 },
   { 8, 8, 8 }
};

static const short symtab[1<<PUZZLE_DIM] = {
	'.','1','2','.','3','.','.','.','4','.','.','.','.','.','.','.','5','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'6','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'7','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'8','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'9','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.'};

static int enumerate_all = 1;
static int lvl = 0;

#ifdef EXPLAIN
static int explain = 0;
#endif

/* Function prototype(s) */
static int mult_elimination(grid *g);
static void print_grid(char *sud, FILE *h);
static char *format_answer(grid *g, char *outbuf);
static void diagnostic_grid(grid *g, FILE *h);

static inline int is_given(int c) { return (c >= '1') && (c <= '9'); }

#if defined(DEBUG)
static void mypause()
{
	char buf[8];
        printf("\tPress enter -> ");
        fgets(buf, 8, stdin);
}
#endif

#if 0
/* Generic (and slow) bitcount function */
static int bitcount(short cell)
{
	int i, count, mask;

        mask = 1;
        for (i = count = 0; i < 16; i++) {
        	if (mask & cell) count++;
                mask <<= 1;
        }
        return count;
}
#endif

/*****************************************************/
/* Return the number of '1' bits in a cell.          */
/* Rather than count bits, do a quick table lookup.  */
/* Warning: Only valid for 9 low order bits.         */
/*****************************************************/
 
static inline short bitcount(short cell)
{
        static const short bcounts[512] = {
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
        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,5,6,6,7,6,7,7,8,6,7,7,8,7,8,8,9};

        return bcounts[cell];
}

#ifdef EXPLAIN

/**************************************************/
/* Indent two spaces for each level of recursion. */
/**************************************************/
static inline void explain_indent(FILE *h)
{
	int i;

        for (i = 0; i < lvl-1; i++) fprintf(h, "  ");
}

/******************************************************************/
/* Construct a string representing the possible values a cell may */
/* contain according to current markup.                           */
/******************************************************************/
static char *clues(short cell)
{
	int i, m, multi, mask;
	static char buf[64], *p;

        multi = m = bitcount(cell);

        if (!multi) return "NULL";

	if (multi > 1) {
        	strcpy(buf, "tuple (");
        }
        else {
        	strcpy(buf, "value ");
        }

        p = buf + strlen(buf);

        for (mask = i = 1; i <= PUZZLE_DIM; i++) {
        	if (mask & cell) {
                	*p++ = symtab[mask];
                        multi -= 1;
                        if (multi) { *p++ = ','; *p++ = ' '; }
                }
                mask <<= 1;
        }
        if (m > 1) *p++ = ')';
	*p = 0;
        return buf;
}

/*************************************************************/
/* Explain removal of a candidate value from a changed cell. */
/*************************************************************/
static void explain_markup_elim(grid *g, int chgd, int clue)
{
	int chgd_row, chgd_col, clue_row, clue_col;

        chgd_row = map[chgd].row+1;
        chgd_col = map[chgd].col+1;
        clue_row = map[clue].row+1;
        clue_col = map[clue].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Candidate %s removed from row %d, col %d because of cell at row %d, col %d\n",
                clues(g->cell[clue]), chgd_row, chgd_col, clue_row, clue_col);
}

/*****************************************/
/* Dump the state of the current markup. */
/*****************************************/
static void explain_current_markup(grid *g)
{
	if (g->exposed >= PUZZLE_CELLS) return;

        fprintf(solnfile, "\n");
        explain_indent(solnfile);
	fprintf(solnfile, "Current markup is as follows:");
        diagnostic_grid(g, solnfile);
        fprintf(solnfile, "\n");
}

/****************************************/
/* Explain the solving of a given cell. */
/****************************************/
static void explain_solve_cell(grid *g, int chgd)
{
	int chgd_row, chgd_col;

        chgd_row = map[chgd].row+1;
        chgd_col = map[chgd].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Cell at row %d, col %d solved with %s\n",
                chgd_row, chgd_col, clues(g->cell[chgd]));
}

/******************************************************************/
/* Explain the current impasse reached during markup elimination. */
/******************************************************************/
static void explain_markup_impasse(grid *g, int chgd, int clue)
{
	int chgd_row, chgd_col, clue_row, clue_col;

        chgd_row = map[chgd].row+1;
        chgd_col = map[chgd].col+1;
        clue_row = map[clue].row+1;
        clue_col = map[clue].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Impasse for cell at row %d, col %d because cell at row %d, col %d removes last candidate\n",
                chgd_row, chgd_col, clue_row, clue_col);
        explain_current_markup(g);
}

/****************************************/
/* Explain naked and/or hidden singles. */
/****************************************/
static void explain_singleton(grid *g, int chgd, int mask, char *vdesc)
{
	int chgd_row, chgd_col, chgd_reg;

        chgd_row = map[chgd].row+1;
        chgd_col = map[chgd].col+1;
        chgd_reg = map[chgd].region+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Cell of region %d at row %d, col %d will only solve for %s in this %s\n",
                chgd_reg, chgd_row, chgd_col, clues(mask), vdesc);
        explain_solve_cell(g, chgd);
}

/*********************************/
/* Explain initial puzzle state. */
/*********************************/
static void explain_markup()
{
        fprintf(solnfile, "\n");
        explain_indent(solnfile);
	fprintf(solnfile, "Assume all cells may contain any values in the range: [1 - 9]\n");
}

/************************/
/* Explain given clues. */
/************************/
static void explain_given(int cell, char val)
{
	int cell_row, cell_col;

        cell_row = map[cell].row+1;
        cell_col = map[cell].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Cell at row %d, col %d is given clue value %c\n", cell_row, cell_col, val);
}

/*******************************************/
/* Explain region/row/column interactions. */
/*******************************************/
static void explain_vector_elim(char *desc, int i, int cell, int val, int region)
{
	int cell_row, cell_col;

        cell_row = map[cell].row+1;
        cell_col = map[cell].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Candidate %s removed from cell at row %d, col %d because it aligns along %s %d in region %d\n",
                clues(val), cell_row, cell_col, desc, i+1, region+1);
}

/******************************************************************/
/* Explain the current impasse reached during vector elimination. */
/******************************************************************/
static void explain_vector_impasse(grid *g, char *desc, int i, int cell, int val, int region)
{
	int cell_row, cell_col;

        cell_row = map[cell].row+1;
        cell_col = map[cell].col+1;

        explain_indent(solnfile);
        fprintf(solnfile, "Impasse at cell at row %d, col %d because candidate %s aligns along %s %d in region %d\n",
                cell_row, cell_col, clues(val), desc, i+1, region+1);
        explain_current_markup(g);
}

/*****************************************************************/
/* Explain the current impasse reached during tuple elimination. */
/*****************************************************************/
static void explain_tuple_impasse(grid *g, char *desc, int elt, int tuple, int count, int bits)
{
        explain_indent(solnfile);
        fprintf(solnfile, "Impasse in %s %d because too many (%d) cells have %d-valued %s\n",
                desc, elt+1, count, bits, clues(tuple));
        explain_current_markup(g);
}

/*********************************************************************/
/* Explain the removal of a tuple of candidate solutions from a cell */
/*********************************************************************/
static void explain_tuple_elim(char *desc, int elt, int tuple, int cell)
{
        explain_indent(solnfile);
        fprintf(solnfile, "Values of %s in %s %d removed from cell at row %d, col %d\n",
                clues(tuple), desc, elt+1, map[cell].row+1, map[cell].col+1);

}

/**************************************************/
/* Indicate that a viable solution has been found */
/**************************************************/
static void explain_soln_found(grid *g)
{
	char buf[90];

        fprintf(solnfile, "\n");
        explain_indent(solnfile);
        fprintf(solnfile, "Solution found: %s\n", format_answer(g, buf));
        print_grid(buf, solnfile);
        fprintf(solnfile, "\n");
}

/***************************/
/* Show the initial puzzle */
/***************************/
static void explain_grid(grid *g)
{
	char buf[90];

        fprintf(solnfile, "Initial puzzle: %s\n", format_answer(g, buf));
        print_grid(buf, solnfile);
        explain_current_markup(g);
        fprintf(solnfile, "\n");
}

/*************************************************/
/* Explain attempt at a trial and error solution */
/*************************************************/
static void explain_trial(int cell, int value)
{
        explain_indent(solnfile);
        fprintf(solnfile, "Attempt trial where cell at row %d, col %d is assigned value %s\n",
                map[cell].row+1, map[cell].col+1, clues(value));
}

/**********************************************/
/* Explain back out of current trial solution */
/**********************************************/
static void explain_backtrack()
{
	if (lvl <= 1) return;

        explain_indent(solnfile);
        fprintf(solnfile, "Backtracking\n\n");
}

#define EXPLAIN_MARKUP                                 if (explain) explain_markup()
#define EXPLAIN_CURRENT_MARKUP(g)                      if (explain) explain_current_markup((g))
#define EXPLAIN_GIVEN(cell, val)	               if (explain) explain_given((cell), (val))
#define EXPLAIN_MARKUP_ELIM(g, chgd, clue)             if (explain) explain_markup_elim((g), (chgd), (clue))
#define EXPLAIN_MARKUP_SOLVE(g, cell)                  if (explain) explain_solve_cell((g), (cell)) 
#define EXPLAIN_MARKUP_IMPASSE(g, chgd, clue)          if (explain) explain_markup_impasse((g), (chgd), (clue))
#define EXPLAIN_SINGLETON(g, chgd, mask, vdesc)        if (explain) explain_singleton((g), (chgd), (mask), (vdesc))
#define EXPLAIN_VECTOR_ELIM(desc, i, cell, v, r)       if (explain) explain_vector_elim((desc), (i), (cell), (v), (r))
#define EXPLAIN_VECTOR_IMPASSE(g, desc, i, cell, v, r) if (explain) explain_vector_impasse((g), (desc), (i), (cell), (v), (r))
#define EXPLAIN_VECTOR_SOLVE(g, cell)                  if (explain) explain_solve_cell((g), (cell)) 
#define EXPLAIN_TUPLE_IMPASSE(g, desc, j, c, count, i) if (explain) explain_tuple_impasse((g), (desc), (j), (c), (count), (i))
#define EXPLAIN_TUPLE_ELIM(desc, j, c, cell)           if (explain) explain_tuple_elim((desc), (j), (c), (cell))
#define EXPLAIN_TUPLE_SOLVE(g, cell)                   if (explain) explain_solve_cell((g), (cell)) 
#define EXPLAIN_SOLN_FOUND(g)			       if (explain) explain_soln_found((g));
#define EXPLAIN_GRID(g)			               if (explain) explain_grid((g));
#define EXPLAIN_TRIAL(cell, val)		       if (explain) explain_trial((cell), (val));
#define EXPLAIN_BACKTRACK                              if (explain) explain_backtrack();
#define EXPLAIN_INDENT(h)			       if (explain) explain_indent((h))

#else

#define EXPLAIN_MARKUP
#define EXPLAIN_CURRENT_MARKUP(g)
#define EXPLAIN_GIVEN(cell, val)
#define EXPLAIN_MARKUP_ELIM(g, chgd, clue)
#define EXPLAIN_MARKUP_SOLVE(g, cell) 
#define EXPLAIN_MARKUP_IMPASSE(g, chgd, clue)
#define EXPLAIN_SINGLETON(g, chgd, mask, vdesc);
#define EXPLAIN_VECTOR_ELIM(desc, i, cell, v, r)
#define EXPLAIN_VECTOR_IMPASSE(g, desc, i, cell, v, r)
#define EXPLAIN_VECTOR_SOLVE(g, cell)
#define EXPLAIN_TUPLE_IMPASSE(g, desc, j, c, count, i)
#define EXPLAIN_TUPLE_ELIM(desc, j, c, cell)
#define EXPLAIN_TUPLE_SOLVE(g, cell)
#define EXPLAIN_SOLN_FOUND(g)
#define EXPLAIN_GRID(g)
#define EXPLAIN_TRIAL(cell, val)
#define EXPLAIN_BACKTRACK
#define EXPLAIN_INDENT(h)

#endif


/*****************************************************/
/* Initialize a grid to an empty state.              */
/* At the start, all cells can have any value        */
/* so set all 9 lower order bits in each cell.       */
/* In effect, the 9x9 grid now has markup that       */
/* specifies that each cell can assume any value     */
/* of 1 through 9.                                   */
/*****************************************************/

static void init_grid(grid *g)
{
	int i;

        for (i = 0; i < PUZZLE_CELLS; i++) g->cell[i] = 0x01ff;
        memset(g->cellflags, 0, PUZZLE_CELLS*sizeof(g->cellflags[0]));
        g->exposed = 0;
        g->givens = 0;
        g->inc = 0;
        g->maxlvl = 0;
        g->score = 0;
        g->solncount = 0;
        g->reward = 1;
        g->next = NULL;
        g->tail = 0;
        EXPLAIN_MARKUP;
}

/*****************************************************/
/* Convert a puzzle from the input format,           */
/* i.e. a string of 81 non-blank characters          */
/* with ASCII digits '1' thru '9' specified          */
/* for the givens, and non-numeric characters        */
/* for the remaining cells. The string, read         */
/* left-to-right fills the 9x9 Sudoku grid           */
/* in left-to-right, top-to-bottom order.            */
/*****************************************************/

static void cvt_to_grid(grid *g, char *game)
{
	int i;

        init_grid(g);

        for (i = 0; i < PUZZLE_CELLS; i++) {
        	if (is_given(game[i])) {
			/* warning -- ASCII charset assumed */
                	g->cell[i] = 1 << (game[i] - '1');
                        g->cellflags[i] = GIVEN;
                        g->givens += 1;
                        g->solved[g->exposed++] = i;
                        EXPLAIN_GIVEN(i, game[i]);
                }
        }
        EXPLAIN_GRID(g);
}

/****************************************************************/
/* Print the partially solved puzzle and all associated markup  */
/* in 9x9 fashion.                                              */
/****************************************************************/

static void diagnostic_grid(grid *g, FILE *h)
{
	int i, j, flag;
        short c;
        char line1[40], line2[40], line3[40], cbuf1[5], cbuf2[5], cbuf3[5], outbuf[PUZZLE_CELLS+1];

	/* Sanity check */
	for (flag = 1, i = 0; flag && i < PUZZLE_CELLS; i++) {
        	if (bitcount(g->cell[i]) != 1) {
	                flag = 0;
                }
        }

        /* Don't need to print grid with diagnostic markup? */
        if (flag) {
                format_answer(g, outbuf);
        	print_grid(outbuf, h);
                fflush(h);
                return;
        }

        strcpy(cbuf1, "   |");
	strcpy(cbuf2, cbuf1);
	strcpy(cbuf3, cbuf1);
	fprintf(h, "\n");

        for (i = 0; i < PUZZLE_DIM; i++) {

        	*line1 = *line2 = *line3 = 0;

        	for (j = 0; j < PUZZLE_DIM; j++) {

                	c = g->cell[row[i][j]];

                        if (bitcount(c) == 1) {
			        strcpy(cbuf1, "   |");
				strcpy(cbuf2, cbuf1);
				strcpy(cbuf3, cbuf1);
                                cbuf2[1] = symtab[c];  
                        }
                        else {
                        	if (c & 1) cbuf1[0] = '*'; else cbuf1[0] = '.';
                                if (c & 2) cbuf1[1] = '*'; else cbuf1[1] = '.';
                                if (c & 4) cbuf1[2] = '*'; else cbuf1[2] = '.';
                        	if (c & 8) cbuf2[0] = '*'; else cbuf2[0] = '.';
                                if (c & 16) cbuf2[1] = '*'; else cbuf2[1] = '.';
                                if (c & 32) cbuf2[2] = '*'; else cbuf2[2] = '.';
                        	if (c & 64) cbuf3[0] = '*'; else cbuf3[0] = '.';
                                if (c & 128) cbuf3[1] = '*'; else cbuf3[1] = '.';
                                if (c & 256) cbuf3[2] = '*'; else cbuf3[2] = '.';
                        }

	                strcat(line1, cbuf1);
			strcat(line2, cbuf2);
			strcat(line3, cbuf3);
                }

		EXPLAIN_INDENT(h);
                fprintf(h, "+---+---+---+---+---+---+---+---+---+\n");
		EXPLAIN_INDENT(h);
                fprintf(h, "|%s\n", line1);
		EXPLAIN_INDENT(h);
		fprintf(h, "|%s\n", line2);
		EXPLAIN_INDENT(h);
		fprintf(h, "|%s\n", line3);
        }
	EXPLAIN_INDENT(h);
        fprintf(h, "+---+---+---+---+---+---+---+---+---+\n"); fflush(h);
}

/***********************************************************************/
/* Validate that a sudoku grid contains a valid solution. Return 1 if  */
/* true, 0 if false. If the verbose argument is non-zero, then print   */
/* reasons for invalidating the solution to stderr.                    */
/***********************************************************************/

static int validate(grid *g, int verbose)
{
	int i, j, regmask, rowmask, colmask, flag = 1;

	/* Sanity check */
	for (i = 0; i < PUZZLE_CELLS; i++) {
        	if (bitcount(g->cell[i]) != 1) {
                	if (verbose) {
				fprintf(rejects, "Cell %d at row %d, col %d has no unique soln.\n", 1+i, 1+map[i].row, 1+map[i].col); fflush(rejects);
	                	flag = 0;
                        } else return 0;
                }
        }

        /* Check rows */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	for (rowmask = j = 0; j < PUZZLE_DIM; j++) {
                        if (bitcount(g->cell[row[i][j]]) == 1) rowmask |= g->cell[row[i][j]];
                }
                if (rowmask != 0x01ff) {
                	if (verbose) {
				fprintf(rejects, "Row %d is inconsistent.\n", 1+i); fflush(rejects);
	                	flag = 0;
                        } else return 0;
                }
        }

        /* Check columns */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	for (colmask = j = 0; j < PUZZLE_DIM; j++) {
                        if (bitcount(g->cell[col[i][j]]) == 1) colmask |= g->cell[col[i][j]];
                }
                if (colmask != 0x01ff) {
                	if (verbose) {
				fprintf(rejects, "Column %d is inconsistent.\n", 1+i); fflush(rejects);
	                	flag = 0;
                        } else return 0;
                }
        }

        /* Check 3x3 regions */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	for (regmask = j = 0; j < PUZZLE_DIM; j++) {
                        if (bitcount(g->cell[region[i][j]]) == 1) regmask |= g->cell[region[i][j]];
                }
                if (regmask != 0x01ff) {
                	if (verbose) {
				fprintf(rejects, "Region %d is inconsistent.\n", 1+i); fflush(rejects);
	                	flag = 0;
                        } else return 0;
                }
        }

        return flag;
}

/********************************************************************************/
/* This function uses the cells with unique values, i.e. the given              */
/* or subsequently discovered solution values, to eliminate said values         */
/* as candidates in other as yet unsolved cells in the associated               */
/* rows, columns, and 3x3 regions.                                              */
/*                                                                              */
/* The function has three possible return values:                               */
/*   NOCHANGE - Markup did not change during the last pass,                     */
/*   CHANGE   - Markup was modified, and                                        */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values */
/********************************************************************************/

static int mark_cells(grid *g)
{
        int i, chgflag, bc;
        int const *r, *c, *reg;
        short elt, mask, before;


       	chgflag = NOCHANGE;

	while (g->tail < g->exposed) {

		elt = g->solved[g->tail++];

                r = row[map[elt].row];
                c = col[map[elt].col];
                reg = region[map[elt].region];

                mask = ~g->cell[elt];

                for (i = 0; i < PUZZLE_DIM; i++) {

			if (r[i] != elt) {

                                /* Get the cell value */
	                        before = g->cell[r[i]];

                                /* Eliminate this candidate value whilst preserving other candidate values */
				g->cell[r[i]] &= mask;

                                /* Did the cell change value? */
                	        if (before != g->cell[r[i]]) {

					chgflag |= CHANGE;	/* Flag that puzzle markup was changed */
                                        g->score += g->inc;	/* More work means higher scoring      */

                                	if (!(bc = bitcount(g->cell[r[i]]))) {
                                                EXPLAIN_MARKUP_IMPASSE(g, r[i], elt);
						return STUCK;	/* Crap out if no candidates remain */
                                        }

                                        EXPLAIN_MARKUP_ELIM(g, r[i], elt);

                                        /* Check if we solved for this cell, i.e. bit count indicates a unique value */
                                	if (bc == 1) {
						g->cellflags[r[i]] = FOUND;	/* Mark cell as found  */
                                                g->score += g->reward;		/* Add to puzzle score */
                                                g->solved[g->exposed++] = r[i];
                                                EXPLAIN_MARKUP_SOLVE(g, r[i]);
                                	}
	                        }
			}

			if (c[i] != elt) {

                        	/* Get the cell value */
	                        before = g->cell[c[i]];

                                /* Eliminate this candidate value whilst preserving other candidate values */
				g->cell[c[i]] &= mask;

                                /* Did the cell change value? */
                	        if (before != g->cell[c[i]]) {

					chgflag |= CHANGE;	/* Flag that puzzle markup was changed */
                                        g->score += g->inc;	/* More work means higher scoring      */

                                	if (!(bc = bitcount(g->cell[c[i]]))) {
                                        	EXPLAIN_MARKUP_IMPASSE(g, c[i], elt);
						return STUCK;	/* Crap out if no candidates remain */
                                        }

                                        EXPLAIN_MARKUP_ELIM(g, c[i], elt);

                                        /* Check if we solved for this cell, i.e. bit count indicates a unique value */
                                	if (bc == 1) {
						g->cellflags[c[i]] = FOUND;	/* Mark cell as found  */
                                                g->score += g->reward;		/* Add to puzzle score */
                                                g->solved[g->exposed++] = c[i];
                                                EXPLAIN_MARKUP_SOLVE(g, c[i]);
                                	}
	                        }
			}

			if (reg[i] != elt) {

                                /* Get the cell value */
	                        before = g->cell[reg[i]];

                                /* Eliminate this candidate value whilst preserving other candidate values */
				g->cell[reg[i]] &= mask;

                                /* Did the cell change value? */
                	        if (before != g->cell[reg[i]]) {

					chgflag |= CHANGE;	/* Flag that puzzle markup was changed */
                                        g->score += g->inc;	/* More work means higher scoring      */

                                	if (!(bc = bitcount(g->cell[reg[i]]))) {
                                        	EXPLAIN_MARKUP_IMPASSE(g, reg[i], elt);
						return STUCK;	/* Crap out if no candidates remain */
                                        }

                                        EXPLAIN_MARKUP_ELIM(g, reg[i], elt);

                                        /* Check if we solved for this cell, i.e. bit count indicates a unique value */
                                	if (bc == 1) {
						g->cellflags[reg[i]] = FOUND;	/* Mark cell as found  */
                                                g->score += g->reward;		/* Add to puzzle score */
                                                g->solved[g->exposed++] = reg[i];
                                                EXPLAIN_MARKUP_SOLVE(g, reg[i]);
                                        }
	                	}
			}
                        	
                }
        }

        return chgflag;
}


/*******************************************************************/
/* Identify and "solve" all cells that, by reason of their markup, */
/* can only assume one specific value, i.e. the cell is the only   */
/* one in a row/column/region (specified by vector) that is        */
/* able to assume a particular value.                              */
/*                                                                 */
/* The function has two possible return values:                    */
/*   NOCHANGE - Markup did not change during the last pass,        */
/*   CHANGE   - Markup was modified.                               */
/*******************************************************************/

static int find_singletons(grid *g, int const *vector, char *vdesc)
{
	int i, j, mask, hist[PUZZLE_DIM], value[PUZZLE_DIM], found = NOCHANGE;

	/* We are going to create a histogram of cell candidate values */
        /* for the specified cell vector (row/column/region).          */
        /* First set all buckets to zero.                              */
        memset(hist, 0, sizeof(hist[0])*PUZZLE_DIM);

        /* For each cell in the vector... */
	for (i = 0; i < PUZZLE_DIM; i++) {

        	/* For each possible candidate value... */
        	for (mask = 1, j = 0; j < PUZZLE_DIM; j++) {

                	/* If the cell may possibly assume this value... */
        		if (g->cell[vector[i]] & mask) {

                		value[j] = vector[i];	/* Save the cell coordinate */
                        	hist[j] += 1;		/* Bump bucket in histogram */
                	}

        		mask <<= 1;	/* Next candidate value */
                }
        }

        /* Examine each bucket in the histogram... */
        for (mask = 1, i = 0; i < PUZZLE_DIM; i++) {

        	/* If the bucket == 1 and the cell is not already solved,  */
		/* then the cell has a unique solution specified by "mask" */
        	if (hist[i] == 1 && !g->cellflags[value[i]]) {

                	found = CHANGE;			/* Indicate that markup has been changed */
                        g->cell[value[i]] = mask;	/* Assign solution value to cell         */
                        g->cellflags[value[i]] = FOUND;	/* Mark cell as solved                   */
                        g->score += g->reward;		/* Bump puzzle score                     */
                        g->solved[g->exposed++] = value[i];
                        EXPLAIN_SINGLETON(g, value[i], mask, vdesc);
                }

                mask <<= 1;		/* Get next candidate value */
        }

	return found;
}


/*******************************************************************/
/* Find all cells with unique solutions (according to markup)      */
/* and mark them as found. Do this for each row, column, and       */
/* region.                                                         */
/*                                                                 */
/* The function has two possible return values:                    */
/*   NOCHANGE - Markup did not change during the last pass,        */
/*   CHANGE   - Markup was modified.                               */
/*******************************************************************/

static int eliminate_singles(grid *g)
{
	int i, found = NOCHANGE;

        /* Do rows */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	found |= find_singletons(g, row[i], "row");
        }

        /* Do columns */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	found |= find_singletons(g, col[i], "column");
        }

        /* Do regions */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	found |= find_singletons(g, region[i], "region");
        }

        return found;
}

/********************************************************************************/
/* Solves simple puzzles, i.e. single elimination                               */
/*                                                                              */
/* The function has three possible return values:                               */
/*   NOCHANGE - Markup did not change during the last pass,                     */
/*   CHANGE   - Markup was modified, and                                        */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values */
/********************************************************************************/
static int simple_solver(grid *g)
{
	int flag = NOCHANGE;

        /* Mark the unsolved cells with candidate solutions based upon the current set of "givens" and solved cells */
        while ((flag |= mark_cells(g)) == CHANGE) {

        	g->inc = 1;	     /* After initial markup, we start scoring for additional markup work */

        	EXPLAIN_CURRENT_MARKUP(g);

		/* Continue to eliminate cells with unique candidate solutions from the game until */
        	/* elimination and repeated markup efforts produce no changes in the remaining     */
	        /* candidate solutions.                                                            */
                if (eliminate_singles(g) == NOCHANGE) break;

                EXPLAIN_CURRENT_MARKUP(g);
        }

        return flag;
}

/************************************************************************************/
/* Test a region to see if the candidate solutions for a paticular number           */
/* are confined to one row or column, and if so, eliminate                          */
/* their occurences in the remainder of the given row or column.                    */
/*                                                                                  */
/* The function has three possible return values:                                   */
/*   NOCHANGE - Markup did not change during the last pass,                         */
/*   CHANGE   - Markup was modified, and                                            */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values     */
/************************************************************************************/

static int region_vector_elim(grid *g, int region_no, int num)
{
	int i, j, r, c, mask, t, found;
        short rowhist[PUZZLE_DIM], colhist[PUZZLE_DIM];

        /* Init */
        found = NOCHANGE;
        memset(rowhist, 0, sizeof(rowhist[0])*PUZZLE_DIM);
        memset(colhist, 0, sizeof(colhist[0])*PUZZLE_DIM);

        mask = 1 << num;

	/* Create histograms for row and column placements for the value being checked */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	j = region[region_no][i];
        	if ((g->cell[j] & mask)) {
                	rowhist[map[j].row] += 1;
                        colhist[map[j].col] += 1;
                }
        }

        /* Figure out if this number lies in only one row or column */

        /* Check rows first*/
        r = c = -1;
        for (i = 0; i < PUZZLE_DIM; i++) {
        	if (rowhist[i]) {
                	if (r < 0) {
                        	r = i;
                        }
                        else {
                        	r = -1;
                                break;
                        }
                }
        }

        /* Now check columns */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	if (colhist[i]) {
                	if (c < 0) {
                        	c = i;
                        }
                        else {
                        	c = -1;
                                break;
                        }
                }
        }

        /* If the number is only in one row, then eliminate this number from the cells in the row outside of this region */
	if (r >= 0) {
        	for (i = 0; i < PUZZLE_DIM; i++) {
                	j = row[r][i];
                        if (map[j].region != region_no && !g->cellflags[j]) {
                        	t = g->cell[j];
                        	if ((g->cell[j] &= ~mask) == 0) { 
                                	EXPLAIN_VECTOR_IMPASSE(g, "row", r, j, mask, region_no);
					g->score += 10;
					return STUCK;
				}
                                if (t != g->cell[j]) {
					found = CHANGE;
                                        g->score += g->inc;
                                        EXPLAIN_VECTOR_ELIM("row", r, j, mask, region_no);
                                        if (bitcount(g->cell[j]) == 1) {
                                        	g->cellflags[j] = FOUND;
                                                g->score += g->reward;
                                                g->solved[g->exposed++] = j;
                                                EXPLAIN_VECTOR_SOLVE(g, j);
                                        }
                                }
                        }
                }
        }

        /* If the number is only in one column, then eliminate this number from the cells in the column outside of this region */
	else if (c >= 0) {
        	for (i = 0; i < PUZZLE_DIM; i++) {
                	j = col[c][i];
                        if (map[j].region != region_no && !g->cellflags[j]) {
                        	t = g->cell[j];
                        	if ((g->cell[j] &= ~mask) == 0) {
                                	EXPLAIN_VECTOR_IMPASSE(g, "column", c, j, mask, region_no);
					g->score += 10;
					return STUCK;
				}
                                if (t != g->cell[j]) {
					found = CHANGE;
                                        g->score += g->inc;
                                        EXPLAIN_VECTOR_ELIM("column", c, j, mask, region_no);
                                        if (bitcount(g->cell[j]) == 1) {
                                        	g->cellflags[j] = FOUND;
                                                g->score += g->reward;
                                                g->solved[g->exposed++] = j;
                                                EXPLAIN_VECTOR_SOLVE(g, j);
                                        }
                                }
                        }
                }
        }

        if (found == CHANGE) {
		g->score += 10;	/* Bump score for sucessfully invoking this rule */
        }

        return found;
}

/**********************************************************************************/
/* Test all regions to see if the possibilities for a number                      */
/* are confined to specific rows or columns, and if so, eliminate                 */
/* the occurence of candidate solutions from the remainder of the                 */
/* specified row or column.                                                       */
/*                                                                                */
/* The function has three possible return values:                                 */
/*   NOCHANGE - Markup did not change during the last pass,                       */
/*   CHANGE   - Markup was modified, and                                          */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values   */
/**********************************************************************************/

static int vector_elimination(grid *g)
{
	int i, j, rc;

        /* For each region... */
        for (rc = NOCHANGE, i = 0; i < PUZZLE_DIM && rc != STUCK; i++) {

        	/* For each digit... */
                for (j = 0; j < PUZZLE_DIM && rc != STUCK; j++) {

			/* Eliminate candidates outside of regions when a particular */
                        /* candidate value aligns itself to a row or column within   */
                        /* a 3x3 region.                                             */
                	rc |= region_vector_elim(g, i, j);
                }
        }

        return rc;
}

/**********************************************************************************/
/* This function implements the rule that when a subset of cells                  */
/* in a row/column/region contain matching subsets of candidate                   */
/* solutions, i.e. 2 matching possibilities for 2 cells, 3                        */
/* matching possibilities for 3 cells, etc., then those                           */
/* candidates may be eliminated from the other cells in the                       */
/* row, column, or region.                                                        */
/*                                                                                */
/* The function has three possible return values:                                 */
/*   NOCHANGE - Markup did not change during the last pass,                       */
/*   CHANGE   - Markup was modified, and                                          */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values   */
/**********************************************************************************/
 
static int elim_matches(grid *g, int const *cell_list, char *desc, int ndx)
{
	int i, j, k, e, count, rc, flag;
        short c, mask, tmp, elts[PUZZLE_DIM], eliminated[PUZZLE_DIM];
        static int counts[1<<PUZZLE_DIM];

        rc = NOCHANGE;

        /* Check for two and three valued subsets. Any more than that burns CPU cycles */
        /* and just slows us down.                                                     */
        for (i = 2; i < 4; i++) {

        	flag = 0;

                /* Create histogram to detect cells with matching subsets */
        	for (j = 0; j < PUZZLE_DIM; j++) {
                	k = cell_list[j];
                        elts[j] = g->cell[k];			/* Copy original cell candidates */

                        if (bitcount(g->cell[k]) == i) {
                        	counts[g->cell[k]] += 1;        /* The bucket records the number of cells with this subset */
                        }
                }

                /* For each cell in the list... */
        	for (e = j = 0; j < PUZZLE_DIM; j++) {

                	c = g->cell[cell_list[j]];		/* Get cell's candidates */

                        /* Check to see if we've already eliminated this subset */
                        for (k = 0; k < e; k++)
                        	if (c == eliminated[k]) break;
                        if (e && k < e) continue;

                        /* Get count from histogram bucket */
                        count = (int) (counts[c]);

                        /* If too few solution candidates for the number of cells, then we're stuck */
                        if (count > i) {
                        	EXPLAIN_TUPLE_IMPASSE(g, desc, ndx, c, count, i);
                        	/* Clean up static array */
                        	for (k = 0; k < 9; k++) counts[elts[k]] = 0;
                                g->score += 10;
				return STUCK;
                        }

                        /* Do candidate and cell counts match? */
                        if (count == i) {

                        	/* Compute mask used to eliminate candidates from other cells */
                        	mask = ~c;

                                /* Record (for later) the values being eliminated */
                                eliminated[e++] = c;

                                /* Eliminate candidates from the other cells in the list */

                                /* For each cell... */
                        	for (k = 0; k < PUZZLE_DIM; k++) {

                                	/* If the cell candidates do not exactly match the current subset... */
                                	if (c != g->cell[cell_list[k]] && !g->cellflags[cell_list[k]]) {

                                        	/* Get cell candidates */
                                        	tmp = g->cell[cell_list[k]];

                                                /* Eliminate candidates with our mask */
                                                g->cell[cell_list[k]] &= mask;

                                                /* Did the elimination change the candidates? */
                                                if (tmp != g->cell[cell_list[k]]) {

                                                	/* Note the change and bump the score */
							flag = CHANGE;
		                                        g->score += i;

                                                        EXPLAIN_TUPLE_ELIM(desc, ndx, c, cell_list[k]);

                                                        /* Did we solve the cell under consideration? */
                        	        	        if (bitcount(g->cell[cell_list[k]]) == 1) {

                                                        	/* Mark cell as found and bump the score */
                                        			g->cellflags[cell_list[k]] = FOUND;
                		                                g->score += g->reward;
                                                        	g->solved[g->exposed++] = cell_list[k];
                                                                EXPLAIN_TUPLE_SOLVE(g, cell_list[k]);
	        	                                }
        	                                }
                                        }
                                }
                        }
                }

                /* Cleanup the static histogram array */
                for (j = 0; j < PUZZLE_DIM; j++) counts[elts[j]] = 0;

                rc |= flag;
        }

        return rc;
}

/**********************************************************************************/
/* Eliminate subsets from rows, columns, and regions.                             */
/*                                                                                */
/* The function has three possible return values:                                 */
/*   NOCHANGE - Markup did not change during the last pass,                       */
/*   CHANGE   - Markup was modified, and                                          */
/*   STUCK    - Markup results are invalid, i.e. a cell has no candidate values   */
/**********************************************************************************/
 
static int mult_elimination(grid *g)
{
	int i, rc = NOCHANGE;

        /* Eliminate subsets from rows */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	rc |= elim_matches(g, row[i], "row", i);
        }

        /* Eliminate subsets from columns */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	rc |= elim_matches(g, col[i], "column", i);
        }

        /* Eliminate subsets from regions */
        for (i = 0; i < PUZZLE_DIM; i++) {
        	rc |= elim_matches(g, region[i], "region", i);
        }

        return rc;
}

/**************************************************/
/* Entry point to the recursive solver algorithm. */
/**************************************************/
static int rsolve(grid *g, return_soln soln_callback)
{
	int i, j, min, c, weight, mask, flag = 0;
        grid mygrid;

        /* Keep track of recursive depth */
        lvl += 1;
        if (lvl > g->maxlvl) g->maxlvl = lvl;

        for (;;) {

	        /* Attempt a simple solution */
        	if (simple_solver(g) == STUCK) break;

                /* Check for solution */
                if (g->exposed >= PUZZLE_CELLS) break;

	        g->reward += 2;		/* Bump reward as we graduate to more "advanced" solving techniques */

		/* Eliminate tuples */
                if ((flag = mult_elimination(g)) == CHANGE) {
			EXPLAIN_CURRENT_MARKUP(g);
			continue;
		}

                /* Check if impasse */
                if (flag == STUCK) break;

                /* Check for solution */
                if (g->exposed >= PUZZLE_CELLS) break;

                /* Eliminate clues aligned within regions from exterior cells in rows or columns */
                if ((flag = vector_elimination(g)) == CHANGE) {
			EXPLAIN_CURRENT_MARKUP(g);
			continue;
		}

                /* Check if impasse */
                if (flag == STUCK) break;

                /* Check for solution */
                if (g->exposed >= PUZZLE_CELLS) break;

                g->reward += 5;		/* Bump reward as we are about to start trial soutions */

                /* Attempt a trial solution */
        	memcpy(&mygrid, g, sizeof(grid));	/* Make working copy of puzzle */

		/* Find the first cell with the smallest number of alternatives */
        	for (weight= 0, c = -1, min = PUZZLE_DIM, i = 0; i < PUZZLE_CELLS; i++) {
        		if (!mygrid.cellflags[i]) {
                		j = bitcount(mygrid.cell[i]);
                                weight += 1;
        	                if (j < min) {
					min = j;
                        	        c = i;
	                        }
        	        }
	        }

                mygrid.score += weight;	/* Add penalty to score */

                /* Cell at index 'c' will be our starting point */
        	if (c >= 0) for (mask = 1, i = 0; i < PUZZLE_DIM; i++) {

                	/* Is this a candidate? */
        		if (mask & g->cell[c]) {

                        	EXPLAIN_TRIAL(c, mask);

	                        mygrid.score += (int)(((50.0 * lvl * weight) / (double)(PUZZLE_CELLS)) + 0.5);	/* Add'l penalty */

                                /* Try one of the possible candidates for this cell */
        	        	mygrid.cell[c] = mask;
                	        mygrid.cellflags[c] = FOUND;
                                mygrid.solved[mygrid.exposed++] = c;

				EXPLAIN_CURRENT_MARKUP(&mygrid);
	                        flag = rsolve(&mygrid, soln_callback);	/* Recurse with working copy of puzzle */

                                /* Did we find a solution? */
                        	if (flag == FOUND && !enumerate_all) {
                                	EXPLAIN_BACKTRACK;
                                        lvl -= 1;
                                        return FOUND;
        	                }

                                /* Preserve score, solution count and recursive depth as we back out of recursion */
                                g->score = mygrid.score;
                                g->solncount = mygrid.solncount;
                                g->maxlvl = mygrid.maxlvl;
                	        memcpy(&mygrid, g, sizeof(grid));
	                }
        		mask <<= 1;	/* Get next possible candidate */
	        }

                break;
        }

        if (g->exposed == PUZZLE_CELLS && validate(g, 0)) {
                soln_callback(g);
                g->solncount += 1;
	        EXPLAIN_SOLN_FOUND(g);
                EXPLAIN_BACKTRACK;
                lvl -= 1;
		flag = FOUND;
        } else {
	        EXPLAIN_BACKTRACK;
		lvl -= 1;
		flag = STUCK;
                if (!lvl && !g->solncount) validate(g, 1);		/* Print verbose diagnostic for insoluble puzzle */
        }

	return flag;
}

/*****************************************************************/
/* Add a puzzle solution to the singly linked list of solutions. */
/* Crap out if no memory available.                              */
/*****************************************************************/

static int add_soln(grid *g)
{
	grid *tmp;

	if ((tmp = malloc(sizeof(grid))) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	memcpy(tmp, g, sizeof(grid));
	tmp->next = soln_list;
	soln_list = tmp;
        return 0;
}

/************************************/
/* Print hints as to command usage. */
/************************************/

static void usage()
{
	fprintf(stderr, "Usage:\n\t%s {-p puzzle | -f <puzzle_file>} [-o <outfile>]\n", myname);
        fprintf(stderr, "\t\t[-r <reject_file>] [-1][-a][-c][-G][-g][-l][-m][-n][-s]\n");
        fprintf(stderr, "where:\n\t-1\tSearch for first solution, otherwise all solutions are returned\n"
                        "\t-a\tRequests that the answer (solution) be printed\n"
                        "\t-c\tPrint a count of solutions for each puzzle\n"
                        "\t-d\tPrint the recursive trial depth required to solve the puzzle\n"
#ifdef EXPLAIN
			"\t-e\tPrint a step-by-step explanation of the solution(s)\n"
#endif
                        "\t-f\tTakes an argument which specifes an input file\n\t\tcontaining one or more unsolved puzzles (default: stdin)\n"
                        "\t-G\tPrint the puzzle solution(s) in a 9x9 grid format\n"
                        "\t-g\tPrint the number of given clues\n"
                        "\t-m\tPrint an octal mask for the puzzle givens\n"
                        "\t-n\tNumber each result\n"
                        "\t-o\tSpecifies an output file for the solutions (default: stdout)\n"
                        "\t-p\tTakes an argument giving a single inline puzzle to be solved\n"
                        "\t-r\tSpecifies an output file for unsolvable puzzles\n\t\t(default: stderr)\n"
                        "\t-s\tPrint the puzzle's score or difficulty rating\n"
			"\t-?\tPrint usage information\n\n");
        fprintf(stderr, "The return code is zero if all puzzles had unique solutions,\n"
                        "(or have one or more solutions when -1 is specified) and non-zero\n"
                        "when no unique solution exists.\n");
}

/********************************************************/
/* Print the puzzle as an 81 character string of digits */
/********************************************************/

static char *format_answer(grid *g, char *outbuf)
{
	int i;

	for (i = 0; i < PUZZLE_CELLS; i++)
		outbuf[i] = symtab[g->cell[i]];
	outbuf[i] = 0;

        return outbuf;
}

/*******************************************/
/* Print the puzzle as a standard 9x9 grid */
/*******************************************/

static void print_grid(char *sud, FILE *h)
{

        fprintf(h, "\n");
        EXPLAIN_INDENT(h);
	fprintf(h, "+---+---+---+\n");

        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud, PUZZLE_ORDER, PUZZLE_ORDER, sud+3, PUZZLE_ORDER, PUZZLE_ORDER, sud+6);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+9, PUZZLE_ORDER, PUZZLE_ORDER, sud+12, PUZZLE_ORDER, PUZZLE_ORDER, sud+15);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+18, PUZZLE_ORDER, PUZZLE_ORDER, sud+21, PUZZLE_ORDER, PUZZLE_ORDER, sud+24);

        EXPLAIN_INDENT(h);
        fprintf(h, "+---+---+---+\n");

        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+27, PUZZLE_ORDER, PUZZLE_ORDER, sud+30, PUZZLE_ORDER, PUZZLE_ORDER, sud+33);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+36, PUZZLE_ORDER, PUZZLE_ORDER, sud+39, PUZZLE_ORDER, PUZZLE_ORDER, sud+42);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+45, PUZZLE_ORDER, PUZZLE_ORDER, sud+48, PUZZLE_ORDER, PUZZLE_ORDER, sud+51);

        EXPLAIN_INDENT(h);
        fprintf(h, "+---+---+---+\n");

        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+54, PUZZLE_ORDER, PUZZLE_ORDER, sud+57, PUZZLE_ORDER, PUZZLE_ORDER, sud+60);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+63, PUZZLE_ORDER, PUZZLE_ORDER, sud+66, PUZZLE_ORDER, PUZZLE_ORDER, sud+69);
        EXPLAIN_INDENT(h);
        fprintf(h, "|%*.*s|%*.*s|%*.*s|\n", PUZZLE_ORDER, PUZZLE_ORDER, sud+72, PUZZLE_ORDER, PUZZLE_ORDER, sud+75, PUZZLE_ORDER, PUZZLE_ORDER, sud+78);

        EXPLAIN_INDENT(h);
        fprintf(h, "+---+---+---+\n");
}

/*****************************************************/
/* Based upon the Left-to-Right-Top-to-Bottom puzzle */
/* presented in "sbuf", create a 27 octal digit      */
/* mask of the givens in the 28 character buffer     */
/* pointed to by "mbuf." Return a pointer to mbuf.   */
/*****************************************************/

static char *cvt_to_mask(char *mbuf, char *sbuf)
{
        char *mask_buf = mbuf;
        static const char *maskchar = "01234567";
        int i, m;

        mask_buf[PUZZLE_DIM*3] = 0;
        for (i = 0; i < PUZZLE_CELLS; i += 3) {
	   m = 0;
           if (is_given(sbuf[i])) {
		m |= 4;
           }
	   else {
                sbuf[i] = '0';
           }
           if (is_given(sbuf[i+1])) {
		m |= 2;
           }
	   else {
                sbuf[i+1] = '0';
           }
           if (is_given(sbuf[i+2])) {
		m |= 1;
           }
	   else {
                sbuf[i+2] = '0';
           }
           *mask_buf++ = maskchar[m];
        }
	return mbuf;
}

/*******************/
/* Mainline logic. */
/*******************/

int main(int argc, char **argv)
{
	int i, rc, bog, count, opt, solved, unsolved, solncount, flag, prt_count, prt_num, prt_score, prt_answer, prt_depth, prt_grid, prt_mask, prt_givens, prt, len;
        char *infile, *outfile, *rejectfile, inbuf[128], outbuf[128], mbuf[28];
	grid g, *s;
        FILE *h;

        /* Get our command name from invoking command line */
	if ((myname = strrchr(argv[0], '/')) == NULL)
        	myname = argv[0];
	else
		myname++;

        /* Print sign-on message to console */
        fprintf(stderr, "%s version %s\n", myname, VERSION); fflush(stderr);

        /* Init */
        h = stdin;
        solnfile = stdout;
        rejects = stderr;
        rejectfile = infile = outfile = NULL;
        rc = bog = prt_mask = prt_grid = prt_score = prt_depth = prt_answer = prt_count = prt_num = prt_givens = 0;
        *inbuf = 0;

        /* Parse command line options */
	while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        	switch (opt) {
                        case '1':
                        	enumerate_all = 0;		/* only find first soln */
                                break;
                        case 'a':
                        	prt_answer = 1;		/* print solution */
                                break;
                        case 'c':
                        	prt_count = 1;		/* number solutions */
                                break;
                        case 'd':
                        	prt_depth = 1;
                                break;
#ifdef EXPLAIN
                        case 'e':
                        	explain = 1;
                                break;
#endif
                	case 'f':
                        	if (*inbuf) {		/* -p and -f options are mutually exclusive */
                                	fprintf(stderr, "The -p and -f options are mutually exclusive\n");
                                	usage();
                                        exit(1);
                                }
                        	infile = optarg;	/* get name of input file */
                                break;
                        case 'G':
                        	prt_grid = 1;
                                break;
                        case 'g':
                        	prt_givens = 1;
                                break;
                        case 'm':
                        	prt_mask = 1;
                                break;
                        case 'n':
                        	prt_num = 1;
                                break;
                	case 'o':
                        	outfile = optarg;
                                break;
                	case 'p':
                        	if (infile) {
                                	fprintf(stderr, "The -p and -f options are mutually exclusive\n");
                                	usage();
                                        exit(1);
                                }
                        	if (strlen(optarg) == PUZZLE_CELLS) {
                                	strcpy(inbuf, optarg);
                                }
                                else {
                                	fprintf(stderr, "Invalid puzzle specified: %s\n", optarg);
                                        usage();
                                        exit(1);
                                }
                                h = NULL;
                                break;
                	case 'r':
                        	rejectfile = optarg;
                                break;
                        case 's':
                        	prt_score = 1;
                                break;
                	default:
                	case '?':
                        	usage();
				exit(1);
                }
        }

        /* Set prt flag if we're printing anything at all */
	prt = prt_mask | prt_grid | prt_score | prt_depth | prt_answer | prt_num | prt_givens;

        /* Anthing else on the command line is bogus */
        if (argc > optind) {
        	fprintf(stderr, "Extraneous args: ");
                for (i = optind; i < argc; i++) {
                	fprintf(stderr, "%s ", argv[i]);
                }
                fprintf(stderr, "\n\n");
                usage();
                exit(1);
        }

        if (!enumerate_all && prt_score) {
        	fprintf(stderr, "Scoring is meaningless when multi-solution mode is disabled.\n");
        }

        if (rejectfile && !(rejects = fopen(rejectfile, "w"))) {
                fprintf(stderr, "Failed to open reject output file: %s\n", rejectfile);
		exit(1);
        }

        if (outfile && !(solnfile = fopen(outfile, "w"))) {
                fprintf(stderr, "Failed to open solution output file: %s\n", outfile);
		exit(1);
        }

	if (infile && strcmp(infile, "-") && !(h = fopen(infile, "r"))) {
        	fprintf(stderr, "Failed to open input game file: %s\n", infile);
		exit(1);
        }

        count = solved = unsolved = 0;
        if (h) fgets(inbuf, 128, h);

        while (*inbuf) {

                if ((len = strlen(inbuf)) && inbuf[len-1] == '\n') {
                	len -= 1;
                	inbuf[len] = 0;
                }

		count += 1;
        	if (len != PUZZLE_CELLS) {
                	fprintf(rejects, "%d: %s bogus puzzle format\n", count, inbuf); fflush(rejects);
	                *inbuf = 0;
                        bog += 1;
		        if (h) fgets(inbuf, 128, h);
                        continue;
                }

	        cvt_to_grid(&g, inbuf);
        	if (g.givens < 17) {
        		fprintf(rejects, "%d: %*.*s bogus puzzle has less than 17 givens\n", count, PUZZLE_CELLS, PUZZLE_CELLS, inbuf); fflush(rejects);
		        *inbuf = 0;
        	        bog += 1;
			if (h) fgets(inbuf, 128, h);
	                continue;
        	}

                for (s = soln_list; s;) {
			s = soln_list->next;
                        free(soln_list);
                        soln_list = s;
                }

		flag = rsolve(&g, add_soln);
        	if (soln_list) {
	               	solved++;
	                for (solncount = 0, s = soln_list; s; s = s->next) {
                        	solncount += 1;
	        		if (prt_num) {
        	                	char nbuf[32];
                	                if (!enumerate_all)
						sprintf(nbuf, "%d: ", count);
                                        else
						sprintf(nbuf, "%d:%d ", count, solncount);
					fprintf(solnfile, "%-s", nbuf);
	                        }
                                if (solncount > 1 || !enumerate_all) g.score = 0;
        	                if (prt_score) fprintf(solnfile, "score: %-7d ", g.score);
                	        if (prt_depth) fprintf(solnfile, "depth: %-3d ", g.maxlvl);
                        	if (prt_answer || prt_grid) format_answer(s, outbuf);
	                        if (prt_answer) fprintf(solnfile, "%s", outbuf);
                                if (prt_mask) fprintf(solnfile, " %s", cvt_to_mask(mbuf, inbuf));
                                if (prt_givens) fprintf(solnfile, " %d", g.givens);
        	                if (prt_grid) print_grid(outbuf, solnfile);
                	        if (prt) fprintf(solnfile, "\n");
                                if (s->next == NULL && prt_count) fprintf(solnfile, "count: %d\n", solncount);
                        }
                        if (solncount > 1 && enumerate_all) {
                               	rc |= 1;
                        }
                }
                else {
                	unsolved++;
                        rc |= 1;
                	fprintf(rejects, "%d: %*.*s unsolved\n", count, PUZZLE_CELLS, PUZZLE_CELLS, inbuf); fflush(rejects);
			diagnostic_grid(&g, rejects);
                        #if defined(DEBUG)
			mypause();
                        #endif
                }

                *inbuf = 0;
	        if (h) fgets(inbuf, 128, h);
	}

        if (prt) fprintf(solnfile, "\nPuzzles: %d, Solved: %d, Unsolved: %d, Bogus: %d\n", count, solved, unsolved, bog);

	return rc;
}
