#include "raylib.h"
#include <stdint.h>
#define MAP_SIZE 10


enum TileValue {
	TILE_EMPTY, 
	TILE_FLOOR, 
	TILE_PLAYER
};

uint8_t g_tileMap[MAP_SIZE][MAP_SIZE] = {};



int isTileInBounds(int x, int y) {
    return x >= 0 && x < MAP_SIZE&& y >= 0 && y < MAP_SIZE;
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

int main() {
    const int screenWidth = 600;
    const int screenHeight = 600;
    int dis = 100;
    
    
    int camX = 0;
    int camY = 0;
    int camSize = 6;
    InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");
    
    SetTargetFPS(60); 



   while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        /*
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (a[x][y] == 0) {
                    DrawRectangle(x * dis, y * dis, dis -1, dis - 1, RED);
                }
                else {
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, WHITE);
                }
            }
        }
        */
		
        g_tileMap[1][1] = TILE_PLAYER;
        for (int x = 0; x < camSize; x++) {
            for (int y = 0; y < camSize; y++) {



               const int tile = getTile(camX + x, camY + y);



               if (tile == TILE_EMPTY) {
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, RED);
                }
                else if (tile == TILE_PLAYER){
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, WHITE);
                }
            }
        }




        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();
            //getTile((int)(mousePosition.x + camX / 100), (int)(mousePosition.y + camY / 100));



       }
        if (IsKeyReleased(KEY_S)) camY++;
        if (IsKeyReleased(KEY_W)) camY--;
        if (IsKeyReleased(KEY_A)) camX--;
        if (IsKeyReleased(KEY_D)) camX++;
        



       EndDrawing();
    }



   // De-Initialization
    
    CloseWindow(); // Close window and OpenGL context



   return 0;
}
