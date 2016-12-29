#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <windows.h>
#include <conio.h>
using namespace std;

ifstream fin("test.in");
ofstream fout("test.out");

#define maxMapSize 22
#define mapSizeSmall 12
#define mapSizeMediu 17
#define minObstacleDistance 4
bool gameOver;
int mapSize,harta[maxMapSize][maxMapSize],nrOfAgents, A,S,D,W;
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
    Vector2 nodeCoordonates;
    WayPointNode *prevPosNode;//asta nu e pt lista
    WayPointNode *nextNode;//simpla inlantuita e enough i think(i hope)
};

struct NodeList
{
    int listLenght;
    WayPointNode *first;
    WayPointNode *last;
};

NodeList openList,closedList;

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

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = 2;

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

    }
    if (nrOfAgents == 3)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = 2;

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc

        Agents[2].pozitie.x = position.x;
        Agents[2].pozitie.y = position.y;


    }
    if (nrOfAgents == 4)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank in dreapta la mij

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank in stanga la mij

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc

        Agents[2].pozitie.x = position.x;
        Agents[2].pozitie.y = position.y;

        targetX = mapSize;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = 2;//tank sus la mijloc

        Agents[3].pozitie.x = position.x;
        Agents[3].pozitie.y = position.y;
    }
}

