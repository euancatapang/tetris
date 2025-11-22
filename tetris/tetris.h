#include <stdint.h>
#include <windows.h>

#pragma once

// BOARD CONSTANTS
#define CELLROWS 20 // DEFAULT 20
#define CELLCOLS 10 // DEFAULT 10

#define ROWS (CELLROWS*1) // DEFAULT (CELLROWS*1)
#define COLS (CELLCOLS*2) // DEFAULT (CELLROWS*2)

#define BOARD_HEIGHT (ROWS+2)
#define BOARD_WIDTH (COLS+2)

#define CELL "\u2588\u2588"

// BORDERS
#define BORDER_WIDTH 1

#define NEXT_PIECE 0 
#define CACHE_BOX 11 // Y-axis offset

// SHAPE PREVIEW OFFSET
#define SHAPE_PREVIEW_OFFSET_X (BOARD_WIDTH+7)
#define SHAPE_PREVIEW_OFFSET_Y 2

// CONTROLS OFFSET
#define CONTROLS_OFFSET 1

// INPUT ACTIONS
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define UP 4
#define CLOCKWISE 5
#define A_CLOCKWISE 6
#define HARD_DOWN 7
#define CACHE_PIECE 8

// ACTION TYPES
#define MOVE 1
#define ROTATE 2
#define SPECIAL 3

// PLACE SHAPE MODES
#define CLEAR 0
#define PLACE_SHAPE 1

#define NEEDS_KICK_FROM_RIGHT -1
#define NEEDS_KICK_FROM_LEFT -2

// TETROMINOES
#define _I 0
#define _O 1
#define _T 2
#define _S 3
#define _Z 4
#define _J 5
#define _L 6

// DEFAULT SHAPE SPAWN
#define DEFAULT_X 3
#define DEFAULT_Y 0
#define DEFAULT_ROTATION 0

// CELL
#define FILLED_CELL L"██"

// BAG RANDOMIZER
#define BAG_SIZE 7

// HARDCODED TETROMINOES
extern const int8_t _tetrominoes[7][4][4][4];
extern const WORD _colors[7][2];

// GLOBALS
typedef struct {
    int8_t x;
    int8_t y;
    int8_t shape;
    int8_t rotation;
    int8_t borderedBoardWithoutShape[CELLROWS + 2][CELLCOLS + 2];

} Shape;

typedef struct {
    uint8_t name;
    uint8_t type;

} Action;


// FUNCTION TEMPLATES

// conout.c (Console Output)

// INITIAL SETUP
void initializeDisplay();
void printBorder(HANDLE* h);
void printSideBoxesBorder(HANDLE* h, int8_t mode);
void printControls(HANDLE* h);
void initializeScoreDisplay(HANDLE* h);

// REFRESH DISPLAY
COORD getDisplayPositionOfCell(int8_t xCellPos, int8_t yCellPos);
void refreshBoardDisplay(HANDLE* h, int8_t boardLayer[CELLROWS][CELLCOLS], WORD baseConsoleAttributes);
void getShapeFootprint(const Shape* shape, bool output[4]);
void updateShapeShadow(Shape* shape, HANDLE* h);
void changeSideBoxDisplayShape(int8_t shape, int8_t mode, HANDLE* h, WORD baseConsoleAttributes);
void updateScoreDisplay(int score, HANDLE* h);
