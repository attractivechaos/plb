/* JSolveMain.c - A very fast Sudoku solver

Version 1.2 of January 22, 2010

Copyright (c) 2009-2010, Jason T. Linhart
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
	•	Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.
	•	Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.
	•	Neither the name Jason T. Linhart nor the names of other contributors
		may be used to endorse or promote products derived from this software
		without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined(_WIN32) || defined(_WINDOWS)
#include <time.h>
#else
#include <sys/resource.h>
#endif // Windows

#include "JSolve.h"

#if defined(_WIN32) || defined(_WINDOWS)
static unsigned long
MilliTime(void)					// 1,000ths of a second of real time
{
	return(clock() / (double)CLOCKS_PER_SEC * 1000);
	}
#else
static unsigned long
MilliTime(void)					// 1,000ths of a second of CPU time
{
	struct rusage usage;

	if (getrusage(RUSAGE_SELF,&usage)>=0) 
		return(usage.ru_utime.tv_sec*1000+usage.ru_utime.tv_usec/1000);
	return(0);
	}
#endif // Windows

static void						// Format a number with commas
PrintComma(char *dest,unsigned long value)
{
	if (value>=1000000000) sprintf(dest,"%ld,%03ld,%03ld,%03ld",
		value/1000000000,value/1000000%1000,value/1000%1000,value%1000);
	else if (value>=1000000) sprintf(dest,"%ld,%03ld,%03ld",
		value/1000000,value/1000%1000,value%1000);
	else if (value>=1000) sprintf(dest,"%ld,%03ld",
		value/1000,value%1000);
	else sprintf(dest,"%ld",value);
	}

static const char *
CommaStr(unsigned long value)	// Format a number with commas for (s,f)printf
{
	static int indx = 0;
	static char strs[4][16];

	indx=(indx+1)&3;
	PrintComma(strs[indx],value);
	return(strs[indx]);
	}

int
main(int argc,char *argv[])
{
	FILE *fd;
	unsigned long time, total, temp;
	unsigned long counts[3];
	char buffer[1024], clues[82], *cptr, *dptr;

	total=0;
	counts[0]=counts[1]=counts[2]=0;
	if (argc>1) {
		fd=fopen(argv[1],"r");
		if (!fd) {
			printf("Unable to open '%s' for input!\n",argv[1]);
			return(1);
			}
		}
	else fd=stdin;
	time=MilliTime();
	while (fgets(buffer,1024,fd)) {
		dptr=clues;
		for (cptr=buffer; *cptr && dptr-clues<81; ++cptr) {
			if (isdigit(*cptr) || *cptr=='.') *dptr++ = *cptr;
			else if (*cptr=='#') break;
			}
		*dptr = 0;
		if (dptr-clues==81) {
			++total;
			counts[JSolve(clues,0,2)]+=1;
			}
		}
	time=MilliTime()-time;
	fclose(fd);
	if (time==0) time=1;
	if (total<42949) temp=(total*100000)/time;
	else if (total<429490) temp=(total*10000)/((time+5)/10);
	else if (total<4294900) temp=(total*1000)/((time+50)/100);
	else if (total<42949000) temp=(total*100)/((time+500)/1000);
	else if (total<429490000) temp=(total*10)/((time+5000)/10000);
	else temp=total/((time+50000)/100000);
	printf("Examined %s puzzles in %lu.%03lu seconds or %s.%02lu puzzles/sec\n",
		CommaStr(total),time/1000,time%1000,CommaStr(temp/100),temp%100);
	printf("Solved: %s puzzles, %s invalid, %s multi-solution\n",
		CommaStr(counts[1]),CommaStr(counts[0]),CommaStr(counts[2]));
	return(0);
	}

/* END OF JSolveMain.c - A very fast Sudoku solver */
