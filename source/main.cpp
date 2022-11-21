#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define MAP_SIZE 8
#define PATH_BUFFER_SIZE 1024
#define OPEN_LIST_SIZE 1024
#define CLOSED_LIST_SIZE 1024
#define MAX_AMOUNT_OF_CREATURES 1024
#define BANNED_CREATURE_ID 255
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 700

#define MAX_LENGHT_OF_STATUS_EFFECTS 128
#define STATUS_COVER_ID 1

enum TileValue {
	TILE_EMPTY,
	TILE_FLOOR,
	TILE_BOX
};
enum Entity {
	ENTITY_EMPTY,
	ENTITY_LOOT
};
enum Cover {
	NO_COVER,
	HALF_COVER,
	FULL_COVER
};
struct Vec2I {
	int x;
	int y;
};
struct StatusEfect {
	int statusEffectId;
	int strenght;
	Vector2 orientation;
	int duration;
};
/*struct HotbarEntity {
	int hotbarEntityValue;
	int imageId;
	int value;
};*/
struct CreatureData {
	int creatureId;
	int health;
	int aim;
	int actionPointsLeft;
	int maxActions;
	int defense;
	bool isAlly;
	int mobility;
	StatusEfect statusEfects[MAX_LENGHT_OF_STATUS_EFFECTS];
	int lenghtOfStatusEfects;

};
struct ListLengts {
	int open;
	int closed;
	int extracted;
};

uint8_t g_tileGrid[MAP_SIZE][MAP_SIZE] = {};
uint8_t g_entityGrid[MAP_SIZE][MAP_SIZE] = {};
uint8_t g_creatureGrid[MAP_SIZE][MAP_SIZE] = {};
int g_camX = 0;
int g_camY = 0;
int g_costOfMovement = 10;
int g_costOfDiagonalMovement = 14;
int camSize = 6;
bool g_playerTurn = true;
bool g_enemyTurn = false;

