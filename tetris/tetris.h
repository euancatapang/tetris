#pragma once

#include <stdint.h>
#include <windows.h>

// BOARD CONSTANTS
#define TETROMINO_CELL_ROWS 20 // DEFAULT 20
#define TETROMINO_CELL_COLS 10 // DEFAULT 10

#define TERMINAL_ROWS (TETROMINO_CELL_ROWS*1) // DEFAULT (TETROMINO_CELL_ROWS*1)
#define TERMINAL_COLS (TETROMINO_CELL_COLS*2) // DEFAULT (TETROMINO_CELL_ROWS*2)

#define BOARD_HEIGHT (TERMINAL_ROWS+2)
#define BOARD_WIDTH (TERMINAL_COLS+2)

// BORDERS
#define BORDER_WIDTH 1

#define NEXT_PIECE 0 
#define CACHE_BOX 11 // Y-axis offset

// SHAPE PREVIEW OFFSET
#define SHAPE_PREVIEW_OFFSET_X (BOARD_WIDTH+7)
#define SHAPE_PREVIEW_OFFSET_Y 2

// CONTROLS OFFSET
#define CONTROLS_OFFSET 1

// PLACE SHAPE MODES
#define CLEAR 0
#define PLACE_SHAPE 1

// TETROMINOES
#define _I 0
#define _O 1
#define _T 2
#define _S 3
#define _Z 4
#define _J 5
#define _L 6

// DEFAULT SHAPE SPAWN
#define DEFAULT_X (TETROMINO_CELL_COLS/2-1)
#define DEFAULT_Y 0
#define DEFAULT_ROTATION 0

// CELL
#define FILLED_CELL L"██"

#define HALF_CELL L'█'
#define EMPTY_HALF_CELL L' '

// BAG RANDOMIZER
#define BAG_SIZE 7

// MOVE SHAPE DOWN INTERVAL
#define MOVE_DOWN_INTERVAL_MS 1000

// KICK TABLE
#define KICK_TEST_COUNT 5
#define KICK_TEST(start, end) (start*10 + end)

#define EXCEEDS(x, y) (x >= TETROMINO_CELL_COLS || x < 0 || y >= TETROMINO_CELL_ROWS || y < 0)

typedef enum {
    PLACE_INVALID,
    PLACE_VALID,

} PlacementValidity;

typedef enum {
    MOVE_DOWN_NO_CHANGE,
    MOVE_DOWN_FAIL,
    MOVE_DOWN_SUCCESS,

} MoveDownStatus;

typedef enum {
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_UP,
    INPUT_CLOCKWISE,
    INPUT_A_CLOCKWISE,
    INPUT_HARD_DOWN,
    INPUT_CACHE_PIECE

} ActionInput;

typedef enum {
    ACTION_TYPE_MOVE,
    ACTION_TYPE_ROTATE,
    ACTION_TYPE_SPECIAL,

} ActionType;

typedef struct {
    int8_t x;
    int8_t y;

} Coord;

typedef struct {
    int8_t x;
    int8_t y;
    int8_t shape;
    int8_t rotation;
    int8_t borderedBoardWithoutShape[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS];

} Shape;

typedef struct {
    ActionInput input;
    ActionType type;

} Action;

typedef struct {
    uint8_t index;
    uint8_t shapes[BAG_SIZE];
    
} Bag;

typedef struct {
    uint32_t score;
    uint32_t previousScore;
    uint32_t highScore;

} Scores;

// HARDCODED TETROMINOES
extern const int8_t _tetrominoes[7][4][4][4];
extern const WORD _colors[7];
extern const Coord _kickTable[2][8][KICK_TEST_COUNT];