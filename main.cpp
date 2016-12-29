#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <conio.h>
#include <windows.h>

using namespace std;

ifstream fin("test.in");
ofstream fout("test.out");

#define maxMapSize 22
#define mapSizeSmall 12
#define mapSizeMediu 17
#define minObstacleDistance 4
#define numberOfFrames 60
//tankz info
#define tankCode 2
#define detectionR 5
#define shootingR 3
#define tankMoveInterval 30;
#define tankHealthPoints 3;


int mapSize,harta[maxMapSize][maxMapSize],tanksPerUnitCount[maxMapSize][maxMapSize],nrOfAgents;
int frames = 0;
int dx[] = {-1,-1,-1,0,1,1,1,0},dy[] = {-1,0,1,1,1,0,-1,-1};
bool playerPlaying;
bool gameRunning;

struct Vector2
{
    int x;
    int y;
};

Vector2 playerMoveDirection;

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
    Vector2 prevPosition;
};

struct NodeList
{
    int lenght;
    WayPointNode nodes[mapSizeMediu * mapSizeMediu];
};

NodeList openList,closedList;

WayPointNode wayPoints[maxMapSize][maxMapSize];

struct  Path
{
    int lenght;
    Vector2 nodes[ mapSizeMediu * mapSizeMediu ];
};

struct Tank
{
    Vector2 pozitie;
    Vector2 rotatie;
    Vector2 pointOfInterest;
    int moveInterval;
    int healthPoints;
    int detectionRange;
    int shootingRange;
    bool hasTarget;
    bool hasPath;
    int pathIndex;
    Path currentPath;
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
            cout<<"Invalid Input";
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
    gameRunning = true;

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

void InitializeAgents( Tank &currentTank )
{
    currentTank.detectionRange = detectionR;
    currentTank.shootingRange = shootingR;
    currentTank.healthPoints = tankHealthPoints;
    currentTank.moveInterval = tankMoveInterval;
    currentTank.hasPath = false;
    currentTank.hasTarget = false;
    if (currentTank.pozitie.x < mapSize / 2)
    {
        currentTank.rotatie.x = 1;
    }
    if ( currentTank.pozitie.x >= mapSize / 2 )
    {
        currentTank.rotatie.x = -1;
    }

    if (currentTank.pozitie.y < mapSize / 2)
    {
        currentTank.rotatie.y = 1;
    }

    if (currentTank.pozitie.y >= mapSize / 2)
    {
        currentTank.rotatie.y = -1;
    }

    tanksPerUnitCount[ currentTank.pozitie.x ][ currentTank.pozitie.y ] = 1;

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
        harta[position.x][position.y] = tankCode;

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = tankCode;

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

    }
    if (nrOfAgents == 3)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);

        harta[position.x][position.y] = tankCode;

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;//tank sus la mijloc

        Agents[2].pozitie.x = position.x;
        Agents[2].pozitie.y = position.y;


    }
    if (nrOfAgents == 4)
    {
        targetX = mapSize / 2 + 1;
        targetY = 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;//tank in dreapta la mij

        Agents[0].pozitie.x = position.x;
        Agents[0].pozitie.y = position.y;

        targetX = mapSize / 2 + 1;
        targetY = mapSize;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;//tank in stanga la mij

        Agents[1].pozitie.x = position.x;
        Agents[1].pozitie.y = position.y;

        targetX = 1;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;//tank sus la mijloc

        Agents[2].pozitie.x = position.x;
        Agents[2].pozitie.y = position.y;

        targetX = mapSize;
        targetY = mapSize / 2 + 1;
        position = FindClosestAvailable(targetX,targetY,1);
        harta[position.x][position.y] = tankCode;//tank sus la mijloc

        Agents[3].pozitie.x = position.x;
        Agents[3].pozitie.y = position.y;
    }

    for (int i = 0; i < nrOfAgents; i++)
    {
        InitializeAgents(Agents[i]);
    }
}