int isCordInBounds(int x, int y) {
	return x >= 0 && x < MAP_SIZE&& y >= 0 && y < MAP_SIZE;
}
Vec2I calculatePosition(Vector2 position) {

	return Vec2I{ (int)(position.x / (100 * 6/camSize)) + g_camX , (int)(position.y / (100 * 6 / camSize)) + g_camY };
}
int getTile(int x, int y) {
	if (!isCordInBounds(x, y)) {
		return TILE_EMPTY;
	}
	return g_tileGrid[x][y];
}
int getEntity(int x, int y) {
	if (!isCordInBounds(x, y)) {
		return ENTITY_EMPTY;
	}
	return g_entityGrid[x][y];
}
int getCreature(int x, int y) {
	if (!isCordInBounds(x, y)) {
		return BANNED_CREATURE_ID;
	}
	return g_creatureGrid[x][y];
}
void setTile(int x, int y, TileValue value) {
	if (!isCordInBounds(x, y)) {
		return;
	}
	g_tileGrid[x][y] = (uint8_t)value;
}
void setEntity(int x, int y, Entity value) {
	if (!isCordInBounds(x, y)) {
		return;
	}
	g_entityGrid[x][y] = (uint8_t)value;
}
void moveTile(int xOrig, int yOrig, int xNew, int yNew) {
	if (!isCordInBounds(xNew, yNew) || !isCordInBounds(xOrig, yOrig)) {
		return;
	}
	g_tileGrid[xNew][yNew] = (uint8_t)getTile(xOrig, yOrig);
	g_tileGrid[xOrig][yOrig] = (uint8_t)TILE_FLOOR;
}
void moveEntity(int xOrig, int yOrig, int xNew, int yNew) {
	if (!isCordInBounds(xNew, yNew) || !isCordInBounds(xOrig, yOrig)) {
		return;
	}
	g_entityGrid[xNew][yNew] = (uint8_t)getEntity(xOrig, yOrig);
	g_entityGrid[xOrig][yOrig] = (uint8_t)ENTITY_EMPTY;
}
void moveCreature(int xOrig, int yOrig, int xNew, int yNew) {
	if (!isCordInBounds(xNew, yNew) || !isCordInBounds(xOrig, yOrig)) {
		return;
	}
	if (getCreature(xNew, yNew) != BANNED_CREATURE_ID) {
		return;
	}
	g_creatureGrid[xNew][yNew] = (uint8_t)getCreature(xOrig, yOrig);
	g_creatureGrid[xOrig][yOrig] = BANNED_CREATURE_ID;
}
int min(int a, int b) {
	if (a > b) return b;
	return a;
}
int abs(int number) {
	if (number > 0) {
		return number;
	}
	else {
		return -number;
	}
}
int claculateEfficencyOfTile(Vec2I startPos, Vec2I endPos) {
	int cost;
	if (getTile(startPos.x, startPos.y) == TILE_EMPTY) { cost = 1024; }
	else { cost = 1; }
	return (cost * abs(startPos.x - endPos.x) + cost * abs(startPos.y - endPos.y));
}
Vec2I findMostEfficentTile(Vec2I startPos, Vec2I endPos) {
	Vec2I cord1 = { startPos.x, startPos.y - 1 };
	Vec2I cord2 = { startPos.x + 1, startPos.y };
	Vec2I cord3 = { startPos.x - 1, startPos.y };
	Vec2I cord4 = { startPos.x, startPos.y + 1 };

	int val1;
	int val2;
	int val3;
	int val4;
	if (!isCordInBounds(cord1.x, cord1.y)) { val1 = 1024; }
	else { val1 = claculateEfficencyOfTile(cord1, endPos); }
	if (!isCordInBounds(cord2.x, cord2.y)) { val2 = 1024; }
	else { val2 = claculateEfficencyOfTile(cord2, endPos); }
	if (!isCordInBounds(cord3.x, cord3.y)) { val3 = 1024; }
	else { val3 = claculateEfficencyOfTile(cord3, endPos); }
	if (!isCordInBounds(cord4.x, cord4.y)) { val4 = 1024; }
	else { val4 = claculateEfficencyOfTile(cord4, endPos); }

	int smallestValue = min(min(val1, val2), min(val3, val4));

	if (smallestValue == val1) return cord1;
	if (smallestValue == val2) return cord2;
	if (smallestValue == val3) return cord3;
	if (smallestValue == val4) return cord4;

}
bool compareIntVectors(Vec2I a, Vec2I b) {
	if ((a.x == b.x && a.y == a.y)) return true;
	return false;
}
void findAdjacentCords(Vec2I position, Vec2I* adjacentCordsBuffer) {
	adjacentCordsBuffer[0] = { position.x - 1, position.y - 1 };
	adjacentCordsBuffer[1] = { position.x, position.y - 1 };
	adjacentCordsBuffer[2] = { position.x + 1, position.y - 1 };
	adjacentCordsBuffer[3] = { position.x - 1, position.y };
	adjacentCordsBuffer[4] = { position.x + 1, position.y };
	adjacentCordsBuffer[5] = { position.x - 1, position.y + 1 };
	adjacentCordsBuffer[6] = { position.x, position.y + 1 };
	adjacentCordsBuffer[7] = { position.x + 1, position.y + 1 };
}
int removeIndexFromArrayVec2I(Vec2I* array, int lenghtOfArray ,int index) {
	for (int i = index; i < lenghtOfArray; i++) {
		array[i] = array[i + 1];
	}
	lenghtOfArray = lenghtOfArray - 1;
	return lenghtOfArray;
}
int removeIndexFromArrayStatusEffect(StatusEfect* array, int lenghtOfArray, int index) {
	for (int i = index; i < lenghtOfArray; i++) {
		array[i] = array[i + 1];
	}
	lenghtOfArray = lenghtOfArray - 1;
	return lenghtOfArray;
}
ListLengts addTileToClosedList(Vec2I position, Vec2I* openList, Vec2I* closedList, Vec2I* adjacentCordsBuffer, ListLengts lenghtOfLists) {
	Vec2I verifiedCordsBuffer[8];
	int lenghtOfVerifiedCords = 0;

	Vec2I nonDuplicitCordsBuffer[8];
	int lenghtOfNonDupCords = 0;

	findAdjacentCords(position, adjacentCordsBuffer);

	for (int i = 0; i < 8; i++) {
		if (isCordInBounds(adjacentCordsBuffer[i].x, adjacentCordsBuffer[i].y)) {
			if (getTile(adjacentCordsBuffer[i].x, adjacentCordsBuffer[i].y) == TILE_FLOOR && getCreature(adjacentCordsBuffer[i].x, adjacentCordsBuffer[i].y) == BANNED_CREATURE_ID) {
				verifiedCordsBuffer[lenghtOfVerifiedCords] = adjacentCordsBuffer[i];
				lenghtOfVerifiedCords++;
			}
		}
	}

	closedList[lenghtOfLists.closed] = position;
	lenghtOfLists.closed++;

	int duplicityIndex = 0;

	for (int a = 0; a < lenghtOfLists.open; a++) {
		for (int b = 0; b < lenghtOfLists.closed; b++) {
			if (openList[a].x == closedList[b].x && openList[a].y == closedList[b].y) {
				duplicityIndex = a;
				lenghtOfLists.open = removeIndexFromArrayVec2I(openList, lenghtOfLists.open, a);
			}
		}
	}

	for (int i = 0; i < lenghtOfLists.open; i++) {
		//printf("open: %i, %i \n", openList[i].x, openList[i].y);
	}
	for (int i = 0; i < lenghtOfLists.closed; i++) {
		//printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
	}
	//printf("AAA");



	bool colision = false;
	for (int i = 0; i < lenghtOfVerifiedCords; i++) {
		for (int a = 0; a < lenghtOfLists.open; a++) {
			if (verifiedCordsBuffer[i].x == openList[a].x && verifiedCordsBuffer[i].y == openList[a].y) {
				colision = true;
			}
		}
		if (!colision) {
			for (int a = 0; a < lenghtOfLists.closed; a++) {
				if (verifiedCordsBuffer[i].x == closedList[a].x && verifiedCordsBuffer[i].y == closedList[a].y) {
					colision = true;
				}
			}
		}
		if (!colision) {
			nonDuplicitCordsBuffer[lenghtOfNonDupCords] = verifiedCordsBuffer[i];
			lenghtOfNonDupCords++;
		}
		else {
			colision = false;
		}
	}

	for (int i = 0; i < lenghtOfNonDupCords; i++) {
		openList[lenghtOfLists.open] = nonDuplicitCordsBuffer[i];
		lenghtOfLists.open++;
	}

	return lenghtOfLists;
}
int findTileWithLowestValue(Vec2I startPos, Vec2I endPos, Vec2I* openList, ListLengts lenghtOfLists) {
	int values[PATH_BUFFER_SIZE];
	int valuesLenght = 0;

	while (valuesLenght < lenghtOfLists.open) {
		int dx = abs(openList[valuesLenght].x - endPos.x);
		int dy = abs(openList[valuesLenght].y - endPos.y);

		values[valuesLenght] = g_costOfMovement * (dx + dy) + (g_costOfDiagonalMovement - 2 * g_costOfMovement) * min(dy, dx);

		valuesLenght++;
	}

	int smallestValueIndex = 0;
	for (int i = 0; i < valuesLenght; i++) {
		if (values[i] < values[smallestValueIndex]) {
			smallestValueIndex = i;
		}
	}
	//printf("SMALLEST CORDS AT THAT TIME: %i, %i \n", openList[smallestValueIndex].x, openList[smallestValueIndex].y);
	return smallestValueIndex;
}
ListLengts findPathA(Vec2I startPos, Vec2I endPos, /*Vec2I* openList, int MaxOpenListSize, Vec2I* closedList, int MaxClosedListSize, Vec2I* adjacentCordsBuffer,*/ Vec2I* extractedPath) {

	static Vec2I openList[PATH_BUFFER_SIZE];
	static Vec2I closedList[PATH_BUFFER_SIZE];
	static Vec2I adjacentCordsBuffer[8];

	ListLengts lenghtOfLists = { 0,0,0 };
	lenghtOfLists = addTileToClosedList(startPos, openList, closedList, adjacentCordsBuffer, lenghtOfLists);

	bool finished = false;

	while (!finished) {
		for (int i = 0; i < lenghtOfLists.closed; i++) {
			if (closedList[i].x == endPos.x && closedList[i].y == endPos.y) {
				finished = true;

			}
		}
		if (lenghtOfLists.open > OPEN_LIST_SIZE || lenghtOfLists.closed > CLOSED_LIST_SIZE) {
			finished = true;
		}
		if (!finished) {
			lenghtOfLists = addTileToClosedList(openList[findTileWithLowestValue(startPos, endPos, openList, lenghtOfLists)], openList, closedList, adjacentCordsBuffer, lenghtOfLists);
		}
		for (int i = 0; i < lenghtOfLists.open; i++) {
			//printf("open: %i, %i \n", openList[i].x, openList[i].y);
		}		
		for (int i = 0; i < lenghtOfLists.closed; i++) {
			//printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
		}
		//printf("a");

	}
	extractedPath[0] = closedList[lenghtOfLists.closed - 1];
	lenghtOfLists.extracted++;

	Vec2I currentTile = closedList[lenghtOfLists.closed - 1];
	Vec2I adjacentCordsExtr[8];
	Vec2I rechableCordsExtr[8];
	Vec2I verifiedCordsExtr[8];
	int verifiedCordsExtrLenght = 0;
	int rechableCordsExtrLenght = 0;

	int valuesExtr[8];
	int smallestValueExtrIndex = 0;

	while (!(startPos.x == currentTile.x && startPos.y == currentTile.y)) {
		adjacentCordsExtr[0] = { currentTile.x - 1, currentTile.y - 1 };
		adjacentCordsExtr[1] = { currentTile.x, currentTile.y - 1 };
		adjacentCordsExtr[2] = { currentTile.x + 1, currentTile.y - 1 };
		adjacentCordsExtr[3] = { currentTile.x - 1, currentTile.y };
		adjacentCordsExtr[4] = { currentTile.x + 1, currentTile.y };
		adjacentCordsExtr[5] = { currentTile.x - 1, currentTile.y + 1 };
		adjacentCordsExtr[6] = { currentTile.x, currentTile.y + 1 };
		adjacentCordsExtr[7] = { currentTile.x + 1, currentTile.y + 1 };

		for (int a = 0; a < lenghtOfLists.closed; a++) {
			for (int b = 0; b < 8; b++) {
				if (closedList[a].x == adjacentCordsExtr[b].x && closedList[a].y == adjacentCordsExtr[b].y) {
					rechableCordsExtr[rechableCordsExtrLenght] = adjacentCordsExtr[b];
					rechableCordsExtrLenght++;
				}
			}

		}

		for (int i = 0; i < 8; i++) {

			if (getTile(rechableCordsExtr[i].x, rechableCordsExtr[i].y) == TILE_FLOOR) {
				verifiedCordsExtr[verifiedCordsExtrLenght] = rechableCordsExtr[i];
				verifiedCordsExtrLenght++;
			}

		}

		for (int i = 0; i < lenghtOfLists.closed; i++) {
			//printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
		}
		//printf("\n");
		for (int i = 0; i < 8; i++) {
			//printf("verified: %i, %i \n", verifiedCordsExtr[i].x, verifiedCordsExtr[i].y);
		}
		//printf("a");

		for (int i = 0; i < verifiedCordsExtrLenght; i++) {
			int dx = abs(verifiedCordsExtr[i].x - startPos.x);
			int dy = abs(verifiedCordsExtr[i].y - startPos.y);

			valuesExtr[verifiedCordsExtrLenght] = g_costOfMovement * (dx + dy) + (g_costOfDiagonalMovement - 2 * g_costOfMovement) * min(dy, dx);
		}

		int smallestValue = valuesExtr[0];
		for (int i = 0; i < verifiedCordsExtrLenght; i++) {
			if (valuesExtr[i] < smallestValue) {
				smallestValue = valuesExtr[i];
				smallestValueExtrIndex = i;
			}
		}

		//printf("backtrack tile: %i, %i \n", currentTile.x, currentTile.y);
		currentTile = verifiedCordsExtr[smallestValueExtrIndex];
		extractedPath[lenghtOfLists.extracted] = currentTile;
		lenghtOfLists.extracted++;
		verifiedCordsExtrLenght = 0;
		rechableCordsExtrLenght = 0;
		for (int i = 0; i < lenghtOfLists.extracted; i++) {
			//printf("extracted: %i, %i \n", extractedPath[i].x, extractedPath[i].y);
		}
		//printf("a");
	}

	Vec2I temp;
	for (int i = 0; i < lenghtOfLists.extracted / 2; i++) {
		temp = extractedPath[i];
		extractedPath[i] = extractedPath[lenghtOfLists.extracted - i - 1];
		extractedPath[lenghtOfLists.extracted - i - 1] = temp;
	}

	return lenghtOfLists;
}
int createCreature(Vec2I position, int lenghtOfCreatureList, int health, int aim, int maxActions, int defense, bool isAlly, int agility, CreatureData* outlistOfAllCreatures) {
	if (isCordInBounds(position.x, position.y)) {
		CreatureData newCreature = { lenghtOfCreatureList, health, aim, maxActions, maxActions, defense, isAlly, agility, {0,0} , 0};
		//g_creatureGrid[position.x, position.y] = (uint8_t)newCreature.creatureId;
		outlistOfAllCreatures[lenghtOfCreatureList] = newCreature;
		g_creatureGrid[position.x][position.y] = newCreature.creatureId;
		lenghtOfCreatureList++;
	}

	return lenghtOfCreatureList;

}
int selectCreature(Vector2 mousePosition) {
	Vec2I position = calculatePosition(mousePosition);
	int value = getCreature(position.x, position.y);
	if (value == BANNED_CREATURE_ID) {
		return BANNED_CREATURE_ID;
	}
	return value;
}
Vec2I findLocationOfCreatureByID(int id) {
	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			if (g_creatureGrid[x][y] == id) return { x, y };
		}
	}
}
void addStatusEffect(int lenghtOfCreatureList, CreatureData* listOfAllCreatures, int idOfcreature, StatusEfect statusEfect) {

	if (idOfcreature > lenghtOfCreatureList) { return; }
	CreatureData currentCreature = listOfAllCreatures[idOfcreature];
	currentCreature.statusEfects[currentCreature.lenghtOfStatusEfects] = statusEfect;
	currentCreature.lenghtOfStatusEfects = currentCreature.lenghtOfStatusEfects + 1;
	listOfAllCreatures[idOfcreature] = currentCreature;

}
int findIndexOfStatusEffectByID(CreatureData creature, int searchedId) {
	for (int i = 0; i < creature.lenghtOfStatusEfects; i++) {
		if (creature.statusEfects[i].statusEffectId == searchedId) {
			int a = 1;
			return i;
		}
	}
}
int getCoverOfTile(Vec2I position) {
	int tile = getTile(position.x, position.y);
	if (tile == TILE_EMPTY || tile == TILE_FLOOR) {
		return NO_COVER;
	}
	if (tile == TILE_BOX) {
		return HALF_COVER;
	}
}
void updateCreatureCover(int lenghtOfCreatureList, CreatureData* listOfAllCreatures, int idOfcreature, Vec2I position) {
	Vec2I adjacentCordsBuffer[4];
	adjacentCordsBuffer[0] = { position.x, position.y - 1 };
	adjacentCordsBuffer[1] = { position.x - 1, position.y};
	adjacentCordsBuffer[2] = { position.x, position.y + 1 };
	adjacentCordsBuffer[3] = { position.x + 1, position.y };

	
	CreatureData currentCreature = listOfAllCreatures[idOfcreature];

	int indexesToDelete[MAX_LENGHT_OF_STATUS_EFFECTS];
	int lenghtOfIndexesToDelete = 0;

	for (int i = 0; i < currentCreature.lenghtOfStatusEfects; i++) {
		if (currentCreature.statusEfects[i].statusEffectId == STATUS_COVER_ID) {
			indexesToDelete[lenghtOfIndexesToDelete] = i;
			lenghtOfIndexesToDelete++;
		}
	}
	for (int i = 0; i < lenghtOfIndexesToDelete; i++) {
		currentCreature.lenghtOfStatusEfects = removeIndexFromArrayStatusEffect(currentCreature.statusEfects, currentCreature.lenghtOfStatusEfects, indexesToDelete[i]);
	}

	for (int i = 0; i < 8; i++) {
		int coverStatus = getCoverOfTile(adjacentCordsBuffer[i]);
		
		if (coverStatus == HALF_COVER) {

			Vector2 VectorOfTwoPoints; // AB->
			Vec2I A = position;
			Vec2I B = adjacentCordsBuffer[i];
			VectorOfTwoPoints = { (float)(B.x - A.x), (float)(B.y - A.y) };
			//double SizeOfVector = sqrt(VectorOfTwoPoints.x * VectorOfTwoPoints.x + VectorOfTwoPoints.y * VectorOfTwoPoints.y);
			//Vector2 NormalaisedVector = { VectorOfTwoPoints.x/SizeOfVector, VectorOfTwoPoints.y / SizeOfVector };

			StatusEfect statusEfect = { STATUS_COVER_ID, HALF_COVER, VectorOfTwoPoints/*NormalaisedVector8*/, 0};

			currentCreature.statusEfects[currentCreature.lenghtOfStatusEfects] = statusEfect;
			currentCreature.lenghtOfStatusEfects = currentCreature.lenghtOfStatusEfects + 1;
			listOfAllCreatures[idOfcreature] = currentCreature;


		}
	}	
	listOfAllCreatures[idOfcreature] = currentCreature;
}
bool isMouseWithinPlayableArea() {
	Vector2 mouse = GetMousePosition();
	if (mouse.x > 0 && mouse.x < SCREEN_WIDTH && mouse.y > 0 && mouse.y < 600) {
		return true;
	}
	else {
		return false;
	}
	
}
void turnHandler(CreatureData* listOfAllCreatures, int lenghtOfCreatureList) {
	int playerTurnsLeft = 0;
	int enemyTurnsLeft = 0;

	if (g_playerTurn) {
		for (int i = 0; i < lenghtOfCreatureList; i++) {
			CreatureData currentCreature = listOfAllCreatures[i];
			if (currentCreature.isAlly && currentCreature.creatureId != BANNED_CREATURE_ID) {
				playerTurnsLeft = playerTurnsLeft + currentCreature.actionPointsLeft;
			}
		}

		if (playerTurnsLeft <= 0) {

			for (int i = 0; i < lenghtOfCreatureList; i++) {
				
				if (!listOfAllCreatures[i].isAlly && listOfAllCreatures[i].creatureId != BANNED_CREATURE_ID) {
					listOfAllCreatures[i].actionPointsLeft = listOfAllCreatures[i].maxActions;
				}
			}


			g_playerTurn = false;
			g_enemyTurn = true;
			printf("END OF PLAYER TURN \n");
			return;
		}
	}
	
	if (g_enemyTurn) {
		for (int i = 0; i < lenghtOfCreatureList; i++) {

			if (!listOfAllCreatures[i].isAlly && listOfAllCreatures[i].creatureId != BANNED_CREATURE_ID) {
				enemyTurnsLeft = enemyTurnsLeft + listOfAllCreatures[i].actionPointsLeft;
			}
		}
		if (enemyTurnsLeft <= 0) {
			for (int i = 0; i < lenghtOfCreatureList; i++) {

				if (listOfAllCreatures[i].isAlly && listOfAllCreatures[i].creatureId != BANNED_CREATURE_ID) {
					listOfAllCreatures[i].actionPointsLeft = listOfAllCreatures[i].maxActions;
				}
			}

			g_playerTurn = true;
			g_enemyTurn = false;
			printf("END OF ENEMY TURN \n");
			return;
		}
	}
}

