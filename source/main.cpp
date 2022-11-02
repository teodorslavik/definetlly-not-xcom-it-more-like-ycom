#include "raylib.h"
#define TILE_EMPTY 0
#define MAP_SIZE 10
int g_tileMap[MAP_SIZE][MAP_SIZE] = {};



int isTileInBounds(int x, int y) {
    return x >= 0 && x < MAP_SIZE&& y >= 0 && y < MAP_SIZE;
}



int getTile(int x, int y) {
    if (!isTileInBounds(x,y)) {
        return TILE_EMPTY;
    }
    return g_tileMap[x][y];
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
        g_tileMap[1][1] = 1;
        for (int x = 0; x < camSize; x++) {
            for (int y = 0; y < camSize; y++) {



               const int tile = getTile(camX + x, camY + y);



               if (tile == 0) {
                    DrawRectangle(x * dis, y * dis, dis - 1, dis - 1, RED);
                }
                else if (tile == 1){
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
