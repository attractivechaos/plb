/* Extracted from http://magictour.free.fr/suexk.exe using
   http://magictour.free.fr/unattach.c. Timing is removed due to portability. -ac
 */

// simple and short sudoku-solver based on an exact-cover-problem-solver
// by Guenter Stertenbrink,sterten@aol.com   compiled with GCC3.2
// some explanations are at : http://magictour.free.fr/suexco.doc
// DOS/Windows-executable is at : http://magictour.free.fr/suexco.exe

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define M 8 // change this for larger grids. Use symbols as in L[] below
#define M2 M*M
#define M4 M2*M2
#define MWC (   (zr=36969*(zr&65535)+(zr>>16))   ^   (wr=18000*(wr&65535)+(wr>>16))   )
unsigned zr=362436069, wr=521288629;
 int A0[M2+9][M2+9],A[M2+9][M2+9],Rows[4*M4+9],Cols[M2*M4+9],Row[4*M4+9][M2+9];
 int Col[M2*M4+9][5],Ur[M2*M4+9],Uc[4*M4+9],V[M2*M4+9];
 int C[M4+9],I[M4+9],T[M2*M4+9],P[M2*M4+9];
 int Mr[9]={0,1,63,1023,4095,16383,46655,131071,262143};
 int Mc[9]={0,1,63,511,1023,4095,8191,16383,16383};
 int Mw[9]={0,1,3,15,15,31,63,63,63};
//unfortunately there is no array-bound-checking in C, if someone knows
// a utility to perform this, please tell me !
 int nocheck=0,time0,time1,max,try,rnd=0,min,clues,gu,tries;
long long Node[M4+9],nodes,tnodes,solutions,vmax,smax;
double xx,yy;

 int q,a,p,i,i1,j,k,l,r,r1,c,c1,c2,n,N=0,N2,N4,m,m0,m1,t1,x,y,s;
char L[66]=".123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#*~";
char*Arg;
 FILE *file;
int shuffle();

