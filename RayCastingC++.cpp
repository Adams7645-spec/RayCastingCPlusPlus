#include "pch.h"
#include <string>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#include <datetimeapi.h>
#include <iomanip>
#include <stdio.h>
#include <conio.h>
#include <ctime>
#include <chrono>
#include <list>
#include <vector>
#include <tuple>
#include <algorithm>
#define _WIN32_WINNT 0x0502

using namespace System;

const int windowWidth = 120;
const int windowHight = 60;

const int mapWidth = 20;
const int mapHight = 20;

const double Fov = double(M_PI) / 3;
const double Depth = 10;

static double playerX = 1;
static double playerY = 1;
static double playerA = 0;

static std::string map = "";

static char Window[windowWidth * windowHight];

void HideCursor()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void SetWindowSize(int width, int height)
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    SMALL_RECT windowRect;
    windowRect.Left = 0;
    windowRect.Top = 0;
    windowRect.Right = width - 1;
    windowRect.Bottom = height - 1;

    SetConsoleWindowInfo(consoleHandle, TRUE, &windowRect);
    COORD bufferSize = { width, height };
    SetConsoleScreenBufferSize(consoleHandle, bufferSize);
}
void GoToXY(int x, int y) {
    COORD pos = { x, y };
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, pos);
}
void SetMapGrid() {
    map += "####################";
    map += "#........#.........#";
    map += "#..................#";
    map += "#........#.........#";
    map += "##########.........#";
    map += "#........#.........#";
    map += "#..................#";
    map += "#........#.........#";
    map += "#........####...####";
    map += "#........#.........#";
    map += "#........#.........#";
    map += "#........#.........#";
    map += "###..#######.......#";
    map += "#....#.....#.......#";
    map += "#..................#";
    map += "#....#...#####..####";
    map += "#....#.............#";
    map += "#..................#";
    map += "#....#.............#";
    map += "####################";
}

int main(array<System::String ^> ^args)
{
    HideCursor();
    SetWindowSize(windowWidth, windowHight);
    SetMapGrid();

    std::chrono::system_clock::time_point dateTimeFrom = std::chrono::system_clock::now();

    while (true)
    {
        std::chrono::system_clock::time_point dateTimeTo = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsedTime = dateTimeTo - dateTimeFrom;
        double elapsedSeconds = elapsedTime.count();
        dateTimeFrom = std::chrono::system_clock::now();

        if (Console::KeyAvailable)
        {
            //int consoleKey = std::cin.get();
            int consoleKey = (int)Console::ReadKey().Key;
            switch (consoleKey)
            {
                case (int)(ConsoleKey::A) :
                    playerA += elapsedTime.count() * 10;
                    break;
                    case (int)(ConsoleKey::D) :
                        playerA -= elapsedTime.count() * 10;
                        break;
                        case (int)(ConsoleKey::W) :
                        {
                            playerX += sin(playerA) * 15 * elapsedTime.count();
                            playerY += cos(playerA) * 15 * elapsedTime.count();

                            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
                            {
                                playerX -= sin(playerA) * 15 * elapsedTime.count();
                                playerY -= cos(playerA) * 15 * elapsedTime.count();
                            }
                            break;
                        }
                        case (int)(ConsoleKey::S) :
                        {
                            playerX -= sin(playerA) * 15 * elapsedTime.count();
                            playerY -= cos(playerA) * 15 * elapsedTime.count();

                            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
                            {
                                playerX += sin(playerA) * 15 * elapsedTime.count();
                                playerY += cos(playerA) * 15 * elapsedTime.count();
                            }
                            break;
                        }
            }
            for (int X = 0; X < windowWidth; X++)
            {
                double rayAngle = playerA + Fov / 2 - X * Fov / windowWidth;

                double rayX = sin(rayAngle);
                double rayY = cos(rayAngle);

                double distanceToWall = 0;
                bool hitWall = false;
                bool isBound = false;

                while (!hitWall && distanceToWall < Depth)
                {
                    distanceToWall += 0.1;

                    int testX = (int)(playerX + rayX * distanceToWall);
                    int testY = (int)(playerY + rayY * distanceToWall);

                    if (testX < 0 || testX >= Depth + playerX || testY < 0 || testY >= Depth + playerY)
                    {
                        hitWall = true;
                        distanceToWall = Depth;
                    }
                    else
                    {
                        char testCell = map[testY * mapWidth + testX];
                        if (testCell == '#')
                        {
                            hitWall = true;

                            std::vector<std::tuple<double, double>> boundsVectorList;

                            for (int tx = 0; tx < 2; tx++)
                            {
                                for (int ty = 0; ty < 2; ty++)
                                {
                                    double vx = testX + tx - playerX;
                                    double vy = testY + ty - playerY;

                                    double vectorModule = sqrt(vx * vx + vy * vy);
                                    double cosAngle = rayX * vx / vectorModule + rayY * vy / vectorModule;

                                    boundsVectorList.push_back(std::tuple<double, double>(vectorModule, cosAngle));
                                }
                            }

                            std::sort(boundsVectorList.begin(), boundsVectorList.end(),
                                [](std::tuple<double, double>& a, std::tuple<double, double>& b)
                                {
                                    return std::get<0>(a) < std::get<0>(b);
                                });

                            double boundAngle = 0.03 / distanceToWall;

                            if (acos(std::get<1>(boundsVectorList[0])) < boundAngle ||
                                acos(std::get<1>(boundsVectorList[1])) < boundAngle)
                            {
                                isBound = true;
                            }
                        }
                    }
                }

                int ceiling = (int)(windowHight / (double) 2 - windowHight * Fov / distanceToWall);
                int floor = windowHight - ceiling;

                char wallShade;

                if (isBound)
                {
                    wallShade = '|';
                }
                else if (distanceToWall <= Depth / (double) 4)
                {
                    wallShade = '\u2588';
                }
                else if (distanceToWall < Depth / (double) 3)
                {
                    wallShade = '\u2593';
                }
                else if (distanceToWall < Depth / (double) 2)
                {
                    wallShade = '\u2592';
                }
                else if (distanceToWall < Depth)
                {
                    wallShade = '\u2591';
                }
                else
                {
                    wallShade = ' ';
                }

                for (int Y = 0; Y < windowHight; Y++)
                {
                    if (Y <= ceiling)
                    {
                        Window[Y * windowWidth + X] = ' ';
                    }
                    else if (Y > ceiling && Y <= floor)
                    {
                        Window[Y * windowWidth + X] = wallShade;
                    }
                    else
                    {
                        char floorShade;

                        double b = 1 - (Y - windowHight / (double) 2) / (windowHight / (double) 2);
                        if (b < 0.25)
                        {
                            floorShade = '#';
                        }
                        else if (b < 0.5)
                        {
                            floorShade = 'x';
                        }
                        else if (b < 0.75)
                        {
                            floorShade = '-';
                        }
                        else if (b < 0.9)
                        {
                            floorShade = '.';
                        }
                        else
                        {
                            floorShade = ' ';
                        }

                        Window[Y * windowWidth + X] = floorShade;
                    }
                }
            }

            //char[] stats = $"X: {playerX}, Y: {playerY}, A: {playerA}, FPS: {(int)(1 / elepsedTime)}".ToCharArray();
            //stats.CopyTo(Window, 0);

            GoToXY(0, 0);
            std::cout << Window << std::endl;

            }
        }
    }