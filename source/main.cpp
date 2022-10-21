#include "raylib.h"

int main() {
    // Initialization
	
    const int screenWidth = 400;
    const int screenHeight = 400;
    int dis = 100;
    int a[4][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} };

    InitWindow(screenWidth, screenHeight, "raylib [core] example - keyboard input");
    
    SetTargetFPS(60); // Set our game to run at 60 frames-per-second when possible


    // Main game loop
	
	// `WindowShouldClose` detects window close
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // Update
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


        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePosition = GetMousePosition();
            a[(int)(mousePosition.x / 100)][(int)(mousePosition.y / 100)] = 1;
        }
        EndDrawing();
    }

    // De-Initialization
	
    CloseWindow(); // Close window and OpenGL context

    return 0;
}