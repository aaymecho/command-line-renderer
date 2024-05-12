#include <iostream>
using namespace std;

#include <Windows.h>
#include <math.h>

int screenWidth = 120;
int screenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main()
{
    // Create a screen buffer to draw the game
    wchar_t *screen = new wchar_t[screenWidth * screenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Define the game map
    wstring map;

    map += L"#########.......";
    map += L"#...............";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#......####..###";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";

    // Main game loop
    while (1)
    {
        // For each column of the screen
        for (int x = 0; x < screenWidth; x++)
        {
            // Calculate the ray angle for the current column
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)screenWidth) * fFOV;

            float fDistanceToWall = 0.0f;
            bool bHitWall = false;

            // Calculate the direction of the ray
            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // Cast the ray to find the distance to the wall
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f;

                int nTextX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // Check if the ray is out of bounds
                if (nTextX < 0 || nTextX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // Check if the ray hit a wall block
                    if (map[nTestY * nMapWidth + nTextX] == '#')
                    {
                        bHitWall = true;
                    }
                }
            }

            // Calculate ceiling and floor distance based on wall distance
            int nCeiling = (float)(screenHeight / 2.0) - screenHeight / ((float)fDistanceToWall);
            int nFloor = screenHeight - nCeiling;

            // Draw the column on the screen
            for (int y = 0; y < screenHeight; y++)
            {
                if (y < nCeiling)
                    screen[y * screenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y * screenWidth + x] = '#';
                else
                    screen[y * screenWidth + x] = ' ';
            }
        }

        // Null-terminate the screen buffer
        screen[screenWidth * screenHeight - 1] = '\0';

        // Update the console with the screen buffer
        WriteConsoleOutputCharacterW(hConsole, screen, screenWidth * screenHeight, {0, 0}, &dwBytesWritten);
    }

    return 0;
}