void DrawMap()
{
     A=S=D=W=0;
    system("cls");
    int i,j;
    for (i = 0; i <= mapSize; i++)
    {
        for (j = 0; j <= mapSize; j++)
        {
            if (harta[i][j] == 9)
                cout<<'*'<<' ';
            else if (harta[i][j] == 2)
                cout<<'T'<<' ';
            else if ( harta[i][j] == 0 )
                cout<<' '<<' ';
            else
                cout<<'X'<<' ';
        }
        cout<<endl;
    }
 //   system("pause>nul");
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

void AddToList( NodeList &targetList, WayPointNode *node )
{
    if (targetList.listLenght == 0)
    {
        targetList.first = node;
        targetList.last = node;
        targetList.first->nextNode = NULL;
        targetList.last->nextNode = NULL;
    }
    else
    {
        targetList.last->nextNode = node;
        targetList.last = node;
    }
    targetList.listLenght++;
}

void RemoveFromList( NodeList &targetList, WayPointNode *targetnode )
{
    WayPointNode *currentNode, *prevNode, *nextNode;
    currentNode = targetList.first;
    prevNode = NULL;
    while (currentNode != targetnode)
    {
        prevNode = currentNode;
        currentNode = targetnode->nextNode;
    }
    nextNode = currentNode->nextNode;

    if (prevNode == NULL)//primul element e tinta
    {
        if (nextNode != NULL)
        {
            targetList.first = nextNode;
        }
        else
        {
            targetList.first = NULL;
            targetList.last = NULL;
        }
    }
    else
    {
        if (nextNode != NULL)
        {
            prevNode->nextNode = nextNode;
        }
        else
        {
            targetList.last = prevNode;
        }
    }
    targetList.listLenght--;
    delete(currentNode);
}

void FindSmallestFcost( NodeList targetLista, WayPointNode  *&smallestFcost )
{
    WayPointNode *currentNode = targetLista.first;
    smallestFcost = currentNode;
    int minFcost,currentFcost;
    minFcost = targetLista.first->fCost;

    while(currentNode != NULL)
    {
        currentNode = currentNode->nextNode;
        if (currentNode != NULL)
        {
            currentFcost = currentNode->fCost;
            if (currentFcost < minFcost)
            {
                minFcost = currentFcost;
                smallestFcost = currentNode;
            }
        }
    }
}

void FindNeighbours(WayPointNode *currentNode, WayPointNode *neighbors[4])
{
    int dx[] = {-1,0,1,0},dy[] = {0,1,0,-1},x,y,xp,yp;
    int i;
    x = currentNode->nodeCoordonates.x;
    y = currentNode->nodeCoordonates.y;
    for (int i = 0; i <= 3; i++ )
    {
        xp = x + dx[i];
        yp = y + dy[i];
        neighbors[i] = new WayPointNode(wayPoints[xp][yp]);
    }
}

bool IsContained(NodeList targetList, WayPointNode *targetNode)
{
    WayPointNode *currentNode;
    currentNode = targetList.first;
    while (currentNode != NULL)
    {
        if (currentNode == targetNode)
            return true;
        currentNode = currentNode->nextNode;
    }
    return false;
}

int DistanceBetweenNodes(Vector2 node1, Vector2 node2)
{
    int dist = 0;
    dist = abs(node1.x - node2.x) + abs(node1.y - node2.y);
    return dist;
}

struct PathOfNodes
{
    int pathLenght;
    Vector2 nodePath[maxMapSize * maxMapSize];
};

void TestDrawPath(PathOfNodes testPath)
{
    int i;
    for (i = 0; i < testPath.pathLenght; i++)
    {
        harta[testPath.nodePath[i].x][testPath.nodePath[i].y] = 9;
    }
}

PathOfNodes ConstructPath(WayPointNode *destination)
{
    PathOfNodes carare;
    WayPointNode *currentNode = destination;
    carare.pathLenght = 0;
    while (currentNode != NULL)
    {
        carare.nodePath[carare.pathLenght].x = currentNode->nodeCoordonates.x;
        carare.nodePath[carare.pathLenght].y = currentNode->nodeCoordonates.y;
        carare.pathLenght++;
        currentNode = currentNode->prevPosNode;
    }
    carare.pathLenght++;
    TestDrawPath(carare);
    return carare;
}

void AstarAlgorithm(Vector2 startPosition, Vector2 targetPosition)
{
    openList.listLenght = 0;
    openList.first = NULL;
    openList.last = NULL;
    closedList.listLenght = 0;
    closedList.first = NULL;
    closedList.last = NULL;
    int gCost,hCost,fCost,gPartial,hPartial,fPartial;

    WayPointNode *startNode,*targetNode,*currentNode;
    startNode = new WayPointNode( wayPoints[startPosition.x][startPosition.y] );
    targetNode = new WayPointNode( wayPoints[targetPosition.x][targetPosition.y] );
    if ( (*targetNode).walkable == false)
    {
        Vector2 targPoz = FindClosestAvailable(targetPosition.x, targetPosition.y, 1);
        targetNode = new WayPointNode ( wayPoints[targPoz.x][targPoz.y] );
    }
    currentNode = startNode;
    AddToList(openList,currentNode);

    gCost = DistanceBetweenNodes(currentNode->nodeCoordonates,startNode->nodeCoordonates);
    hCost = DistanceBetweenNodes(currentNode->nodeCoordonates,targetNode->nodeCoordonates);
    fCost = gCost + fCost;

    currentNode->gCost = gCost;
    currentNode->hCost = hCost;
    currentNode->fCost = fCost;
    currentNode->prevPosNode = NULL;
    int iter = 0;
    while ( openList.listLenght > 0 )
    {
        iter++;
        cout<<"list lenght: "<<openList.listLenght<<endl;
        FindSmallestFcost(openList,currentNode);
        cout<<"center Node: "<<currentNode->nodeCoordonates.x<<' '<<currentNode->nodeCoordonates.y<<' '<<iter<<endl;
        RemoveFromList(openList,currentNode);
        AddToList(closedList,currentNode);
        if (currentNode == targetNode)
        {
            //pathfound
            ConstructPath(currentNode);
            return;
        }
        WayPointNode *neighbous[4];
        FindNeighbours(currentNode,neighbous);
        for (int i =0; i <= 3; i++)
        {
            gPartial = DistanceBetweenNodes(neighbous[i]->nodeCoordonates,startNode->nodeCoordonates);
            hPartial = DistanceBetweenNodes(neighbous[i]->nodeCoordonates,targetNode->nodeCoordonates);
            fPartial = gPartial + hPartial;
            if ( neighbous[i]->walkable == false || IsContained(closedList,neighbous[i]) == true )
            {
                cout<<"unavailable nodes: "<<neighbous[i]->nodeCoordonates.x<<' '<<neighbous[i]->nodeCoordonates.y<<' '<<"is Contained: "<<IsContained(closedList,neighbous[i])<<" is walkable: "<<neighbous[i]->walkable<<endl;
                continue;
            }
            else if ( IsContained(openList,neighbous[i]) == false || fPartial < neighbous[i]->fCost)
            {
                //cout<<"xx"<<endl;
                cout<<"available nodes: "<<neighbous[i]->nodeCoordonates.x<<' '<<neighbous[i]->nodeCoordonates.y<<' '<<endl;
                neighbous[i]->gCost = gPartial;
                neighbous[i]->hCost = hPartial;
                neighbous[i]->fCost = fPartial;
                neighbous[i]->prevPosNode = currentNode;
                if ( IsContained(openList,neighbous[i]) == false )
                {
                    AddToList(openList,neighbous[i]);
                }
            }
        }

        cout<<endl<<endl;

    }

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

            wayPoints[i][j].nodeCoordonates.x = i;
            wayPoints[i][j].nodeCoordonates.y = j;
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
    if(D==1 && harta[Agents[0].pozitie.y][Agents[0].pozitie.x+1]==0){

        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=0;
        Agents[0].pozitie.x++;
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=2;
      //  tank.column++;
    }
    else if(A==1 && harta[Agents[0].pozitie.y][Agents[0].pozitie.x-1]==0){
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=0;
        Agents[0].pozitie.x--;
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=2;
      //  tank.column--;

    }
    else if(W==1 && harta[Agents[0].pozitie.y-1][Agents[0].pozitie.x]==0){
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=0;
        Agents[0].pozitie.y--;
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=2;
        //tank.line--;
    }
    else if(S==1 && harta[Agents[0].pozitie.y+1][Agents[0].pozitie.x]==0){
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=0;
        Agents[0].pozitie.y++;
        harta[Agents[0].pozitie.y][Agents[0].pozitie.x]=2;
       // tank.line++;
    }

}
int main()
{
    Awake();
    GenerateObsatacoles();
    Bordare();
  //  SetWalkable();
    PlaceAgents();
    Vector2 testPos1,testPos2;
    testPos1.x = Agents[0].pozitie.x;
    testPos1.y = Agents[0].pozitie.y;
    testPos2.x = mapSize / 2;
    testPos2.y = mapSize / 2;
    gameOver=false;
    while(!gameOver)
    {
       //  system("pause>nul");
        DrawMap();
        Input();
        Logic();
         //system("pause>nul");
        Sleep(100);
    }

  //  ShowWalkable();
//    AstarAlgorithm(testPos1,testPos2);

    return 0;
}
