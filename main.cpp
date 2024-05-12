// Include necessary headers
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <Windows.h>

using namespace std;

int screenWidth = 120;
int screenHeight = 40;
int nMapWidth = 16;
int nMapHeight = 16;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;
float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;
float fSpeed = 5.0f;

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

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    // Main game loop
    while (1)
    {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Controls
        // CCW-Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

        // Forward/Reverse Movement
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;

            // Reverses movement if player hits the wall
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;

            // Reverses movement if player hits the wall
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
            }
        }

        for (int x = 0; x < screenWidth; x++)
        {
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)screenWidth) * fFOV;
            float fStepSize = 0.1f;
            float fDistanceToWall = 0.0f;
            bool bHitWall = false;
            bool bBoundary = false;
            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            // Cast the ray to find the distance to the wall
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += fStepSize;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
                    {
                        bHitWall = true;
                        vector<pair<float, float>> p;

                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx*vx + vy*vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }


            // Calcuate celing and floor distance based on wall distance
            int nCeiling = (float)(screenHeight / 2.0) - screenHeight / ((float)fDistanceToWall);
            int nFloor = screenHeight - nCeiling;

            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f)          nShade = 0x2588;    // Very close    
            else if (fDistanceToWall < fDepth / 3.0f)       nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)       nShade = 0x2592;
            else if (fDistanceToWall < fDepth)               nShade = 0x2591;
            else                                            nShade = ' ';

            if (bBoundary)        nShade = ' ';

            for (int y = 0; y < screenHeight; y++)
            {
                if (y <= nCeiling)
                    screen[y*screenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y*screenWidth + x] = nShade;
                else
                {
                    float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
                    if (b < 0.25)        nShade = '#';
                    else if (b < 0.5)    nShade = 'x';
                    else if (b < 0.75)    nShade = '.';
                    else if (b < 0.9)    nShade = '-';
                    else                nShade = ' ';
                    screen[y*screenWidth + x] = nShade;
                }
            }
        }

        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1)*screenWidth + nx] = map[ny * nMapWidth + nx];
            }
        screen[((int)fPlayerX + 1) * screenWidth + (int)fPlayerY] = 'o';


        // Null-terminate the screen buffer
        screen[screenWidth * screenHeight - 1] = '\0';

        // Update the console with the screen buffer
        WriteConsoleOutputCharacterW(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten); 
    }

    return 0;
}

