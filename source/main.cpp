#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <math.h>

#define MAP_SIZE 10
#define PATH_BUFFER_SIZE 1024

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

uint8_t g_tileGrid[MAP_SIZE][MAP_SIZE] = {};
uint8_t g_entityGrid[MAP_SIZE][MAP_SIZE] = {};
int g_camX = 0;
int g_camY = 0;

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
	if (getTile(startPos.x, startPos.y) == TILE_EMPTY) { cost = 1024; } else { cost = 1; }
	return (cost*abs(startPos.x - endPos.x) + cost*abs(startPos.y - endPos.y));
}

Vec2I findMostEfficentTile(Vec2I startPos, Vec2I endPos) {
	Vec2I cord1 = {startPos.x, startPos.y - 1 };
	Vec2I cord2 = {startPos.x + 1, startPos.y};
	Vec2I cord3 = {startPos.x - 1, startPos.y};
	Vec2I cord4 = {startPos.x, startPos.y + 1 };

	int val1;
	int val2;
	int val3;
	int val4;
	if (!isCordInBounds(cord1.x, cord1.y)){val1 = 1024;}else {val1 = claculateEfficencyOfTile(cord1, endPos);}
	if (!isCordInBounds(cord2.x, cord2.y)){val2 = 1024;}else {val2 = claculateEfficencyOfTile(cord2, endPos);}
	if (!isCordInBounds(cord3.x, cord3.y)){val3 = 1024;}else {val3 = claculateEfficencyOfTile(cord3, endPos);}
	if (!isCordInBounds(cord4.x, cord4.y)){val4 = 1024;}else {val4 = claculateEfficencyOfTile(cord4, endPos);}

	int smallestValue = min(min(val1, val2), min(val3, val4));

	if (smallestValue == val1) return cord1;
	if (smallestValue == val2) return cord2;
	if (smallestValue == val3) return cord3;
	if (smallestValue == val4) return cord4;
	
}

int findPath(Vec2I startPos, Vec2I endPos, Vec2I* outPathBuffer, int maxPathBufferSize) {
	int lenght = 1;
	outPathBuffer[0] = startPos;
	while (lenght < maxPathBufferSize && !(startPos.x == endPos.x && startPos.y == endPos.y)) {
		startPos = findMostEfficentTile(startPos, endPos);
		outPathBuffer[lenght] = startPos;
		lenght++;
	}
	
	return lenght;
}

int main() {
	const int screenWidth = 600;
	const int screenHeight = 600;
	int dis = 100;
	int camSize = 6;
	bool choosingFirstTile = false;
	bool choosingSecondTile = false;
	Vec2I firstSelectedPosition;
	int pathLenght = 0;
	Vec2I pathBuffer[PATH_BUFFER_SIZE];

	for (int x = 0; x < MAP_SIZE; x++) {
		for (int y = 0; y < MAP_SIZE; y++) {
			setTile(x, y, TILE_FLOOR);
		}
	}

	setTile(2, 2, TILE_EMPTY);
	setEntity(1, 1, ENTITY_PLAYER);
	InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");
	SetTargetFPS(60);

	int a = findPath({ 0,0 }, { 2,2 }, pathBuffer, PATH_BUFFER_SIZE);


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

		for (int i = 0; i < pathLenght; i++) {
			DrawCircle(pathBuffer[i].x * dis + dis / 2, pathBuffer[i].y * dis + dis / 2, 6, YELLOW);
		}

		if (choosingFirstTile) DrawRectangle(0, 0, 10, 10, YELLOW);
		if (choosingSecondTile) DrawRectangle(10, 0, 10, 10, BLUE);

		if (IsKeyReleased(KEY_S)) g_camY++;
		if (IsKeyReleased(KEY_W)) g_camY--;
		if (IsKeyReleased(KEY_A)) g_camX--;
		if (IsKeyReleased(KEY_D)) g_camX++;


		if (IsKeyReleased(KEY_SPACE) && !choosingSecondTile) choosingFirstTile = true;
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && choosingSecondTile) {
			Vector2 mousePosition = GetMousePosition();
			int pathLenghtTemp = findPath(firstSelectedPosition, calculatePosition(GetMousePosition()), pathBuffer, PATH_BUFFER_SIZE);
			if (pathLenghtTemp < 5) {
				pathLenght = pathLenghtTemp;
				moveEntity(firstSelectedPosition.x, firstSelectedPosition.y, calculatePosition(GetMousePosition()).x, calculatePosition(GetMousePosition()).y);
				choosingSecondTile = false;
			}
			else {
				pathLenght = 0;
			}

		}
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && choosingFirstTile) {
			firstSelectedPosition = calculatePosition(GetMousePosition());
			choosingFirstTile = false;
			choosingSecondTile = true;
		}

		EndDrawing();
	}
	CloseWindow();



	return 0;
}
