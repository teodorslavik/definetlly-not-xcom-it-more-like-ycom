#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define MAP_SIZE 10
#define PATH_BUFFER_SIZE 1024
#define OPEN_LIST_SIZE 1024
#define CLOSED_LIST_SIZE 1024

enum TileValue {
	TILE_EMPTY,
	TILE_FLOOR
};
enum Entity {
	ENTITY_EMPTY,
	ENTITY_PLAYER
};

struct Vec2I {
	int x;
	int y;
};

struct ListLengts {
	int open;
	int closed;
	int extracted;
};

uint8_t g_tileGrid[MAP_SIZE][MAP_SIZE] = {};
uint8_t g_entityGrid[MAP_SIZE][MAP_SIZE] = {};
int g_camX = 0;
int g_camY = 0;
int g_costOfMovement = 10;
int g_costOfDiagonalMovement = 14;

int isCordInBounds(int x, int y) {
	return x >= 0 && x < MAP_SIZE&& y >= 0 && y < MAP_SIZE;
}
Vec2I calculatePosition(Vector2 position) {

	return Vec2I{ (int)(position.x / 100) + g_camX , (int)(position.y / 100) + g_camY };
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
	return (a.x == b.x && a.y == a.y);
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

int removeIndexFromArray(Vec2I* array, int lenghtOfArray ,int index) {
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
			if (getTile(adjacentCordsBuffer[i].x, adjacentCordsBuffer[i].y) == TILE_FLOOR) {
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
				lenghtOfLists.open = removeIndexFromArray(openList, lenghtOfLists.open, a);
			}
		}
	}

	for (int i = 0; i < lenghtOfLists.open; i++) {
		printf("open: %i, %i \n", openList[i].x, openList[i].y);
	}
	for (int i = 0; i < lenghtOfLists.closed; i++) {
		printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
	}
	printf("AAA");



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
	printf("SMALLEST CORDS AT THAT TIME: %i, %i \n", openList[smallestValueIndex].x, openList[smallestValueIndex].y);
	return smallestValueIndex;
}

ListLengts findPathA(Vec2I startPos, Vec2I endPos, Vec2I* openList, int MaxOpenListSize, Vec2I* closedList, int MaxClosedListSize, Vec2I* adjacentCordsBuffer, Vec2I* extractedPath) {
	ListLengts lenghtOfLists = { 0,0,0 };
	lenghtOfLists = addTileToClosedList(startPos, openList, closedList, adjacentCordsBuffer, lenghtOfLists);

	bool finished = false;

	while (!finished) {
		for (int i = 0; i < lenghtOfLists.closed; i++) {
			if (closedList[i].x == endPos.x && closedList[i].y == endPos.y) {
				finished = true;

			}
		}
		if (lenghtOfLists.open > MaxOpenListSize || lenghtOfLists.closed > MaxClosedListSize) {
			finished = true;
		}
		if (!finished) {
			lenghtOfLists = addTileToClosedList(openList[findTileWithLowestValue(startPos, endPos, openList, lenghtOfLists)], openList, closedList, adjacentCordsBuffer, lenghtOfLists);
		}
		for (int i = 0; i < lenghtOfLists.open; i++) {
			printf("open: %i, %i \n", openList[i].x, openList[i].y);
		}		
		for (int i = 0; i < lenghtOfLists.closed; i++) {
			printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
		}
		printf("a");

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

	while (!compareIntVectors(currentTile, startPos)) {
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
			printf("closed: %i, %i \n", closedList[i].x, closedList[i].y);
		}
		printf("\n");
		for (int i = 0; i < 8; i++) {
			printf("verified: %i, %i \n", verifiedCordsExtr[i].x, verifiedCordsExtr[i].y);
		}
		printf("a");

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

		printf("backtrack tile: %i, %i \n", currentTile.x, currentTile.y);
		currentTile = verifiedCordsExtr[smallestValueExtrIndex];
		extractedPath[lenghtOfLists.extracted] = currentTile;
		lenghtOfLists.extracted++;
		verifiedCordsExtrLenght = 0;
		rechableCordsExtrLenght = 0;
		for (int i = 0; i < lenghtOfLists.extracted; i++) {
			printf("extracted: %i, %i \n", extractedPath[i].x, extractedPath[i].y);
		}
		printf("a");
	}


	return lenghtOfLists;
}

