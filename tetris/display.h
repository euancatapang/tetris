#pragma once

#include <windows.h>
#include <stdint.h>
#include "tetris.h"

#define ESCAPE_NEWLINES 10

typedef struct {
	HANDLE h;
	CONSOLE_SCREEN_BUFFER_INFO info;
	WORD baseConsoleAttributes;
	DWORD written;

} Terminal;

// FUNCTION TEMPLATES

// INITIAL SETUP
void initializeDisplay(Terminal* terminal);
void setupGameBoard(Terminal* terminal);
void printBorder(Terminal* terminal);
void printSideBoxesBorder(Terminal* terminal, int8_t mode);
void printControls(Terminal* terminal);
void initializeScoreDisplay(Terminal* terminal, int32_t highScore);

// REFRESH DISPLAY
void getShapeFootprint(const Shape* shape, bool output[4]);

COORD getDisplayPositionOfCell(int8_t xCellPos, int8_t yCellPos);
void refreshBoardDisplay(Terminal* terminal, const int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);
void updateShapeShadow(Terminal* terminal, Shape* shape);
void changeSideBoxDisplayShape(Terminal* terminal, int8_t shape, int8_t mode);
void updateScoreDisplay(Terminal* terminal, uint32_t score);
void updateHighScoreDisplay(Terminal* terminal, uint32_t highScore);
void showLoseScreen(Terminal* terminal, uint32_t score, bool newHighScore);