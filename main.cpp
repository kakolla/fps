
#include <iostream>
#include <cmath>
#include <Windows.h>
#include <conio.h>  // for macos converted inputs
#include <chrono> // for fps


using namespace std;

int screenWidth = 120;
int screenHeight = 40;


float fPlayerX = 8.0f; // player X coord 
float fPlayerY = 8.0f; // player Y coord
float fPlayerAngle = 0.0f; // player angle 

// Map dims
int nMapHeight = 16;
int nMapWidth = 16;



float fFOV = 3.141592 / 4.0; // field of view
float fDepth = 16.0f;

int main() {
    // map of Wide strings
    wstring map;    
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    // Create unicode screen array 
    wchar_t *screen = new wchar_t[screenHeight*screenWidth];

    // put buffer into screen so it doesn't scroll
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole); // set buffer target to the console
    DWORD dwBytesWritten = 0;

   
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();


    // Game loop
    while(1) {
        // get system time for fps synchronization
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
        

        // Controls
        // rotation CCW & CW 
        if (_kbhit()) { // non blocking IO
            char ch = _getch();
            if (ch == 'a' || ch == 'A')
                fPlayerAngle -= 0.1f * fElapsedTime * 20.0f;
            else if (ch == 'd' || ch == 'D')
                fPlayerAngle += 0.1f * fElapsedTime * 20.0f;
            else if (ch == 'w' || ch == 'W') {
                // direction unit vector * movement speed 
                fPlayerX += sinf(fPlayerAngle) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerAngle) * 5.0f * fElapsedTime;
            }
            else if (ch == 's' || ch == 'S') {
                // direction unit vector * movement speed 
                fPlayerX -= sinf(fPlayerAngle) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerAngle) * 5.0f * fElapsedTime;
            }
        }



        for (int x = 0; x < screenWidth; ++x) {
            // for each column (x), calc projected ray angle into world space
            /*
                *FOV is 2 rays beaming out of player*
                ( fPlayerAngle - fFOV/2 ) is left ray 
                x/screenWidth * fFOV --> slice FOV into screenwidth # of rays, 
                then pick the correct offset from leftmost ray, which gets us the angle for this column

                x=0 is leftmost ray, x=screenwidth/2 is center ray, x= screenwidth-1 is rightmost ray
            */
            float fRayAngle = (fPlayerAngle - fFOV / 2.0f) + ((float)x / (float)screenWidth) * fFOV;
            // now we have angle to look at to see this column

            float fDistanceToWall = 0;
            bool bHitWall = false; // keep track if we hit wall

            // Need to get a direction vector using angle (how far to step in x & w dirs)
            float fEyeX = sinf(fRayAngle); // x component of dir vector
            float fEyeY = cosf(fRayAngle); // y component

            while (!bHitWall && fDistanceToWall < fDepth ) {
                // trace the ray incrementally until we hit a wall
                // 'shooting' the ray
                fDistanceToWall += 0.1f; 
                // while tracing, check if we are out of bounds
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // out of bounds (hit wall)
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                } else {
                    // if we are within bounds of map
                    // dist to wall will be less than fDepth
                    
                    // check if we hit a wall -- check if ray cell is a wall block
                    if (map[nTestY * nMapWidth + nTestX] == '#') {
                        bHitWall = true;
                    }
                }

            }
            
            // ceiling is larger for larger distances
            int nCeiling = (float)(screenHeight / 2.0) - screenHeight / ((float)fDistanceToWall);
            int nFloor = screenHeight - nCeiling;


            // -- Shading --
            // determine which char to render (shading) based on perspective (distance) of wall
            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588;
            else if (fDistanceToWall < fDepth / 3.0f) nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f) nShade = 0x2592;
            else if (fDistanceToWall < fDepth ) nShade = 0x2591;
            else nShade = ' ';




            // clamp nCeiling & nFloor
            // to screen height range
            nCeiling = max(0, min(screenHeight -1, nCeiling));
            nFloor = max(0, min(screenHeight -1, nFloor));


            // calculate distance to ceiling and floor
            for (int y = 0; y < screenHeight; y++) {
                if (y < nCeiling) screen[y*screenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor) screen[y*screenWidth + x] = nShade;
                else screen[y*screenWidth + x] = '.';
            }

        }


        // Write to screen
        screen[screenWidth * screenHeight - 1] = '\0'; // set last char of screen array to null char
        // handle, buffer, number of bytes, coords of text to be written, 
        WriteConsoleOutputCharacterW(hConsole, screen, screenWidth*screenHeight, {0,0}, &dwBytesWritten);


        Sleep(8);
    }

    return 0;

}