int main() {
	const int screenWidth = 600;
	const int screenHeight = 600;
	bool displayPaths = false;
	int dis = 100;
	int camSize = 6;
	bool choosingFirstTile = false;
	bool choosingSecondTile = false;
	Vec2I firstSelectedPosition;
	int pathLenght = 0;
	Vec2I pathBuffer[PATH_BUFFER_SIZE];
	Vec2I openList[PATH_BUFFER_SIZE];
	Vec2I closedList[PATH_BUFFER_SIZE];
	Vec2I extractedPath[PATH_BUFFER_SIZE];
	Vec2I adjacentCordsBuffer[8];
	ListLengts Lenghts;
	int moveLimit = 5;

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			setTile(x, y, TILE_FLOOR);
		}
	}

	setTile(2, 2, TILE_EMPTY);
	setTile(2, 3, TILE_EMPTY);
	setTile(3, 2, TILE_EMPTY);
	setTile(4, 2, TILE_EMPTY);

	setEntity(2, 4, ENTITY_PLAYER);
	InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);

		for (int x = 0; x < camSize; x++) {
			for (int y = 0; y < camSize; y++) {

				const int tile = getTile(g_camX + x, g_camY + y);
				const int entity = getEntity(g_camX + x, g_camY + y);

				if (tile == TILE_FLOOR) {
					DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, RED);
				}
				if (entity == ENTITY_PLAYER) {
					DrawCircle(x * dis + dis / 2, y * dis + dis / 2, 25, BLUE);
				}
			}
		}
		if (displayPaths) {
			for (int i = 0; i < Lenghts.open; i++) {
				DrawCircle(openList[i].x * dis + dis / 2 - g_camX * dis, openList[i].y * dis + dis / 2 - g_camY * dis, 8, WHITE);
			}

			for (int i = 0; i < Lenghts.closed; i++) {
				DrawCircle(closedList[i].x * dis + dis / 2 - g_camX * dis, closedList[i].y * dis + dis / 2 - g_camY * dis, 8, PINK);
			}
			for (int i = 0; i < Lenghts.extracted; i++) {
				DrawCircle(extractedPath[i].x * dis + dis / 2 - g_camX * dis, extractedPath[i].y * dis + dis / 2 - g_camY * dis, 4, YELLOW);
			}
		}


		if (choosingFirstTile) DrawRectangle(0, 0, 10, 10, YELLOW);
		if (choosingSecondTile) DrawRectangle(10, 0, 10, 10, BLUE);

		if (IsKeyReleased(KEY_S)) g_camY++;
		if (IsKeyReleased(KEY_W)) g_camY--;
		if (IsKeyReleased(KEY_A)) g_camX--;
		if (IsKeyReleased(KEY_D)) g_camX++;


		if (IsKeyPressed(KEY_SPACE) && !choosingSecondTile) choosingFirstTile = true; 
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && choosingSecondTile) {
			Vector2 mousePosition = GetMousePosition();
			Vec2I secondSelectedPosition = calculatePosition(GetMousePosition());
			//ListLengts Lenghts = findPathA(Vec2I{ 0,0 }, Vec2I{ 3,3 }, openList, PATH_BUFFER_SIZE, closedList, PATH_BUFFER_SIZE, adjacentCordsBuffer, extractedPath);
			Lenghts = findPathA(firstSelectedPosition, secondSelectedPosition, openList, PATH_BUFFER_SIZE, closedList, PATH_BUFFER_SIZE, adjacentCordsBuffer, extractedPath);
			if (Lenghts.extracted < moveLimit + 1) {
				moveEntity(firstSelectedPosition.x, firstSelectedPosition.y, secondSelectedPosition.x, secondSelectedPosition.y);
				choosingSecondTile = false;
				displayPaths = true;
			}

		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && choosingFirstTile) {
			firstSelectedPosition = calculatePosition(GetMousePosition());
			choosingFirstTile = false;
			choosingSecondTile = true;
			displayPaths = false;
		}

		EndDrawing();
	}
	CloseWindow();

	return 0;
}
