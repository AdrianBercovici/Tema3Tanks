#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <cmath>
using namespace std;

ifstream fin("test.in");
ofstream fout("test.out");

#define maxMapSize 22
#define mapSizeSmall 12
#define mapSizeMediu 17
#define minObstacleDistance 4

int mapSize,harta[maxMapSize][maxMapSize],nrOfAgents;
int dx[] = {-1,-1,-1,0,1,1,1,0},dy[] = {-1,0,1,1,1,0,-1,-1};
bool playerPlaying;

struct Vector2
{
    int x;
    int y;
};

struct WayPointNode
{
    //distanta intre 2 patrate oblice e @1.4
    //distanta intre 2 patrate pe verticala sau orizontala e 1
    // le dam multiply by 10 ca sa lucram cu numere frum: 10 si 14
    int gCost;//distance from the startingNode
    int hCost;//distance from the targetNode
    int fCost;//suma gCost + fCost;
    bool walkable;
    Vector2 previousPosition;

};

WayPointNode wayPoints[maxMapSize][maxMapSize];


struct Tank
{
    Vector2 pozitie;
    int healthPoints;
};
Tank Agents[4];

int obstacleMatrix[2][maxMapSize / 2];

void CheckIfPassable(int x,int y,int directionX,int directionY)
{
    int checkX,checkY;
    if (directionX == -1 && directionY == -1)
    {
        int topLeftX[3] = {0,-1,-1};
        int topLeftY[3] = {-1,-1,0};
        int k;
        for (k = 0; k <= 2; k++)
        {
            checkX = x + topLeftX[k];
            checkY = y + topLeftY[k];
            if (harta[checkX][checkY] == 1)
            {
                harta[x][y] = harta[checkX][checkY] = 0;
            }
        }
        return;
    }
    if (directionX == -1 && directionY == 1)
    {
        int toprightX[3] = {-1,-1,0};
        int toprightY[3] = {0,1,1};
        int k;
        for (k = 0; k <= 2; k++)
        {
            checkX = x + toprightX[k];
            checkY = y + toprightY[k];
            if (harta[checkX][checkY] == 1)
            {
                harta[x][y] = harta[checkX][checkY] = 0;
            }
        }
        return;
    }
    if (directionX == 1 && directionY == 1)
    {
        int bottomRightX[3] = {0,1,1};
        int bottomRightY[3] = {1,1,0};
        int k;
        for (k = 0; k <= 2; k++)
        {
            checkX = x + bottomRightX[k];
            checkY = y + bottomRightY[k];
            if (harta[checkX][checkY] == 1)
            {
                harta[x][y] = harta[checkX][checkY] = 0;
            }
        }
        return;
    }
    if (directionX == 1 && directionY == -1)
    {
        int bottomLeftX[3] = {1,1,0};
        int bottomLeftY[3] = {0,-1,-1};
        int k;
        for (k = 0; k <= 2; k++)
        {
            checkX = x + bottomLeftX[k];
            checkY = y + bottomLeftY[k];
            if (harta[checkX][checkY] == 1)
            {
                harta[x][y] = harta[checkX][checkY] = 0;
            }
        }
        return;
    }
    checkX = x + directionX;
    checkY = y + directionY;
    if (harta[checkX][checkY] == 1)
    {
        harta[x][y] = harta[checkX][checkY] = 0;
    }
}


void ObstacleAroundCenter(int centerI,int centerJ)
{
    int i,x,y;

    harta[centerI][centerJ] = 1;

    for (i = 0; i <=7; i++ )
    {
        x = centerI + dx[i];
        y = centerJ + dy[i];
        harta[x][y] = 1;
        CheckIfPassable( x, y, dx[i], dy[i] );
    }
}


int IsValidPosition(int obstacleIndex)
{
    int i,dist;
    for (i = 0; i < obstacleIndex; i++)
    {
        dist = abs(obstacleMatrix[0][i] - obstacleMatrix[0][obstacleIndex]) + abs(obstacleMatrix[1][i] - obstacleMatrix[1][obstacleIndex]);
        if (dist < minObstacleDistance)
            return 0;
    }
    return 1;
}

void GenerateObsatacoles()
{
    srand(time(NULL));
    float seedRand;
    int nrOfObstacles = (mapSize - 2) / 2;
    int i,centerPointI,centerPointJ;
    for (i = 0; i < nrOfObstacles; i++)
    {
        do {
        centerPointI = rand() % mapSize + 1;
        obstacleMatrix[0][i] = centerPointI;
        centerPointJ = rand() % mapSize + 1;
        obstacleMatrix[1][i] = centerPointJ;
        }while (IsValidPosition(i) == 0);

       // fout<<centerPointI<<' '<<centerPointJ<<endl;

        ObstacleAroundCenter(centerPointI,centerPointJ);
    }

}

