#include <Windows.h>
#include <chrono>
#include <iostream>
#include <math.h>

using namespace std;
int screenWidth = 120;
int screenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main() {
  // Create a screen buffer to draw the game
  wchar_t *screen = new wchar_t[screenWidth * screenHeight];
  HANDLE hConsole = CreateConsoleScreenBuffer(
      GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
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
  while (1) {

    tp2 = chrono::system_clock::now();
    chrono::duration<float> elapsedTime = tp2 - tp1;
    tp1 = tp2;
    float fElapsedTime = elapsedTime.count();

    // Controls
    // CCW-Rotation
    if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
      fPlayerA -= (0.8f) * fElapsedTime;
    if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
      fPlayerA += (0.8f) * fElapsedTime;
    if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
      fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
      fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
    }
    if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
      fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
      fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
    }

    // For each column of the screen
    for (int x = 0; x < screenWidth; x++) {
      // Calculate the ray angle for the current column
      float fRayAngle =
          (fPlayerA - fFOV / 2.0f) + ((float)x / (float)screenWidth) * fFOV;

      float fDistanceToWall = 0.0f;
      bool bHitWall = false;

      // Calculate the direction of the ray
      float fEyeX = sinf(fRayAngle);
      float fEyeY = cosf(fRayAngle);

      // Cast the ray to find the distance to the wall
      while (!bHitWall && fDistanceToWall < fDepth) {
        fDistanceToWall += 0.1f;

        int nTextX = (int)(fPlayerX + fEyeX * fDistanceToWall);
        int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

        // Check if the ray is out of bounds
        if (nTextX < 0 || nTextX >= nMapWidth || nTestY < 0 ||
            nTestY >= nMapHeight) {
          bHitWall = true;
          fDistanceToWall = fDepth;
        } else {
          // Check if the ray hit a wall block
          if (map[nTestY * nMapWidth + nTextX] == '#') {
            bHitWall = true;
          }
        }
      }

      // Calculate ceiling and floor distance based on wall distance
      int nCeiling =
          (float)(screenHeight / 2.0) - screenHeight / ((float)fDistanceToWall);
      int nFloor = screenHeight - nCeiling;

      short nShade = ' ';

      if (fDistanceToWall <= fDepth / 4.0f)
        nShade = 0x2599; // Close
      else if (fDistanceToWall < fDepth / 3.0f)
        nShade = 0x2593;
      else if (fDistanceToWall < fDepth / 2.0f)
        nShade = 0x2592;
      else if (fDistanceToWall < fDepth)
        nShade = 0x2591;
      else
        nShade = ' ';

      // Draw the column on the screen
      for (int y = 0; y < screenHeight; y++) {
        if (y < nCeiling)
          screen[y * screenWidth + x] = ' ';
        else if (y > nCeiling && y <= nFloor)
          screen[y * screenWidth + x] = nShade;
        else {
          float b = 1.0f - (((float)y - screenHeight / 2.0f) /
                            ((float)screenHeight / 2.0f));
          if (b < 0.25)
            nShade = '#';
          else if (b < 0.5)
            nShade = 'x';
          else if (b < 0.75)
            nShade = '.';
          else if (b < 0.9)
            nShade = '_';
          else
            nShade = ' ';
          screen[y * screenWidth + x] = nShade;
        }
      }
    }

    // Null-terminate the screen buffer
    screen[screenWidth * screenHeight - 1] = '\0';

    // Update the console with the screen buffer
    WriteConsoleOutputCharacterW(hConsole, screen, screenWidth * screenHeight,
                                 {0, 0}, &dwBytesWritten);
  }

  return 0;
}
