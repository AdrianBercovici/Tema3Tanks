#include <iostream>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#define NMAX 20
using namespace std;
char a[NMAX][NMAX];
bool A,D,S,W,gameOver;
struct target{
    int line;
    int column;
}tank;
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
            else if(a[i][j]==0)cout<<' ';
            else cout<<a[i][j];


        }
        cout<<endl;
        }
}
void Input()
{
    if(_kbhit()) //If player clicks something

{
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

    }

}
}
void Logic()
{
    if(D==1){
        tank.column++;
    }
    else if(A==1){
        tank.column--;

    }
    else if(W==1){
        tank.line--;
    }
    else if(S==1){
        tank.line++;
    }
}
int main()
{
    tank.line=tank.column=3;
    gameOver=false;
    while(!gameOver)
    {
        type();
        Input();
        Logic();
    }

    return 0;
}
