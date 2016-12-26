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
    Vector2 nodeCoordonates;
    WayPointNode *reachedFrom;//asta nu e pt lista
    WayPointNode *nextNode;
    WayPointNode *prevNode;//lista dublu inlantuita
};


struct NodeList
{
    WayPointNode *terminalStanga,*terminalDreapta,*first,*last;
    int lenght;
} openList,closedList;

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
    int i,j;
    for (i = 0; i <= mapSize; i++)
    {
        for (j = 0; j <= mapSize; j++)
        {
            if (harta[i][j] == 9)
                fout<<'*'<<' ';
            else if (harta[i][j] == 2)
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

void AfisareLista( NodeList targetList )
{
    WayPointNode *curr;
    curr = targetList.first;
    cout<<endl;
    while (curr != targetList.terminalDreapta)
    {
        cout<<curr->nodeCoordonates.x<<' '<<curr->nodeCoordonates.y<<" - ";
        curr = curr->nextNode;
    }
    cout<<endl;
}

bool EqualNodes( WayPointNode *node1, WayPointNode *node2 )
{
    if ( node1->nodeCoordonates.x == node2->nodeCoordonates.x &&  node1->nodeCoordonates.y == node2->nodeCoordonates.y )
    {
        return true;
    }
    return  false;
}

void InitializareListe ( NodeList &targetList )
{
    targetList.lenght = 0;
    targetList.terminalStanga = new WayPointNode;
    targetList.terminalDreapta = new WayPointNode;//creez terminalele

    targetList.terminalStanga->prevNode = NULL;
    targetList.terminalStanga->nextNode = targetList.terminalDreapta;//initializez termianlul stanga

    targetList.terminalDreapta->prevNode = targetList.terminalStanga;
    targetList.terminalDreapta->nextNode = NULL;//initializez termianl dreapta

    targetList.first = NULL;
    targetList.last = NULL;//initializez primul si ultimul element cu null
}

void AddNode( NodeList &targetList, WayPointNode *newNode )
{
    if (targetList.lenght == 0)
    {
        targetList.first = newNode;

        targetList.first->prevNode = targetList.terminalStanga;
        targetList.terminalStanga->nextNode = targetList.first;//modific relatiile terminal stanga - first

        targetList.first->nextNode = targetList.terminalDreapta;
        targetList.terminalDreapta->prevNode = targetList.first;//modific relatiile terminal dreapta - last

        targetList.last = targetList.first;
    }
    else
    {
        targetList.last->nextNode = newNode;
        newNode->prevNode = targetList.last;
        newNode->nextNode = targetList.terminalDreapta;
        targetList.terminalDreapta->prevNode = newNode;

        targetList.last = newNode;
    }

    targetList.lenght++;
}

void RemoveNode( NodeList &targetList, WayPointNode *toRemove )
{
    WayPointNode *currentNode = targetList.first;
    while ( EqualNodes(currentNode,toRemove) && currentNode != targetList.terminalDreapta )
    {
        currentNode = currentNode->nextNode;
    }
    if (currentNode ==  targetList.terminalDreapta)
    {
        return;
    }

    WayPointNode *prev,*urm;
    prev = currentNode->prevNode;
    urm = currentNode->nextNode;
    prev->nextNode = urm;
    urm->prevNode = prev;
    delete(currentNode);
    targetList.lenght--;
}

void FindSmallestFcost( NodeList targetLista, WayPointNode *&smallestFcost )
{
    int smallestF = targetLista.first->fCost;
    smallestFcost = targetLista.first;
    WayPointNode *currentNode;
    currentNode = targetLista.first->nextNode;
    while (currentNode != targetLista.terminalDreapta)
    {
        if (currentNode->fCost < smallestF)
        {
            smallestF = currentNode->fCost;
            smallestFcost = currentNode;
        }
        currentNode = currentNode->nextNode;
    }
}

bool IsContained( NodeList targetList, WayPointNode *lookFor )
{
    WayPointNode *currentNode;
    currentNode = targetList.first;
    while ( currentNode != NULL )
    {
        if  ( currentNode == lookFor )
        {
            return true;
        }
        currentNode = currentNode->nextNode;
    }
    return false;
}

void FindNeighbours( WayPointNode *centerNode, WayPointNode *neighbours[5] )
{
    WayPointNode *auxNode;
    int x,y,xp[] = {-1,0,1,0},yp[] = {0,1,0,-1};
    x = centerNode->nodeCoordonates.x;
    y = centerNode->nodeCoordonates.y;
    int k;
    for (k = 0; k <= 3; k++)
    {
        auxNode = new WayPointNode;
        auxNode = new WayPointNode( wayPoints[x + xp[k] ][ y + yp[k] ] );
        neighbours[k] = auxNode;
    }
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
        currentNode = currentNode->reachedFrom;
    }
    carare.pathLenght++;
    TestDrawPath(carare);
    return carare;
}


void AstarAlgorithm(Vector2 startPost, Vector2 targetPos)
{
    WayPointNode *startNode,*currentNode,*targetNode;
    WayPointNode *neighbours[4];
    int gCost,hCost,fCost,gPartial,hPartial,fPartial;

    startNode = new WayPointNode ( wayPoints[ startPost.x ][ startPost.y ] );
    targetNode = new WayPointNode ( wayPoints[ targetPos.x ][ targetPos.y ] );

    if (targetNode->walkable == false)
    {
        Vector2 targPoz = FindClosestAvailable(targetPos.x, targetPos.y, 1);
        targetNode = new WayPointNode ( wayPoints[targPoz.x][targPoz.y] );
    }
    currentNode = startNode;  //castez nodurile current si target ca pointeri;

    InitializareListe(openList);
    InitializareListe(closedList);//golesc listele

    gCost = DistanceBetweenNodes(currentNode->nodeCoordonates,startNode->nodeCoordonates);
    hCost = DistanceBetweenNodes(currentNode->nodeCoordonates,targetNode->nodeCoordonates);
    currentNode->gCost = gCost;
    currentNode->hCost = hCost;
    currentNode->fCost = gCost + hCost;
    currentNode->reachedFrom = NULL;//calculate starting node Costs

    AddNode(openList,currentNode);

    while ( openList.lenght > 0 )
    {
        //cout<<"while entered"<<endl;
        cout<<"open list: "<<openList.lenght;
        AfisareLista(openList);
        FindSmallestFcost(openList,currentNode);
        cout<<"smallest fcost: "<<currentNode->nodeCoordonates.x<<' '<<currentNode->nodeCoordonates.y<<endl;
        RemoveNode(openList,currentNode);
        cout<<"open list after removeal: "<<openList.lenght;
        AfisareLista(openList);
        cout<<endl<<endl;
        AddNode(closedList,currentNode);
        if (currentNode == targetNode)
        {
            ConstructPath(targetNode);
            return;
        }
        for (int j = 0; j <= 3; j++ )
            {
                neighbours[j] = NULL;
            }
        FindNeighbours(currentNode,neighbours);
        int i;
        for (i = 0; i <= 3; i++)
        {
            //cout<<"plzfuckoff"<<endl;
            gPartial = DistanceBetweenNodes(neighbours[i]->nodeCoordonates,startNode->nodeCoordonates);
            hPartial = DistanceBetweenNodes(neighbours[i]->nodeCoordonates,targetNode->nodeCoordonates);
            fPartial = gPartial + hPartial;
            if ( neighbours[i]->walkable == false || IsContained( closedList,neighbours[i] ) )
            {
                continue;
            }
            else if ( !IsContained(openList, neighbours[i]) || fPartial < neighbours[i]->fCost )
            {
                neighbours[i]->gCost = gPartial;
                neighbours[i]->hCost = hPartial;
                neighbours[i]->fCost = fPartial;
                neighbours[i]->reachedFrom = currentNode;
                if ( IsContained(openList,neighbours[i]) == false )
                {
                    AddNode(openList,neighbours[i]);
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
    testPos2.x = mapSize / 2;
    testPos2.y = mapSize / 2;
    DrawMap();
    //ShowWalkable();
    AstarAlgorithm(testPos1,testPos2);

    return 0;
}