int main(int argc,char*argv[]){ 
  if(argc<2){m5:printf("\nusage:suexco file [nn] [p] [smax] [vmax] \n\n");
  printf("prints the number of (<max)solutions of the sudokus in file\n\n");
  printf("empty cells are -.* or 0 , other nondigit-characters are");
  printf(" ignored\n\n");
  printf("n3:n=3,9*9-sudoku  (default:guess the size)\n");
  printf("s47:interrupt after 47 solutions   (default=999)\n");
  printf("v500000:interrupt after 500000 nodes (default=4000000)\n");
  printf("r99999: random restart after 99999 nodes (default=oo)\n");
  printf("r1: randomly shuffle the exact-cover matrix (default=don't)\n");
  printf("r17: random choice on ties (default:first match)\n");
  printf("r18: alternate on ties\n");
  printf("p:print solutions  p=6,only counts(default=don't)\n");
  printf("q50:prunes valid placements (32<i<65) with probability 50/1.28 %% \n");
  printf("t1000:same calculation 1000-fold for benchmarking (default=1)\n");
  exit(1);}vmax=4000000;smax=999;tries=1;p=0;q=0;
  for(k=2;k<argc;k++){Arg=argv[k]+1;
  if(argv[k][0]=='n')sscanf(Arg,"%i",&N);
  if(argv[k][0]=='s')sscanf(Arg,"%Li",&smax);
  if(argv[k][0]=='p'){sscanf(Arg,"%i",&p);if(p==0)p=1;}
  if(argv[k][0]=='r')sscanf(Arg,"%i",&rnd);
  if(argv[k][0]=='q')sscanf(Arg,"%i",&q);
  if(argv[k][0]=='v')sscanf(Arg,"%Li",&vmax);
  if(argv[k][0]=='c')nocheck=1;
  if(argv[k][0]=='t')sscanf(Arg,"%i",&tries);}

 x=time(0);zr^=x;wr+=x;
 if(rnd<999){zr^=rnd;wr+=rnd;for(i=1;i<rnd;i++)MWC;}
if(q){vmax=99999999;smax=99999999;}

 if(N==0){if((file=fopen(argv[1],"rb"))==NULL)
    {fclose(file);printf("\nfile-error\n\n");goto m5;}
 y=0;while(feof(file)==0){x=fgetc(file);if(x>y && x<123)y=x;}
 fclose(file);N=3+(y>66)+(y>79)+(y>90)+(y>99);}
  N2=N*N;N4=N2*N2;m=4*N4;n=N2*N4;


 if((file=fopen(argv[1],"rb"))==NULL)
    {fclose(file);printf("\nfile-error\n\n");goto m5;}
    time0=clock();
m6:clues=0;t1=clock();i=0;for(x=1;x<=N2;x++)for(y=1;y<=N2;y++){
   m1:if(feof(file))exit(1);
   c=fgetc(file);j=0;if(c=='-' || c=='.'|| c=='0' || c=='*')goto m7;
   while(L[j]!=c && j<=N2)j++;if(j>N2)goto m1;
   m7:A0[x][y]=j;if(j)clues++;i++;};if(clues==N4){clues--;A0[1][1]=0;}

// for(x=1;x<=N2;x++){for(y=1;y<=N2;y++)printf("%i",A0[x][y]);printf("\n");}

if(p<8){for(i=0;i<=N4;i++)Node[i]=0;}tnodes=0;

for(try=1;try<=tries;try++){ // you can do multiple tries for benchmarking here

restart:;
r=0;for(x=1;x<=N2;x++)for(y=1;y<=N2;y++)for(s=1;s<=N2;s++){
r++;Cols[r]=4;Col[r][1]=x*N2-N2+y;Col[r][4]=(N*((x-1)/N)+(y-1)/N)*N2+s+N4;
Col[r][3]=x*N2-N2+s+N4*2;Col[r][2]=y*N2-N2+s+N4*3;}
for(c=1;c<=m;c++)Rows[c]=0;
for(r=1;r<=n;r++)for(c=1;c<=Cols[r];c++){
  x=Col[r][c];Rows[x]++;Row[x][Rows[x]]=r;}

 for(x=1;x<=N2;x++)for(y=1;y<=N2;y++)A[x][y]=A0[x][y];
 for(i=0;i<=n;i++)Ur[i]=0;for(i=0;i<=m;i++)Uc[i]=0;
solutions=0;
 for(x=1;x<=N2;x++)for(y=1;y<=N2;y++)
   if(A[x][y]){r=x*N4-N4+y*N2-N2+A[x][y];
     for(j=1;j<=Cols[r];j++){c1=Col[r][j];if(Uc[c1]>0 && nocheck==0)goto next_try;Uc[c1]++;
       for(k=1;k<=Rows[c1];k++){r1=Row[c1][k];Ur[r1]++;}}}
if(rnd>0 && rnd!=17 &&rnd!=18)shuffle();
 for(c=1;c<=m;c++){V[c]=0;for(r=1;r<=Rows[c];r++)if(Ur[Row[c][r]]==0)V[c]++;}

//---------walk through the searchtree now------------------
   i=clues;nodes=0;m0=0;m1=0;gu=0;solutions=0;
m2:i++;I[i]=0;min=n+1;if(i>N4 || m0)goto m4;
   if(m1){C[i]=m1;goto m3;}
   for(c=1;c<=m;c++)if(!Uc[c]){if(V[c]<=min)c1=c;
     if(V[c]<min){min=V[c];C[i]=c;if(min<2)goto m3;}}
   gu++;if(min>2)goto m3;

if((rnd&255)==18)if(nodes&1){c=m+1;m3v:c--;if(Uc[c] || V[c]!=2)goto m3v;C[i]=c;}

if((rnd&255)==17){mr5:c1=MWC&Mc[N];if(c1>=m)goto mr5;c1++;
   for(c=c1;c<=m;c++)if(!Uc[c])if(V[c]==2){C[i]=c;goto m3;}
   for(c=1;c<c1;c++)if(!Uc[c])if(V[c]==2){C[i]=c;goto m3;}}

m3:c=C[i];I[i]++;if(I[i]>Rows[c])goto m4;
   r=Row[c][I[i]];if(Ur[r])goto m3;m0=0;m1=0;


if(q>0 && i>32 && i<65)if((MWC&127)<q)goto m3;
//if(q>0 && i>q)goto m3; //##q
//if(i==37 && (MWC&1023)>1)goto m3;
//if(i==48 && (MWC&1023)>1)goto m3;

// j=N2;k=N4;x=(r-1)/k+1;y=((r-1)%k)/j+1;s=(r-1)%j+1;printf("%i:%i%i%i\n",i,x,y,s);
   if(p&1){j=N2;k=N4;x=(r-1)/k+1;y=((r-1)%k)/j+1;s=(r-1)%j+1;A[x][y]=s;if(i==k)
    {for(x=1;x<=j;x++)for(y=1;y<=j;y++)printf("%c",L[A[x][y]]);printf("\n");
//goto next_try;
}}


   for(j=1;j<=Cols[r];j++){c1=Col[r][j];Uc[c1]++;}
   for(j=1;j<=Cols[r];j++){c1=Col[r][j];
      for(k=1;k<=Rows[c1];k++){r1=Row[c1][k];Ur[r1]++;if(Ur[r1]==1)
         for(l=1;l<=Cols[r1];l++){c2=Col[r1][l];V[c2]--;
            if(Uc[c2]+V[c2]<1)m0=c2;if(Uc[c2]==0 && V[c2]<2)m1=c2;}}}
   Node[i]++;tnodes++;nodes++;if(rnd>99 && nodes>rnd){printf("restart\n");goto restart;}
    if(i==N4)solutions++;
    if(solutions>=smax){if(try==1)printf("+");goto next_try;}
   if(tnodes>vmax){if(try==1)printf("-");goto next_try;}
   goto m2;
m4:i--;c=C[i];r=Row[c][I[i]];if(i==clues)goto next_try;
   for(j=1;j<=Cols[r];j++){c1=Col[r][j];Uc[c1]--;
      for(k=1;k<=Rows[c1];k++){r1=Row[c1][k];Ur[r1]--;
         if(Ur[r1]==0)for(l=1;l<=Cols[r1];l++){c2=Col[r1][l];V[c2]++;}}}
   if(p){j=N2;k=N4;x=(r-1)/k+1;y=((r-1)%k)/j+1;s=(r-1)%j+1;A[x][y]=0;}
   if(i>clues)goto m3;
next_try:;}

if(q){xx=128;yy=128-q;xx=xx/yy;yy=solutions;for(i=1;i<33;i++)yy=yy*xx;printf("clues:%i  estimated solutions:%1.2le\n",clues,yy);goto m6;}
if(!p && tnodes<=999999){printf("%Li sol.  %6Li nodes  %i guesses  NA/91sec  %i \n",solutions,tnodes,gu,x);goto m6;}
if(p==6){printf("%9Li\n",solutions);goto m6;}
if(!p){printf("%Li sol.  %Li nodes  %i guesses  NA/91sec  %i \n",solutions,tnodes,gu,x);}
if(p>5){x=0;for(i=1;i<=N4;i++){x+=Node[i];printf("%Li ",Node[i]);}printf("  %i\n",x);}

goto m6; }