void DrawMap()
{
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

bool EqualNodes( WayPointNode node1, WayPointNode node2 )
{
    if ( node1.nodeCoordonates.x == node2.nodeCoordonates.x && node1.nodeCoordonates.y == node2.nodeCoordonates.y )
        return true;
    return false;
}

void AddToList( NodeList &targetList ,WayPointNode Node )
{
    targetList.nodes[targetList.lenght] = Node;
    targetList.lenght++;
}

void RemoveFromList( NodeList &targetList, WayPointNode Node )
{
    int i,j;
    for (i = 0; i < targetList.lenght; i++)
    {
        if (  EqualNodes( targetList.nodes[i], Node ) == true )
        {
            for (j = i; j < targetList.lenght - 1; j++)
            {
                targetList.nodes[j] = targetList.nodes[j+1];
            }
        }
    }
    targetList.lenght--;
}

bool IsContained( NodeList targetList, WayPointNode Node )
{
    int i;
    for (i = 0; i < targetList.lenght; i++)
        if (EqualNodes( targetList.nodes[i], Node ))
            return true;
    return false;
}

WayPointNode FindSmallestFcost( NodeList targetList )
{
    int i,minF;
    WayPointNode minFnode;

    minF = targetList.nodes[0].fCost;
    minFnode = targetList.nodes[0];

    for (i = 1; i < targetList.lenght; i++)
    {
        if ( targetList.nodes[i].fCost < minF )
        {
            minF = targetList.nodes[i].fCost;
            minFnode = targetList.nodes[i];
        }
    }
    return minFnode;
}

void FindNeighbours( WayPointNode centerNode, WayPointNode neighbours[4] )
{
    int x,y, xp[] = {-1,0,1,0}, yp[] = {0,1,0,-1};
    x = centerNode.nodeCoordonates.x;
    y = centerNode.nodeCoordonates.y;
    int i;
    for ( i = 0; i <= 3; i++ )
    {
        neighbours[i] = wayPoints[ x + xp[i] ] [ y + yp[i] ];
    }
}

void ReversePath( Path &targetPath )
{
    int pathLengh = targetPath.lenght,i;
    Vector2 aux;
    for (i = 0; i < pathLengh / 2; i++)
    {
        aux = targetPath.nodes[i];
        targetPath.nodes[i] = targetPath.nodes[ pathLengh - 1 - i ];
        targetPath.nodes[ pathLengh - 1 - i ] = aux;
    }
}

void ConstructPath(Tank &pathRequester, WayPointNode targetNode )
{
    WayPointNode currentNode;
    currentNode = wayPoints[targetNode.nodeCoordonates.x][targetNode.nodeCoordonates.y];

    Path pathFound;
    pathFound.lenght = 0;

    //fout<<endl<<endl;
    while (1)
    {
        //fout<<currentNode.nodeCoordonates.x<<' '<<currentNode.nodeCoordonates.y<<endl;
        if (currentNode.prevPosition.x == -1 && currentNode.prevPosition.y == -1)
            break;
        else
        {
            //harta[currentNode.nodeCoordonates.x][currentNode.nodeCoordonates.y] = 7;
            pathFound.nodes[pathFound.lenght].x = currentNode.nodeCoordonates.x;
            pathFound.nodes[pathFound.lenght].y = currentNode.nodeCoordonates.y;
            pathFound.lenght++;
            currentNode = wayPoints[currentNode.prevPosition.x][currentNode.prevPosition.y];
        }
    }

    ReversePath(pathFound);
    pathRequester.currentPath = pathFound;
    pathRequester.hasPath = true;
    pathRequester.pathIndex = 0;
}

int DistanceBetweenNodes(Vector2 node1, Vector2 node2)
{
    int dist = 0;
    dist = abs(node1.x - node2.x) + abs(node1.y - node2.y);
    return dist;
}

void AstarAlgorithm( Tank &pathReqester , Vector2 startPos, Vector2 targetPostition )
{
    int i;
    int gCost,hCost,fCost;
    WayPointNode startingNode,currentNode,targetNode;
    startingNode = wayPoints[startPos.x][startPos.y];
    currentNode = startingNode;
    if ( wayPoints[targetPostition.x][targetPostition.y].walkable == true )
    {
        targetNode = wayPoints[targetPostition.x][targetPostition.y];
    }
    else
    {
        Vector2 closestTarget;
        closestTarget = FindClosestAvailable(targetPostition.x,targetPostition.y,1);
        targetNode = wayPoints[closestTarget.x][closestTarget.y];
    }
    openList.lenght = 0;
    closedList.lenght = 0;

    gCost = DistanceBetweenNodes(currentNode.nodeCoordonates,startingNode.nodeCoordonates);
    hCost = DistanceBetweenNodes(currentNode.nodeCoordonates,targetNode.nodeCoordonates);
    fCost = gCost + hCost;
    startingNode.gCost = gCost;
    startingNode.hCost = hCost;
    startingNode.fCost = fCost;

    wayPoints[startPos.x][startPos.y].prevPosition.x = -1;
    wayPoints[startPos.x][startPos.y].prevPosition.y = -1;

    AddToList(openList,startingNode);

    while ( openList.lenght > 0 )
    {
        currentNode = FindSmallestFcost(openList);
        RemoveFromList(openList,currentNode);
        AddToList(closedList,currentNode);

        if ( EqualNodes(currentNode,targetNode) == true)
        {
            ConstructPath(pathReqester,targetNode);
            return;
        }

        WayPointNode neighbours[4];
        WayPointNode currentNeighbour;
        FindNeighbours( currentNode, neighbours);
        for (i = 0; i <= 3; i++)
        {
            currentNeighbour = neighbours[i];
            gCost = DistanceBetweenNodes(currentNeighbour.nodeCoordonates,startingNode.nodeCoordonates);
            hCost = DistanceBetweenNodes(currentNeighbour.nodeCoordonates,targetNode.nodeCoordonates);
            fCost = gCost + hCost;
            if ( currentNeighbour.walkable == false || IsContained(closedList,currentNeighbour) == true )
            {
                continue;
            }
            else if ( fCost < currentNeighbour.fCost || IsContained(openList,currentNeighbour) == false )
            {
                wayPoints[ currentNeighbour.nodeCoordonates.x ][ currentNeighbour.nodeCoordonates.y ].fCost = fCost;
                wayPoints[ currentNeighbour.nodeCoordonates.x ][ currentNeighbour.nodeCoordonates.y ].hCost = hCost;
                wayPoints[ currentNeighbour.nodeCoordonates.x ][ currentNeighbour.nodeCoordonates.y ].gCost = gCost;
                wayPoints[ currentNeighbour.nodeCoordonates.x ][ currentNeighbour.nodeCoordonates.y ].prevPosition = currentNode.nodeCoordonates;
                neighbours[i].fCost = fCost;
                neighbours[i].hCost = hCost;
                neighbours[i].gCost = gCost;
                neighbours[i].prevPosition = currentNode.prevPosition;
                if ( IsContained(openList,currentNeighbour) == false )
                {
                    AddToList(openList,currentNeighbour);
                }
            }
        }
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

void ClearConsole()
{
    system("cls");
}

bool EqualButCloser(Vector2 currentPosition, Vector2 optimalPoint,int i, int j )
{
    if ( harta[optimalPoint.x][optimalPoint.y] == harta[i][j] )
    {
        Vector2 auxV2;
        auxV2.x = i;
        auxV2.y = j;
        if ( DistanceBetweenNodes(currentPosition,optimalPoint) < DistanceBetweenNodes(currentPosition,auxV2) )
            return true;
    }
    return false;
}

void LookForTargets( Tank &currentTank )
{
    int i,j,x,y;
    Vector2 currentPointOfInterest;
    Vector2 optimalPointOfInterest;
    optimalPointOfInterest.x = -1;
    optimalPointOfInterest.y = -1;
    int currentRay;
    x = currentTank.pozitie.x;
    y = currentTank.pozitie.y;
    currentRay = currentTank.detectionRange;
    for (i = x - currentRay; i <= x + currentRay; i++)
    {
        for ( j = y - currentRay; j <= y + currentRay; j++ )
        {
            if ( i > 0 && i < mapSize && j > 0 && j < mapSize )
            {
                if ( i == x && j == y )
                    continue;
                else
                    {
                        if ( harta[i][j] > 1 )
                        {
                            currentPointOfInterest.x = i;
                            currentPointOfInterest.y = j;
                            if ( optimalPointOfInterest.x == -1 && optimalPointOfInterest.y == -1 || harta[ optimalPointOfInterest.x ][ optimalPointOfInterest.y ] < harta[i][j] || EqualButCloser( currentTank.pozitie, optimalPointOfInterest, i, j) == true )
                            {
                                optimalPointOfInterest = currentPointOfInterest;
                            }
                        }
                    }
            }
        }
    }

    currentTank.pointOfInterest = optimalPointOfInterest;
    if (optimalPointOfInterest.x != -1 && optimalPointOfInterest.y != -1)
    {
        currentTank.hasTarget = true;
    }
    else
    {
        currentTank.hasTarget = false;
    }
}

void SetRotation( Tank &currentTank ,Vector2 nextPos)
{
    Vector2 currentPos = currentTank.pozitie;
    if ( currentPos.x + -1 == nextPos.x && currentPos.y + 0 == nextPos.y )
    {
        currentTank.rotatie.x = -1;
        currentTank.rotatie.y = 0;
    }

    if ( currentPos.x + 0 == nextPos.x && currentPos.y + 1 == nextPos.y )
    {
        currentTank.rotatie.x = 0;
        currentTank.rotatie.y = 1;
    }

    if ( currentPos.x + 1 == nextPos.x && currentPos.y + 0 == nextPos.y )
    {
        currentTank.rotatie.x = 1;
        currentTank.rotatie.y = 0;
    }

    if ( currentPos.x + 0 == nextPos.x && currentPos.y + -1 == nextPos.y )
    {
        currentTank.rotatie.x = 0;
        currentTank.rotatie.y = -1;
    }
}

void MoveOneStep( Tank &currentTank )
{
    Vector2 nextPos = currentTank.currentPath.nodes[currentTank.pathIndex];
    if (frames % currentTank.moveInterval == 0 && harta[nextPos.x][nextPos.y] != tankCode)
    {

        Vector2 prevPos = currentTank.pozitie;
        tanksPerUnitCount[ prevPos.x ][ prevPos.y ]--;
        if ( tanksPerUnitCount[ prevPos.x ][ prevPos.y ] == 0 )
        {
            harta[prevPos.x][prevPos.y] = 0;
        }
        nextPos = currentTank.currentPath.nodes[currentTank.pathIndex];
        Vector2 currentPos = currentTank.pozitie;
        tanksPerUnitCount[nextPos.x][nextPos.y]++;

        SetRotation(currentTank,nextPos);

        currentTank.pozitie = currentTank.currentPath.nodes[currentTank.pathIndex];
        harta[ currentTank.pozitie.x ][ currentTank.pozitie.y ] = tankCode;
        currentTank.pathIndex++;
        if ( currentTank.pathIndex >= currentTank.currentPath.lenght )
        {
            currentTank.hasPath = false;
        }
    }
}

void Shoot()
{
    //cout<<"pew pew"<<endl;
}

bool CanShoot(Vector2 poz1, Vector2 poz2)
{
    int i,mine,maxe;
    if ( poz1.x == poz2.x )
    {
        if (poz1.y > poz2.y)
        {
            maxe = poz1.y;
            mine = poz2.y;
        }
        else
        {
            maxe = poz2.y;
            mine = poz1.y;
        }

        for (i = mine + 1; i < maxe; i++ )
        {
            if (harta[poz1.x][i] == tankCode)
            {
                return false;
            }
        }
        return true;
    }

    if ( poz1.y == poz2.y )
    {
        if (poz1.x > poz2.x)
        {
            maxe = poz1.x;
            mine = poz2.x;
        }
        else
        {
            maxe = poz2.x;
            mine = poz1.x;
        }

        for (i = mine + 1; i < maxe; i++ )
        {
            if (harta[i][poz1.y] == tankCode)
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

void TankAI( Tank &currentTank )
{
    if ( currentTank.hasTarget )
    {
        if ( DistanceBetweenNodes( currentTank.pozitie, currentTank.pointOfInterest ) <= currentTank.shootingRange && harta[ currentTank.pointOfInterest.x ][ currentTank.pointOfInterest.y ] == tankCode )
        {
            if ( CanShoot( currentTank.pozitie, currentTank.pointOfInterest ) )
            {
                Shoot();
            }
        }
        else
        {
            AstarAlgorithm( currentTank, currentTank.pozitie, currentTank.pointOfInterest );
            MoveOneStep( currentTank );
        }
    }
    else
    {
        if (!currentTank.hasTarget)
        {
            Vector2 randPos;
            randPos.x = rand() % mapSize;
            randPos.y = rand() % mapSize;
            AstarAlgorithm( currentTank, currentTank.pozitie, randPos );
        }
        MoveOneStep(currentTank);
    }
}

void ProccesAI()
{
    int i;
    for (i = 1; i < nrOfAgents; i++)
    {
        LookForTargets( Agents[i] );
        TankAI ( Agents[i] );
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
                    playerMoveDirection.x = 0;
                    playerMoveDirection.y = 1;
               }
                break;
            }

            case 'w':
            {
                {
                    playerMoveDirection.x = -1;
                    playerMoveDirection.y = 0;
                }
            break;
            }

            case 'a':
            {
                {
                    playerMoveDirection.x = 0;
                    playerMoveDirection.y = -1;
                }
            break;
            }

            case 's':
            {
                {
                    playerMoveDirection.x = 1;
                    playerMoveDirection.y = 0;
                }
            break;
            }

        }

    }
    else
    {
        playerMoveDirection.x = 0;
        playerMoveDirection.y = 0;
    }
}

void MovePlayer(Tank &player)
{
    if ( frames % player.moveInterval == 0 )
    {
        //fout<<"move player"<<endl;
        Vector2 nextPoz;
        nextPoz.x = player.pozitie.x + playerMoveDirection.x;
        nextPoz.y = player.pozitie.y + playerMoveDirection.y;

        if ( harta[ nextPoz.x ][ nextPoz.y ] != 1 )
        {
            tanksPerUnitCount[ player.pozitie.x ][ player.pozitie.y ]--;
            if ( tanksPerUnitCount[ player.pozitie.x ][ player.pozitie.y ] == 0 )
            {
                harta [ player.pozitie.x ][ player.pozitie.y ] = 0;
            }
            tanksPerUnitCount[nextPoz.x][nextPoz.y]++;
            harta[nextPoz.x][nextPoz.y] = tankCode;
            player.pozitie = nextPoz;
            //fout<<nextPoz.x<<' '<<nextPoz.y<<endl;
           // fout<<player.pozitie.x<<' '<<player.pozitie.y<<endl<<endl;
        }
    }
}

void Update()
{
    double threshold = 1 / (double) numberOfFrames;
    double currentTimer,prevTimer,deltaTime,timeCounter = 0,framesCounter = 0,refreshCounter = 0;
    currentTimer = (double) clock() / (double) CLOCKS_PER_SEC;
    prevTimer = (double)currentTimer;
    while (gameRunning)
    {
        currentTimer = (double) clock() / (double) CLOCKS_PER_SEC;
        deltaTime = (double)currentTimer - (double) prevTimer;
        prevTimer = (double) currentTimer;
        timeCounter += (double) deltaTime;
        framesCounter += (double) deltaTime;
        refreshCounter += (double) deltaTime;
        if (timeCounter >=  threshold )
        {
            /*if (refreshCounter >= threshold * (double)24)
            {
                ClearConsole();
                DrawMap();
                refreshCounter = 0;
            }*/
            ClearConsole();
            ProccesAI();
            Input();
            MovePlayer(Agents[0]);
            DrawMap();
            frames++;
            timeCounter = 0;
        }
        if ( (double) framesCounter > (double) 1 )
        {
            //fout<<"frames per second: "<<frames<<endl;
            framesCounter = 0;
            frames = 0;
        }
    }

}

int main()
{
    Awake();
    GenerateObsatacoles();
    Bordare();
    SetWalkable();
    PlaceAgents();
    Vector2 testPos1,testPos2;
    testPos1.x = Agents[0].pozitie.x;
    testPos1.y = Agents[0].pozitie.y;
   // testPos2.x = mapSize / 2;
    //testPos2.y = mapSize / 2;

    testPos2.x = rand() % mapSize - 1 + 1;
    testPos2.y = rand() % mapSize - 1 + 1;
    //ShowWalkable();
    //AstarAlgorithm(testPos1,testPos2);
   // DrawMap();
   // fout<<endl;
   // fout<<"target pos x: "<<testPos2.x<<"   target pos y: "<<testPos2.y;
    //fout<<endl<<(double) ( (1 / (double) numberOfFrames)  )<<endl;
    Update();

    return 0;
}