int main() {
	bool displayPath = false;
	bool displayPathDebug = false;
	
	int defaultCamSize = 6;
	int minCamSize = 3;
	int maxCamSize = 12;

	camSize = defaultCamSize;

	int dis = 60/camSize * 10;
	bool choosingFirstTile = false;
	bool choosingSecondTile = false;
	Vec2I firstSelectedPosition;
	int pathLenght = 0;
	Vec2I pathBuffer[PATH_BUFFER_SIZE];
	Vec2I extractedPath[PATH_BUFFER_SIZE];

	ListLengts Lenghts = {0,0,0};
	int moveLimit = 5;
	int idOfselectedCreature = BANNED_CREATURE_ID;
	bool selectingSecondTile = false;

	static CreatureData listOfAllCreatures[MAX_AMOUNT_OF_CREATURES];
	int lenghtOfCreatureList = 0;

	for (int x = 0;x < MAP_SIZE ; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			g_creatureGrid[x][y] = BANNED_CREATURE_ID;
		}
	}

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			setTile(x, y, TILE_FLOOR);
		}
	}

	setTile(2, 2, TILE_BOX);
	setTile(2, 3, TILE_EMPTY);
	setTile(3, 2, TILE_BOX);
	setTile(4, 2, TILE_BOX);
	setTile(0, 0, TILE_BOX);

	lenghtOfCreatureList = createCreature({ 1,1 }, lenghtOfCreatureList, 6, 65, 2, 60, true, 4, listOfAllCreatures);

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			//printf("%i ", g_creatureGrid[x][y]);
		}
		//printf("\n");
	}
	for (int i = 0; i < lenghtOfCreatureList; i++) {
		//printf("list: %i \n", listOfAllCreatures[i].creatureId);
	}


	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Y-COM");

	SetTargetFPS(60);
	Texture2D ally_normal = LoadTexture("./textures/player_normal_001.png");
	Texture2D floor_normal = LoadTexture("./textures/floor_basic_001.png");
	Texture2D box_normal = LoadTexture("./textures/box_basic_001.png");

	while (!WindowShouldClose()) {
		BeginDrawing();	
		ClearBackground(BLACK);
		

		if (GetMouseWheelMove() == -1 && camSize < maxCamSize) camSize++;
		if (GetMouseWheelMove() == 1 && camSize > minCamSize) camSize--;
		
		dis = (int)((6 / (float)camSize) * 100);

		for (int x = 0; x < camSize; x++) {
			for (int y = 0; y < camSize; y++) {

				const int tile = getTile(g_camX + x, g_camY + y);
				const int entity = getEntity(g_camX + x, g_camY + y);
				const int idOfSearchedCreature = getCreature(g_camX + x, g_camY + y);

				if (tile == TILE_FLOOR) {
					int rotations[4] = { 0,90,180,270 };
					DrawTextureEx(floor_normal, { (float)(x * dis), (float)(y * dis) }, rotations[0/*GetRandomValue(0, 3) */], (float)(dis-1) / floor_normal.width, RAYWHITE);

				}
				else if (tile == TILE_BOX) {
					int rotations[4] = { 0,90,180,270 };
					DrawTextureEx(floor_normal, { (float)(x * dis), (float)(y * dis) }, rotations[0/*GetRandomValue(0, 3) */], (float)(dis - 1) / floor_normal.width, RAYWHITE);
					DrawTextureEx(box_normal, { (float)(x * dis), (float)(y * dis) }, 0, (float)(dis - 1) / box_normal.width, RAYWHITE);
				}

				if (entity == ENTITY_LOOT) {
					DrawCircle(x * dis + dis / 2, y * dis + dis / 2, dis/4, GOLD);
				}
				if (idOfSearchedCreature != BANNED_CREATURE_ID) {
					if (idOfSearchedCreature == idOfselectedCreature) {
						Vector2 cornerLU[3] = { {(float)x * dis, (float)y * dis + dis/3}, {(float)x * dis + dis / 3, (float)y * dis}, {(float)x * dis, (float)y * dis} };
						Vector2 cornerRU[3] = { {(float)x * dis + dis, (float)y * dis + dis / 3}, {(float)x * dis + dis - dis / 3, (float)y * dis}, {(float)x * dis + dis, (float)y * dis} } ;
						Vector2 cornerLB[3] = {{(float)x * dis, (float)y * dis + dis - dis / 3} , {(float)x * dis + dis / 3, (float)y * dis + dis}, {(float)x * dis, (float)y * dis + dis}};
						Vector2 cornerRB[3] = { {(float)x * dis + dis, (float)y * dis + dis - dis / 3} , {(float)x * dis + dis - dis / 3, (float)y * dis + dis} , {(float)x * dis + dis, (float)y * dis + dis} };

						DrawLineBezierQuad(cornerLU[0], cornerLU[1], cornerLU[2], 2, WHITE);
						DrawLineBezierQuad(cornerRU[0], cornerRU[1], cornerRU[2], 2, WHITE);
						DrawLineBezierQuad(cornerLB[0], cornerLB[1], cornerLB[2], 2, WHITE);
						DrawLineBezierQuad(cornerRB[0], cornerRB[1], cornerRB[2], 2, WHITE);

						CreatureData currentCreature = listOfAllCreatures[idOfselectedCreature];
						
						DrawText(TextFormat("HEALTH: %i", currentCreature.health), 5, 620, 15, WHITE);
						DrawText(TextFormat("AP: %i", currentCreature.actionPointsLeft), 5, 640, 15, WHITE);

						
						if (currentCreature.lenghtOfStatusEfects != 0) {
							int indexInStatusEffects = findIndexOfStatusEffectByID(currentCreature, STATUS_COVER_ID);

							if (currentCreature.statusEfects[indexInStatusEffects].strenght == 1) {
								DrawCircle(150, 650, 42, WHITE);
								DrawCircle(150, 650, 32, BLACK);
							}
							if (currentCreature.statusEfects[indexInStatusEffects].strenght == 2) {
								DrawCircle(150, 650, 42, WHITE);
							}
						}
					}
					DrawTextureEx(ally_normal, { (float)(x * dis), (float)(y * dis) }, 0, (float)dis/ally_normal.width, RAYWHITE);
					//DrawCircle(x * dis + dis / 2, y * dis + dis / 2, dis/4, BLUE);
				}
			}
		}
		/*
		if (displayPathDebug) {
			for (int i = 0; i < Lenghts.open; i++) {
				DrawCircle(openList[i].x * dis + dis / 2 - g_camX * dis, openList[i].y * dis + dis / 2 - g_camY * dis, dis/10, WHITE);
			}

			for (int i = 0; i < Lenghts.closed; i++) {
				DrawCircle(closedList[i].x * dis + dis / 2 - g_camX * dis, closedList[i].y * dis + dis / 2 - g_camY * dis, dis / 10, PINK);
			}
			for (int i = 0; i < Lenghts.extracted; i++) {
				DrawCircle(extractedPath[i].x * dis + dis / 2 - g_camX * dis, extractedPath[i].y * dis + dis / 2 - g_camY * dis, dis / 20, YELLOW);
			}
		}*/
		if (displayPath) {
			for (int i = 1; i < Lenghts.extracted; i++) {
				Vector2 start = { extractedPath[i-1].x * dis + dis / 2 - g_camX * dis, extractedPath[i-1].y * dis + dis / 2 - g_camY * dis };
				Vector2 end = { extractedPath[i].x * dis + dis / 2 - g_camX * dis, extractedPath[i].y * dis + dis / 2 - g_camY * dis };
				DrawLineEx(start, end, 4, YELLOW);

				DrawCircle(extractedPath[i].x * dis + dis / 2 - g_camX * dis, extractedPath[i].y * dis + dis / 2 - g_camY * dis, 2, YELLOW);
			}
		}


		if (choosingFirstTile) DrawRectangle(0, 0, 10, 10, YELLOW);
		if (choosingSecondTile) DrawRectangle(10, 0, 10, 10, BLUE);
		
		if (IsKeyReleased(KEY_S)) g_camY++;
		if (IsKeyReleased(KEY_W)) g_camY--;
		if (IsKeyReleased(KEY_A)) g_camX--;
		if (IsKeyReleased(KEY_D)) g_camX++;
		
		if (IsKeyReleased(KEY_SPACE)) { idOfselectedCreature = BANNED_CREATURE_ID; displayPath = false; selectingSecondTile = false; }
		
		if (g_playerTurn) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isMouseWithinPlayableArea()) {
				if (selectCreature(GetMousePosition()) == BANNED_CREATURE_ID) {

				}
				else {
					idOfselectedCreature = selectCreature(GetMousePosition());
					selectingSecondTile = false;
					displayPath = false;
				}
			}
			if (idOfselectedCreature != BANNED_CREATURE_ID && selectCreature(GetMousePosition()) == BANNED_CREATURE_ID && isMouseWithinPlayableArea() && listOfAllCreatures[idOfselectedCreature].actionPointsLeft > 0) {
				Vec2I secondPos = calculatePosition(GetMousePosition());
				if (getTile(secondPos.x, secondPos.y) == TILE_FLOOR) {
					Vec2I firstPos = findLocationOfCreatureByID(idOfselectedCreature);

					Lenghts = findPathA(firstPos, secondPos, /*openList, OPEN_LIST_SIZE, closedList, CLOSED_LIST_SIZE, adjacentCordsBuffer,*/ extractedPath);

					if (Lenghts.extracted > listOfAllCreatures[idOfselectedCreature].mobility + 1) {
						Lenghts.extracted = listOfAllCreatures[idOfselectedCreature].mobility + 1;
					}
					displayPath = true;
					selectingSecondTile = true;
					//printf("display time? : %i, %i \n", Lenghts.extracted, Lenghts.closed);

					if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && selectingSecondTile) {
						moveCreature(firstPos.x, firstPos.y, extractedPath[Lenghts.extracted - 1].x, extractedPath[Lenghts.extracted - 1].y);//secondPos.x, secondPos.y);
						listOfAllCreatures[idOfselectedCreature].actionPointsLeft = listOfAllCreatures[idOfselectedCreature].actionPointsLeft--;
						updateCreatureCover(lenghtOfCreatureList, listOfAllCreatures, idOfselectedCreature, secondPos);

						displayPath = false;
						selectingSecondTile = false;
					}
				}
			}
		}
		
		turnHandler(listOfAllCreatures, lenghtOfCreatureList);
		EndDrawing();
	}
	CloseWindow();

	return 0;
}
