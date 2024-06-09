#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum GameState
{
    FIRST_WINDOW,
    SECOND_WINDOW,
    THIRD_WINDOW,
    FORTH_WINDOW,
    FIFTH_WINDOW,
    SIXTH_WINDOW,

};

enum Levels
{
    Easy,
    Medium,
    Hard,
};

enum Levels Selected_level = Easy;

FILE *ptr;

struct Player
{
    char name[20];
    int score;
} data;

void displayPlayerData(struct Player player)
{
    DrawText(TextFormat("Name: %s", player.name), 32, 210, 40, WHITE);
    DrawText(TextFormat("Score: %d", player.score), 32, 270, 40, WHITE);
}

const int initial_lives = 5;
const int window_width = 480;
const int window_height = 640;
const Vector2 window_size = {window_width, window_height};
int lives;
int score = 0;
int final_score = 100;

// For main car
typedef struct
{
    Texture2D tex;
    Vector2 pos;
} Sprite;

// For obstacle
typedef struct
{
    Texture2D tex;
    int x;
    int y;
    int width;
    int height;
} Obstacle;

// To generate random value for obstacle position
int random(void)
{
    int rnd;
    int num = rand() % 3;
    switch (num)
    {
    case 0:
        rnd = (window_width / 6) - 60; // left pos
        return rnd;
    case 1:
        rnd = (window_width / 2) - 60; // center pos
        return rnd;
    case 2:
        rnd = (window_width / 1.2) - 60; // right pos
        return rnd;

    default:
        break;
    }

    return -1;
}