void Awake()
{
    char PlayerImput,validInput = 0;
    cout<<"Introduceti dimensiunea hartii: "<<endl;
    cout<<"Press 'S' for small      Press 'M' for medium        Press 'L' for large"<<endl;
    while (validInput == 0)
    {
        cin>>PlayerImput;
        if (PlayerImput == 'S' || PlayerImput == 's')
        {
            mapSize = mapSizeSmall;
            validInput = 1;
        }
        else  if (PlayerImput == 'M' || PlayerImput == 'm')
        {
            mapSize = mapSizeMediu;
            validInput = 1;
        }
        else  if (PlayerImput == 'L' || PlayerImput == 'l')
        {
            mapSize = maxMapSize;
            validInput = 1;
        }
        else
            cout<<"Invalid Input";;
    }

    validInput = 0;

    cout<<"Introduceti numarul de tankuri (de la 2 la 4): ";
    while (validInput == 0)
    {
        cin>>PlayerImput;
        if (PlayerImput < '2' || PlayerImput > '4')
            cout<<"Invalid Input"<<endl;
        else
        {
            nrOfAgents = PlayerImput - '0';
            validInput = 1;
        }
    }

    validInput = 0;

    cout<<"Doriti sa controlati un tank: "<<endl;
    cout<<"Press 'Y' for yes        Press 'N' for no"<<endl;

    while (validInput == 0)
    {
        cin>>PlayerImput;
        if (PlayerImput == 'Y' || PlayerImput == 'y')
        {
            playerPlaying = true;
            validInput = 1;
        }
        else  if (PlayerImput == 'N' || PlayerImput == 'n')
        {
            playerPlaying = false;
            validInput = 1;
        }
        else
            cout<<"Invalid Input"<<endl;
    }

    validInput = 0;

    /*mapSize = maxMapSize;
    nrOfAgents = 2;
    cout<<nrOfAgents<<endl;*/
}


Vector2 FindClosestAvailable(int centerX,int centerY,int Radius)
{
    int x,y,xp = centerX,yp = centerY;
    Vector2 pozitie;

    if ( harta[centerX][centerY] == 0 )
    {
        pozitie.x = xp;
        pozitie.y = yp;
        return pozitie;
    }

    x = -Radius;
    xp = centerX + x;
    for (y = - Radius; y <= Radius; y++)
    {
        yp = centerY + y;
        if (xp < 1 || xp >= mapSize || yp < 1 || yp >= mapSize)
            continue;
        else if (harta[xp][yp] == 0)
        {
            pozitie.x = xp;
            pozitie.y = yp;
            return pozitie;
        }
    }

    y = Radius;
    yp = centerY + y;
    for (x = -Radius; x <= Radius; x++)
    {
        xp = centerX + x;
        if (xp < 1 || xp >= mapSize || yp < 1 || yp >= mapSize)
            continue;
        else if (harta[xp][yp] == 0)
        {
            pozitie.x = xp;
            pozitie.y = yp;
            return pozitie;
        }
    }
    x = Radius;
    xp = centerX + x;
    for (y = Radius; y >= -Radius; y--)
    {
        yp = centerY + y;
        if (xp < 1 || xp >= mapSize || yp < 1 || yp >= mapSize)
            continue;
        else if (harta[xp][yp] == 0)
        {
            pozitie.x = xp;
            pozitie.y = yp;
            return pozitie;
        }
    }
    y = -Radius;
    yp = centerY + y;
    for (x = Radius; x >= -Radius; x--)
    {
        xp = centerX + x;
        if (xp < 1 || xp >= mapSize || yp < 1 || yp >= mapSize)
            continue;
        else if (harta[xp][yp] == 0)
        {
            pozitie.x = xp;
            pozitie.y = yp;
            return pozitie;
        }
    }

    return FindClosestAvailable(centerX,centerY,Radius + 1);

}


void PlaceAgents()
{
    int targetX,targetY;
    Vector2 position;
   // cout<<nrOfAgents;
    if (nrOfAgents == 2)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = 2;

    }
    if (nrOfAgents == 3)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = 2;

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc


    }
    if (nrOfAgents == 4)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank in dreapta la mij

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank in stanga la mij

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc

        targetX = mapSize;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc
    }
}

void DrawMap()
{
    int i,j;
    for (i = 0; i <= mapSize; i++)
    {
        for (j = 0; j <= mapSize; j++)
        {
            if (harta[i][j] == 2)
                fout<<'T'<<' ';
            else if ( harta[i][j] == 0 )
                fout<<' '<<' ';
            else
                fout<<'X'<<' ';
        }
        fout<<endl;
    }
}

void Bordare()
{
    int i,j;
    for (j = 0; j < maxMapSize; j++ )
    {
        harta[0][j] = harta[mapSize][j] = 1;
    }
    for (i = 0; i < mapSize; i++)
    {
        harta[i][0] = harta[i][mapSize] = 1;
    }
}

void AstarAlgorithm(Vector2 startPosition, Vector2 targetPosition)
{
   int i; i++;
}

void SetWalkable()
{
    int  i,j;
    for ( i = 0; i <= maxMapSize; i++ )
    {
        for ( j = 0; j <= maxMapSize; j++ )
        {
            if (harta[i][j] == 1)
                wayPoints[i][j].walkable = false;
            else
                wayPoints[i][j].walkable = true;
        }
    }
}

void ShowWalkable()
{
    fout<<endl;
    int i,j;
    for (i = 0; i <= mapSize; i++)
    {
        for (j = 0; j <= mapSize; j++)
        {
            if (wayPoints[i][j].walkable == 0)
                fout<<'X'<<' ';
            else
                fout<<' '<<' ';
        }
        fout<<endl;
    }//this shit needs to be lafel with the base afisare;
}

int main()
{
    Awake();
    GenerateObsatacoles();
    Bordare();
    SetWalkable();
    PlaceAgents();
    DrawMap();
    //ShowWalkable();

    return 0;
}
