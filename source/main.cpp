#include "raylib.h"
#include <stdint.h>
#define MAP_SIZE 10


enum TileValue {
    TILE_EMPTY,
    TILE_FLOOR,
    TILE_PLAYER
};
uint8_t g_tileMap[MAP_SIZE][MAP_SIZE] = {};
int camX = 0;
int camY = 0;

int isTileInBounds(int x, int y) {
    return x >= 0 && x < MAP_SIZE&& y >= 0 && y < MAP_SIZE;
}

Vector2 calculatePosition(Vector2 position) {
    
    return Vector2{ (float)(position.x / 100) + camX , (float)(position.y / 100) + camY };
}

int getTile(int x, int y) {
    if (!isTileInBounds(x,y)) {
        return TILE_EMPTY;
    }
    return g_tileMap[x][y];
}


void setTile(int x, int y, TileValue value){
    if (!isTileInBounds(x,y)) {
        return;
    }
        g_tileMap[x][y] = (uint8_t)value;
}

void moveTile(int xOrig, int yOrig, int xNew, int yNew) {
    if (!isTileInBounds(xNew, yNew) || !isTileInBounds(xOrig, yOrig)) {
        return;
    }
    
    g_tileMap[xNew][yNew] = (uint8_t)getTile(xOrig, yOrig);
    g_tileMap[xOrig][yOrig] = (uint8_t)TILE_FLOOR;
}


int main() {
    const int screenWidth = 600;
    const int screenHeight = 600;
    int dis = 100;
    int camSize = 6;
    bool choosingFirstTile = false;
    bool choosingSecondTile = false;
    Vector2 firstSelectedPosition;

   for (int x = 0; x < MAP_SIZE; x++) {
        for (int y = 0; y < MAP_SIZE; y++) {
            setTile(x, y, TILE_FLOOR);
       }
    }

   setTile(1, 1, TILE_PLAYER);
   setTile(2, 2, TILE_EMPTY);
   InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");
   SetTargetFPS(60);

  while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        for (int x = 0; x < camSize; x++) {
            for (int y = 0; y < camSize; y++) {



              const int tile = getTile(camX + x, camY + y);

              if (tile == TILE_FLOOR) {
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, RED);
                }
                else if (tile == TILE_PLAYER){
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, WHITE);
                }
            }
        }

        if (choosingFirstTile) DrawRectangle(0, 0, 10, 10, YELLOW);
        if (choosingSecondTile) DrawRectangle(10, 0, 10, 10, BLUE);

        if (IsKeyReleased(KEY_S)) camY++;
        if (IsKeyReleased(KEY_W)) camY--;
        if (IsKeyReleased(KEY_A)) camX--;
        if (IsKeyReleased(KEY_D)) camX++;

        if (IsKeyReleased(KEY_SPACE) && !choosingSecondTile) choosingFirstTile = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && choosingSecondTile) {
            Vector2 mousePosition = GetMousePosition();
            moveTile(firstSelectedPosition.x, firstSelectedPosition.y, calculatePosition(GetMousePosition()).x, calculatePosition(GetMousePosition()).y);
            choosingSecondTile = false;
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