int main(void)
{

    srand(time(NULL));

    SetTargetFPS(60);
    InitWindow(window_width, window_height, "CAR GAME");
    InitAudioDevice();
    Texture2D background = LoadTexture("raceflag2.png");
   
    Music music1 = LoadMusicStream("rock beat.mp3");

    bool GameOver = false;
    bool GameWon = false;

    enum GameState gameState = FIRST_WINDOW;
    struct Player data;
    while (!WindowShouldClose())
    {
        switch (gameState)
        {

        case FIRST_WINDOW:
        {

            UpdateMusicStream(music1);
            PlayMusicStream(music1);
            BeginDrawing();

            ClearBackground(BLACK);

            DrawTextureEx(background, (Vector2){0, 0}, 0, 0.69, WHITE);

            DrawText("CAR GAME", 113, 270, 50, WHITE);
            DrawText("Press enter key to proceed", 113, 340, 19, WHITE);

            if (IsKeyPressed(KEY_ENTER))
            {
                StopMusicStream(music1);
                gameState = SECOND_WINDOW;
            }
            EndDrawing();

            break;
        }
        case SECOND_WINDOW:
        {
            static char playerName[20]; // Buffer to store player name
            static int bufferIndex = 0; // Index to keep track of the buffer position

            DrawText("Enter your name:", 25, 210, 30, WHITE);

            // Check if any key is pressed (except Enter and Backspace)
            for (int key = KEY_A; key <= KEY_Z; key++)
            {
                if (IsKeyPressed(key))
                {
                    if (bufferIndex < sizeof(playerName) - 1)
                    {
                       playerName[bufferIndex++] = IsKeyDown(KEY_LEFT_SHIFT)  ? (char)(key - KEY_A + 'A') : (char)(key - KEY_A + 'a');
                        playerName[bufferIndex] = '\0';                        // Null-terminate the string
                    }
                }
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                if (bufferIndex > 0)
                {
                    playerName[--bufferIndex] = '\0'; // Remove the last character
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);

            DrawRectangleLines(30, 260, window_width - 60, 50, WHITE);
            DrawText(playerName, 45, 270, 30, WHITE);

            DrawText("Press enter key to proceed", 113, 340, 19, WHITE);
            EndDrawing();

            if (IsKeyPressed(KEY_ENTER))
            {
                // Copy the final player name to the data structure
                strncpy(data.name, playerName, sizeof(data.name) - 1);
                data.name[sizeof(data.name) - 1] = '\0'; // Null-terminate to ensure a valid string

                gameState = THIRD_WINDOW;
            }
            break;
        }

        case THIRD_WINDOW:
        {
            BeginDrawing();
            ClearBackground(BLACK);

            DrawText("Levels", 30, 30, 80, WHITE);
            DrawText("Easy", 30, 180, 39, (Selected_level == Easy) ? WHITE : GRAY);
            DrawText("Medium", 30, 240, 39, (Selected_level == Medium) ? WHITE : GRAY);
            DrawText("Hard", 30, 300, 39, (Selected_level == Hard) ? WHITE : GRAY);

            EndDrawing();

            if (IsKeyPressed(KEY_DOWN))
            {
                // Selected_level = (Selected_level == Hard) ? Easy : (Selected_level + 1);
                Selected_level = Selected_level + 1;
            }
            else if (IsKeyPressed(KEY_UP))
            {
                // Selected_level = (Selected_level == Easy) ? Hard : (Selected_level - 1);
                Selected_level = Selected_level - 1;
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = FORTH_WINDOW;
            }
            break;
        }

        case FORTH_WINDOW:
        {
            GameOver = false;
            GameWon = false;

            Music music2 = LoadMusicStream("stranger-things-124008.mp3");

            int world_height = window_height * 50;
            int obs_count = world_height / (0.5 * window_height);
            lives = initial_lives;
            int score = 0;
            bool vulnerable = true;
            double vulnerable_time = 0;
            double time = 0;

            // main car
            Sprite car;
            car.tex = LoadTexture("car_piel1.png"); // is of dimension 80 * 120
            car.pos.x = window_width / 2 - car.tex.width / 2;
            car.pos.y = window_height - car.tex.height - 20;

            int car_velocity;
            if (Selected_level == Easy)
            {
                car_velocity = 6;
            }
            else if (Selected_level == Medium)
            {
                car_velocity = 10;
            }
            else if (Selected_level == Hard)
            {
                car_velocity = 12;
            }

            Vector2 camera_offset = {
                .x = 0,
                .y = 0,
            };
            Vector2 camera_target = {
                .x = 0,
                .y = 0,
            };

            Camera2D camera = {
                .offset = camera_offset,
                .target = camera_target,
                .rotation = 0,
                .zoom = 1,
            };

            Texture2D obstacle_texture = LoadTexture("car_piel1.png");
            Obstacle obs[obs_count];

            for (int i = 0; i < obs_count; i++)
            {
                obs[i].x = random();
                if (Selected_level == Easy)
                {
                    obs[i].y = 280 - 320 * i;
                }

                else if (Selected_level == Medium)
                {
                    obs[i].y = 220 - 320 * i;
                }
                else if (Selected_level == Hard)
                {
                    obs[i].y = 160 - 320 * i;
                }
                // obs[i].y = 280 - 320 * i; // Adjusted the y position for each obstacle
                obs[i].width = 80;   // Set the width of the obstacle
                obs[i].height = 120; // Set the height of the obstacle
                

                int a= rand() % 3 +1;
                switch (a)
                {
                case 1:
                    obs[i].tex = LoadTexture("blue car png-03.png");
                    break;
                case 2:
                     obs[i].tex = LoadTexture("black car png-04.png");
                    break;
                case 3:
                    obs[i].tex = LoadTexture("light green car png-03.png");        
                    break;
                
                }
            }

            while (!GameOver)
            {
                UpdateMusicStream(music2);
                PlayMusicStream(music2);

                for (int i = 0; i < obs_count; i++)
                {
                    Rectangle rec1 = {car.pos.x, car.pos.y, car.tex.width, car.tex.height};
                    Rectangle rec2 = {obs[i].x, obs[i].y, obs[i].width, obs[i].height};

                    if (CheckCollisionRecs(rec1, rec2))
                    {
                        if (vulnerable == true)
                        {
                            lives -= 1;
                            car.pos.y -= 120 - 10;
                            vulnerable = false;
                        }
                    }
                }

                if (!vulnerable)
                {
                    vulnerable_time += GetFrameTime();
                    if (vulnerable_time > 1)
                    {
                        vulnerable = true;
                        vulnerable_time = 0;
                    }
                }

                time += GetFrameTime();
                if (time > 1)
                {
                    score++;
                    time = 0;
                }

                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
                {
                    car.pos.x -= car_velocity;
                }

                else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
                {
                    car.pos.x += car_velocity;
                }

                else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
                {
                    car.pos.y -= car_velocity;
                }

                else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
                {
                    car.pos.y += car_velocity;
                }

                // both ifs does not allow the car to move past the x-dimensions
                if (car.pos.x > window_width)
                {
                    car.pos.x -= car.tex.width;
                }
                if (car.pos.x < 0)
                {
                    car.pos.x += car.tex.width / 2;
                }

                if (lives <= 0)
                {
                    GameOver = true;
                }

                if (score >= final_score)
                {
                    GameWon = true;
                    GameOver = true;
                }

                data.score = score;
                BeginDrawing();

                BeginMode2D(camera);

                if (car.pos.y > window_height * 0.8)
                {
                    camera.offset.y = -(car.pos.y - window_height * 0.8);
                }
                else if (car.pos.y < window_height * 0.35)
                {
                    camera.offset.y = -(car.pos.y - window_height * 0.35);
                }

                ClearBackground(GRAY);
                DrawRectangle(window_width / 3 - 1, -camera.offset.y, 2, world_height, BLACK);
                DrawRectangle(window_width * 2 / 3 - 1, -camera.offset.y, 2, world_height, BLACK);

                // obstacle
                for (int i = 0; i < obs_count; i++)
                {
                    DrawTexture(obs[i].tex, obs[i].x, obs[i].y, WHITE);
                }

                // main car
                DrawTexture(car.tex, car.pos.x, car.pos.y, RED);

                const char *scoretext = TextFormat("SCORE = %d", score);
                DrawText(scoretext, 16, -camera.offset.y, 28, WHITE);

                const char *livestext = TextFormat("LIVES = %d", lives);
                DrawText(livestext, 256, -camera.offset.y, 28, WHITE);

                EndMode2D();
                EndDrawing();
            }

            gameState = FIFTH_WINDOW;
            UnloadMusicStream(music2);
            UnloadTexture(obstacle_texture);
        }

        case FIFTH_WINDOW:
        {

            Sound sound = LoadSound("congratulations-deep-voice-172193.ogg");

            BeginDrawing();
            ClearBackground(BLACK);
            if (GameOver)
            {
                if (GameWon == true)
                {
                    DrawText("Yon Won", 113, 270, 50, WHITE);
                    PlaySound(sound);
                    DrawText("Press enter key to proceed", 113, 340, 19, WHITE);
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        gameState = SIXTH_WINDOW;
                    }
                }

                else
                {
                    DrawText("Game Over", 113, 270, 50, WHITE);
                    DrawText("Press Enter key to proceed", 113, 340, 19, WHITE);
                    if (IsKeyReleased(KEY_ENTER))
                    {
                        gameState = SIXTH_WINDOW;
                    }
                }
            }

            EndDrawing();

            break;
        }

        case SIXTH_WINDOW:
        {
            BeginDrawing();
            ClearBackground(BLACK);

            ptr = fopen("score.bin", "w");
            if (ptr != NULL)
            {
                fwrite(&data, sizeof(data), 1, ptr);
                fclose(ptr);
            }
            else
            {
                printf("Error opening file for writing\n");
            }
            // Read data from the file
            ptr = fopen("score.bin", "r");
            if (ptr != NULL)
            {
                struct Player var;

                // Read data from the file
                size_t elements_read = fread(&var, sizeof(struct Player), 1, ptr);

                fclose(ptr);

                // Check if reading was successful
                if (elements_read == 1)
                {
                    displayPlayerData(var);
                }
                else
                {
                    printf("Error reading data from file\n");
                }

                DrawText("Press Backspace key to return", 113, 340, 19, WHITE);
                if(IsKeyPressed(KEY_BACKSPACE))
                {
                    gameState=FIRST_WINDOW;
                    break;
                }            

                EndDrawing();
                break;
            }
        }
        default:
            break;
        }
    }

    UnloadTexture(background);
    CloseAudioDevice();
    CloseWindow();
}
