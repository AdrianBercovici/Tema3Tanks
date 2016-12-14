#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#define NMAX 20
using namespace std;
char a[NMAX][NMAX];
bool A,D,S,W,X,X2,X3,gameOver;
bool previousA,previousD,previousS,previousW;
char ProjectileForm;
bool release;
struct target{
    int line;
    int column;
}tank;
struct proiectil{
    int line;
    int column;
}P1,P2,P3;
void type()
{
    A=S=D=W=0;
    system("cls");
    for(int i=0; i<NMAX; i++){
            a[0][i]=a[NMAX-1][i]='#';
            a[i][0]=a[i][NMAX-1]='#';
            }
    for(int i=0; i<NMAX;i++)
      {
        for(int j=0; j<NMAX;j++)
        {
            if(i==tank.line && j==tank.column)cout<<'T';
            else if(i==P1.line && j==P1.column && X)cout<<'o';
          //  else if(a[i][j]==0)cout<<' ';
            else cout<<a[i][j];


        }
        cout<<endl;
        }
}
void Input()
{

    if(_kbhit()) //If player clicks something

{  release=false;
    char key;
    key = _getch(); //Now this click is key
    switch( key )
    {
        case 'd':

        {
           {
            D=1;
            A=S=W=0;
           }
            break;
        }

        case 'w':
        {
            {
                A = 0, S = 0, D = 0,
                W = 1;
            }
        break;
        }

        case 'a':
        {
            {
                D = 0, W = 0, S = 0,
                A = 1;
			}
        break;
        }

        case 's':
        {
            {
                A = 0, W = 0, D = 0,
                S = 1;
            }
        break;
        }
        case ' ':
        {
            {
               A=S=W=D=0;
               X=1;
            }
        break;
        }

    }

}
}
void Logic()
{
    if(D==1){
        if(!X)previousD=1;
        tank.column++;

    }
    else if(A==1){
        if(!X)previousA=1;
        tank.column--;

    }
    else if(W==1){
        if(!X)previousW=1;
        tank.line--;
    }
    else if(S==1){
        if(!X)previousS=1;
        tank.line++;
    }
    if(X==1){
        if(previousS)P1.line++;
     /*   if(previousA)P1.column--;
        if(previousD)P1.column++;
        if(previousW)P1.line--;*/
    }
    else if(X==0)
    {
        P1.line=tank.line;
        P1.column=tank.column;
    }
    if(tank.line == 0 || tank.line == NMAX-1 || tank.column == 0 || tank.column == NMAX-1){
        tank.line=tank.column=NMAX/2;
    }
    if(P1.line == 0 || P1.line == NMAX-1 || P1.column == 0 || P1.column == NMAX-1)
    {
        P1.line=tank.line;
        P1.column=tank.column;
        X=0;
    }

}
int main()
{
    tank.line=tank.column=NMAX/2;
    gameOver=false;
    release=true;
    while(!gameOver)
    {
        type();
        Input();
        Logic();
//        Sleep();
    }

    return 0;
}