int shuffle(){
for(i=1;i<=m;i++){m43:a=(MWC>>8)&Mc[N];if(a>=i)goto m43;a++;P[i]=P[a];P[a]=i;}
for(c=1;c<=m;c++){Rows[c]=0;T[c]=Uc[c];}for(c=1;c<=m;c++)Uc[P[c]]=T[c];
for(r=1;r<=n;r++)for(i=1;i<=Cols[r];i++){
    c=P[Col[r][i]];Col[r][i]=c;Rows[c]++;Row[c][Rows[c]]=r;}

for(i=1;i<=n;i++){m42:a=(MWC>>8)&Mr[N];if(a>=i)goto m42;a++;P[i]=P[a];P[a]=i;}
for(r=1;r<=n;r++){Cols[r]=0;T[r]=Ur[r];}for(r=1;r<=n;r++)Ur[P[r]]=T[r];
for(c=1;c<=m;c++)for(i=1;i<=Rows[c];i++){
    r=P[Row[c][i]];Row[c][i]=r;Cols[r]++;Col[r][Cols[r]]=c;}

for(r=1;r<=n;r++){
  for(i=1;i<=Cols[r];i++){m45:a=(MWC>>8)&7;if(a>=i)goto m45;a++;P[i]=P[a];P[a]=i;}
  for(i=1;i<=Cols[r];i++)T[i]=Col[r][P[i]];
  for(i=1;i<=Cols[r];i++)Col[r][i]=T[i];}

for(c=1;c<=m;c++){
  for(i=1;i<=Rows[c];i++){m46:a=(MWC>>8)&Mw[N];if(a>=i)goto m46;a++;P[i]=P[a];P[a]=i;}
  for(i=1;i<=Rows[c];i++)T[i]=Row[c][P[i]];
  for(i=1;i<=Rows[c];i++)Row[c][i]=T[i];}

}
