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
#define projectileCode -1
#define powerupCode 3
#define detectionR 5
#define shootingR 3
#define tankMoveInterval 10;
#define tankHealthPoints 3;
#define maxProjectileNr 400
#define projectileMoveInterval 5
#define baseAttackSpeed 1
#define spawnFrequency 100
#define maxPowerupsnr 5
#define defaultDmg 1
#define dangerRange 3

int mapSize,harta[maxMapSize][maxMapSize],tanksPerUnitCount[maxMapSize][maxMapSize],nrOfAgents;
int frames = 0,firstAi;
int dx[] = {-1,-1,-1,0,1,1,1,0},dy[] = {-1,0,1,1,1,0,-1,-1};
bool playerPlaying;
bool gameRunning;

struct Vector2
{
    int x;
    int y;
};

struct powerUp
{
    Vector2 pozitie;
    int type;
};

powerUp powerupArray[ maxPowerupsnr ];

int activePowerups;

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

struct Proiectil
{
    int ownerId;
    int dmg;
    Vector2 moveDirection;
    Vector2 currentPozition;
};

Proiectil projectileArray[maxProjectileNr];

int activeProjectiles;

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
    int dmg;
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
            firstAi = 1;
            validInput = 1;
        }
        else  if (PlayerImput == 'N' || PlayerImput == 'n')
        {
            playerPlaying = false;
            firstAi = 0;
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
    currentTank.dmg = defaultDmg;
    currentTank.moveInterval = tankMoveInterval;
    currentTank.hasPath = false;
    currentTank.hasTarget = false;
    currentTank.rotatie.x = 0;
    currentTank.rotatie.y = 1;
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
            else if (harta[i][j] == tankCode)
                cout<<'T'<<' ';
            else if ( harta[i][j] == 0 )
                cout<<' '<<' ';
            else if ( harta[i][j] == projectileCode )
                cout<<'.'<<' ';
            else if ( harta[i][j] == powerupCode )
                cout<<'$'<<' ';
            else if ( harta[i][j] == powerupCode + 1 )
                cout<<'@'<<' ';
            else if ( harta[i][j] == powerupCode + 2 )
                cout<<'#'<<' ';
            else
                cout<<'X'<<' ';
        }
        cout<<endl;
    }
    for (i = 0; i < nrOfAgents; i++)
    {
        cout<<"Tank "<<i<<": ";
        for (j = 0; j < Agents[i].healthPoints; j++)
        {
            cout<<"* ";
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

void DestroyProjectile(int index)
{
    int i;
    Vector2 pozitie = projectileArray[index].currentPozition;
    //fout<<pozitie.x<<' '<<pozitie.y<<endl;
    /*if (index == activeProjectiles - 1)
    {
        activeProjectiles--;
        harta[pozitie.x][pozitie.y] = 0;
        return;
    }*/
    activeProjectiles--;
    for ( i = index; i < activeProjectiles; i++ )
    {
        projectileArray[i] = projectileArray[i + 1];
    }
    harta[pozitie.x][pozitie.y] = 0;
}

bool isOwnerTank(int projectileIndex, Vector2 pozitie)
{
    int ownerIndex = projectileArray[projectileIndex].ownerId;
    if ( Agents[ownerIndex].pozitie.x == pozitie.x && Agents[ownerIndex].pozitie.x == pozitie.y )
        return true;
    return false;
}

int FindTankAtLocation(Vector2 targetLocation)
{
    int i;
    for (i = 0; i < nrOfAgents; i++ )
    {
        if ( Agents[i].pozitie.x == targetLocation.x && Agents[i].pozitie.y == targetLocation.y )
        {
            return i;
        }
    }
    return 0;
}

void MoveProjectile(int index)
{
    if (frames % projectileMoveInterval == 0)
    {
        Vector2 nextPoz;
        nextPoz.x = projectileArray[index].currentPozition.x + projectileArray[index].moveDirection.x;
        nextPoz.y = projectileArray[index].currentPozition.y + projectileArray[index].moveDirection.y;
        if ( harta[nextPoz.x][nextPoz.y] == 1 || ( harta[nextPoz.x][nextPoz.y] == tankCode && isOwnerTank(index,nextPoz) == false ) )
        {
            DestroyProjectile(index);
            if ( harta[nextPoz.x][nextPoz.y] == tankCode )
            {
                int targetIndex;
                targetIndex = FindTankAtLocation(nextPoz);
                Agents[targetIndex].healthPoints -= projectileArray[index].dmg;
            }
        }
        else
        {
            Vector2 currentPozition = projectileArray[index].currentPozition;
            harta[currentPozition.x][currentPozition.y] = 0;
            projectileArray[index].currentPozition = nextPoz;
            harta[nextPoz.x][nextPoz.y] = projectileCode;
        }
    }
}

void ProcesProjectiles()
{
    int i;
    for (i = 0; i < activeProjectiles; i++)
    {
        MoveProjectile(i);
    }
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

void Shoot( int tankIndex )
{
    if ( frames % baseAttackSpeed == 0 )
    {
        activeProjectiles++;
        projectileArray[activeProjectiles].moveDirection = Agents[tankIndex].rotatie;
        projectileArray[activeProjectiles].currentPozition.x = Agents[tankIndex].pozitie.x + Agents[tankIndex].rotatie.x;
        projectileArray[activeProjectiles].currentPozition.y = Agents[tankIndex].pozitie.y + Agents[tankIndex].rotatie.y;
        Vector2 checkPoz = projectileArray[ activeProjectiles ].currentPozition;
        if (harta[checkPoz.x][checkPoz.y] == 1)
        {
            activeProjectiles--;
            return;
        }
        projectileArray[activeProjectiles].dmg = Agents[tankIndex].dmg;
        projectileArray[activeProjectiles].ownerId = tankIndex;
    }
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

struct Vector2Bool
{
    bool boolValue;
    Vector2 Vector2value;
};

bool checkProjectileParent( Vector2 projectilePosition, int tankIndex )
{
    int i;
    for (i = 0; i < activeProjectiles; i++)
    {
        if ( projectileArray[i].ownerId == tankIndex )
            return true;
    }
    return false;
}

Vector2 MultiplyVector2( Vector2 baseVec, int multiplyer )
{
    Vector2 aux;
    aux.x = baseVec.x * multiplyer;
    aux.y = baseVec.y * multiplyer;
    return aux;
}

Vector2Bool CheckNeighbour1 (Vector2 vect,Vector2 initPoz,int tankIndex)
{
    Vector2 neighbour1;
    if ( vect.x == 0 )
    {
        neighbour1.x = vect.y;
        neighbour1.y = 0;
    }
    else
    {
        neighbour1.x = 0;
        neighbour1.y = vect.x;
    }

    neighbour1.x += initPoz.x;
    neighbour1.x += initPoz.y;

    Vector2Bool availablePos;
    if ( harta[neighbour1.x][neighbour1.y] == 1 || ( harta[neighbour1.x][neighbour1.y] == projectileCode && checkProjectileParent( neighbour1,tankIndex == false ) )|| harta[neighbour1.x][neighbour1.y] == tankCode )
    {
        availablePos.boolValue = false;
    }
    else
    {
        availablePos.boolValue = true;
        availablePos.Vector2value = neighbour1;
    }
    return availablePos;
}

Vector2Bool CheckNeighbour2( Vector2 vect,Vector2 initPoz,int tankIndex )
{
    Vector2 neighbour1;
    if ( vect.x == 0 )
    {
        neighbour1.x = -vect.y;
        neighbour1.y = 0;
    }
    else
    {
        neighbour1.x = 0;
        neighbour1.y = -vect.x;
    }

    neighbour1.x += initPoz.x;
    neighbour1.x += initPoz.y;

    Vector2Bool availablePos;
    if ( harta[neighbour1.x][neighbour1.y] == 1 || ( harta[neighbour1.x][neighbour1.y] == projectileCode && checkProjectileParent( neighbour1,tankIndex == false ) ) || harta[neighbour1.x][neighbour1.y] == tankCode )
    {
        availablePos.boolValue = false;
    }
    else
    {
        availablePos.boolValue = true;
        availablePos.Vector2value = neighbour1;
    }
    return availablePos;
}

Vector2Bool CheckOpositeNeighbour( Vector2 vect,Vector2 initPoz ,int tankIndex)
{
    Vector2 neighbour1;
    neighbour1.x = -vect.x;
    neighbour1.y = -vect.y;
    neighbour1.x += initPoz.x;
    neighbour1.x += initPoz.y;

    Vector2Bool availablePos;
    if ( harta[neighbour1.x][neighbour1.y] == 1 || ( harta[neighbour1.x][neighbour1.y] == projectileCode && checkProjectileParent( neighbour1,tankIndex == false ) )|| harta[neighbour1.x][neighbour1.y] == tankCode )
    {
        availablePos.boolValue = false;
    }
    else
    {
        availablePos.boolValue = true;
        availablePos.Vector2value = neighbour1;
    }
    return availablePos;
}


Vector2Bool inDanger( int tankIndex )
{
    int i;
    Vector2 sus,jos,stanga,dreapta,aux;
    sus.x = -1; sus.y = 0;
    dreapta.x = 0; dreapta.y = 1;
    jos.x = 1; sus.y = 0;
    stanga.x = 0; stanga.y = -1;
    Vector2Bool availablePosition;

    for (i = 1; i <= dangerRange; i++ )
    {
        aux = MultiplyVector2( stanga, i );
        if ( harta[aux.x][aux.y] == projectileCode && checkProjectileParent( aux, tankIndex ) )
        {
            availablePosition = CheckNeighbour1( stanga, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckNeighbour2( stanga, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckOpositeNeighbour( stanga, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition.boolValue = false;
            return availablePosition;
        }//check vecin la stanga pt proiectil

        aux = MultiplyVector2( sus, i );
        if ( harta[aux.x][aux.y] == projectileCode && checkProjectileParent( aux, tankIndex ) )
        {
            availablePosition = CheckNeighbour1( sus, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckNeighbour2( sus, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckOpositeNeighbour( sus, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition.boolValue = false;
            return availablePosition;
        }//check vecin la sus pt proiectil

        aux = MultiplyVector2( dreapta, i );
        if ( harta[aux.x][aux.y] == projectileCode && checkProjectileParent( aux, tankIndex ) )
        {
            availablePosition = CheckNeighbour1( dreapta, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckNeighbour2( dreapta, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckOpositeNeighbour( dreapta, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition.boolValue = false;
            return availablePosition;
        }//check vecin la dreapta pt proiectil

        aux = MultiplyVector2( jos, i );
        if ( harta[aux.x][aux.y] == projectileCode && checkProjectileParent( aux, tankIndex ) )
        {
            availablePosition = CheckNeighbour1( jos, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckNeighbour2( jos, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition = CheckOpositeNeighbour( jos, Agents[tankIndex].pozitie,tankIndex );
            if  (availablePosition.boolValue == true)
                return availablePosition;

            availablePosition.boolValue = false;
            return availablePosition;
        }//check vecin la jos pt proiectil
    }
    availablePosition.boolValue = false;
    return availablePosition;
}

void DestroyTank(int tankIndex)
{
    nrOfAgents--;
    int i;
    tanksPerUnitCount[ Agents[tankIndex].pozitie.x ][ Agents[tankIndex].pozitie.y ]--;
    if ( tanksPerUnitCount[ Agents[tankIndex].pozitie.x ][ Agents[tankIndex].pozitie.y ] == 0 )
    {
        harta[ Agents[tankIndex].pozitie.x ][ Agents[tankIndex].pozitie.y ] = 0;
    }
    for (i = tankIndex; i < nrOfAgents; i++)
    {
        Agents[i] = Agents[i+1];
    }
}

void TankAI( Tank &currentTank,int tankIndex )
{
    if ( currentTank.healthPoints <= 0 )
    {
        DestroyTank(tankIndex);
    }

    /*Vector2Bool availablePos;
    availablePos = inDanger( tankIndex );
    if ( availablePos.boolValue == true )
    {
        fout<<"tank "<<tankIndex<<" inDanger";
        AstarAlgorithm( currentTank, currentTank.pozitie, availablePos.Vector2value );
    }
    else*/
    if ( currentTank.hasTarget )
    {
        if ( DistanceBetweenNodes( currentTank.pozitie, currentTank.pointOfInterest ) <= currentTank.shootingRange && harta[ currentTank.pointOfInterest.x ][ currentTank.pointOfInterest.y ] == tankCode )
        {
            if ( CanShoot( currentTank.pozitie, currentTank.pointOfInterest ) )
            {
                Shoot( tankIndex );
                //int glz;
                //glz++;
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
    for (i = firstAi; i < nrOfAgents; i++)
    {
        LookForTargets( Agents[i] );
        TankAI ( Agents[i], i );
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
        if ( !(playerMoveDirection.x == 0 && playerMoveDirection.y == 0) )
        {
            player.rotatie = playerMoveDirection;
        }

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

int pwframes;

void DestroyPowerup( int index )
{
    Vector2 destroyedPoz = powerupArray[index].pozitie;
    harta[destroyedPoz.x][destroyedPoz.y] = 0;
   /* if ( index == activePowerups - 1 )
    {
        activePowerups--;
        return;
    }*/
    int i;
    for ( i = index; i < activePowerups; i++ )
    {
        powerupArray[i] = powerupArray[i + 1];
    }
    activePowerups--;
}

bool EqualVectors( Vector2 vec1, Vector2 vec2 )
{
    if ( vec1.x == vec2.x && vec1.y == vec2.y )
        return true;
    return false;
}

void ConsumePowerUp(int index,int type)
{
     if(type==3)
           Agents[index].healthPoints++;
     else
        if(type==2)Agents[index].dmg++;
     else
     if(type==1){Agents[index].dmg+=2;Agents[index].healthPoints--;}

}

void CheckCollision()
{
    int i,j;
    for (i = 0; i < nrOfAgents; i++ )
    {
        for ( j = 0; j < activePowerups; j++ )
        {
            if ( EqualVectors( Agents[i].pozitie , powerupArray[j].pozitie ) == true )
            {
                //fout<<"AGENT"<<' '<<i<<' '<<Agents[i].pozitie.x<<' '<<Agents[i].pozitie.y<<
                //fout<<"ACTIVE POWERUPS:"<<' '<<activePowerups<<endl;
                DestroyPowerup( j );
                harta[Agents[i].pozitie.x][Agents[i].pozitie.y]=tankCode;
                //ConsumePowerUp();
            }
        }
    }
}

void PlacePowerups()
{
    int i;
    for (i = 0; i < activePowerups; i++)
    {
        harta[ powerupArray[i].pozitie.x ][ powerupArray[i].pozitie.y ] = powerupCode+powerupArray[i].type-1;
    }
}

void SpawnPowerups()
{
    if ( pwframes > spawnFrequency )
    {
        pwframes = 0;
        activePowerups++;
        //fout<<activePowerups<<endl;
        if ( activePowerups > maxPowerupsnr - 1 )
        {
            DestroyPowerup(0);
            activePowerups = maxPowerupsnr - 1;
        }
        int x,y;
        x = rand() % mapSize;
        y = rand() % mapSize;
        Vector2 pwpoz;
        pwpoz.x = x;
        pwpoz.y = y;
        if ( harta[ pwpoz.x ][ pwpoz.y ] != 0 )
        {
            pwpoz = FindClosestAvailable(x,y,1);
        }
        powerupArray[ activePowerups ].pozitie = pwpoz;
        powerupArray[ activePowerups ].type = rand()%3 + 1;
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
            ProcesProjectiles();
            //Shoot(0);
            ProccesAI();
            Input();
            MovePlayer(Agents[0]);
            SpawnPowerups();
            PlacePowerups();
            CheckCollision();
            DrawMap();
            frames++;
            pwframes++;
            timeCounter = 0;
        }
        if ( (double) framesCounter > (double) 1 )
        {
            //fout<<"frames per second: "<<frames<<endl;
            fout<<nrOfAgents<<endl;
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
    /*char pls[50][50]={"000000000000",
                      "000000000000",
                      "000000000000",
                      "000000000000",
    };
    bool isGG=false;
    while(!isGG)
    {
        system("cls");
        for(int i=0;i<4;i++)cout<<pls[i]<<endl;}*/
    return 0;
}
