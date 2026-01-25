#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "display.h"
#include "tetris.h"

// INITIAL SETUP

// Expand display area to fit board
void initializeDisplay(Terminal* terminal) {
    system("cls");
    SetConsoleOutputCP(CP_UTF8);

    terminal->h = GetStdHandle(STD_OUTPUT_HANDLE);
    
    GetConsoleScreenBufferInfo(terminal->h, &terminal->info);
    terminal->baseConsoleAttributes = (terminal->info).wAttributes;

    SMALL_RECT consoleArea = { 0, 0, BOARD_WIDTH * 2, BOARD_HEIGHT + CONTROLS_OFFSET + 7 };
    SetConsoleWindowInfo(terminal->h, true, &consoleArea);
}

void setupGameBoard(Terminal* terminal) {
    printBorder(terminal);
    printSideBoxesBorder(terminal, CACHE_BOX);
    printSideBoxesBorder(terminal, NEXT_PIECE);
    printControls(terminal);
}

void printBorder(Terminal* terminal) {

    // Print top border (bottom border is updated per refresh)
    SetConsoleCursorPosition(terminal->h, (COORD) { 0, 0 });
    for (uint8_t j = 0; j < BOARD_WIDTH; j++) {
        printf("-");
    }

    // Print left and right border
    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < BOARD_HEIGHT; j++) {
            SetConsoleCursorPosition(terminal->h, (COORD) { i* (BOARD_WIDTH - 1), j });
            printf("|");
        }
    }
}

void printSideBoxesBorder(Terminal* terminal, int8_t mode) {

    // Print top and bottom box border
    for (uint8_t i = 0; i < 2; i++) {
        SetConsoleCursorPosition(terminal->h, (COORD) { BOARD_WIDTH + 3, (i * 7) + mode });

        for (uint8_t j = 0; j < 16; j++) {
            printf("-");
        }
    }

    // Print left and right box border
    for (uint8_t i = 0; i < 2; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            SetConsoleCursorPosition(terminal->h, (COORD) { (i * 15) + (BOARD_WIDTH + 3), j + mode });
            printf("|");
        }
    }
}

void printControls(Terminal* terminal) {
    SetConsoleCursorPosition(terminal->h, (COORD) { 0, BOARD_HEIGHT + CONTROLS_OFFSET });
    printf("'A' - Left\n");
    printf("'D' - Right\n");
    printf("'S' - Down\n");
    printf("' ' - Hard Down\n");
    printf("'E' - Rotate Clockwise\n");
    printf("'Q' - Rotate Counterclockwise\n");
    printf("'R' - Switch Piece\n");
}

void initializeScoreDisplay(Terminal* terminal, int32_t highScore) {
    SetConsoleCursorPosition(terminal->h, (COORD) { BOARD_WIDTH + 4, BOARD_HEIGHT - 2 });
    printf("Score : ");
    SetConsoleCursorPosition(terminal->h, (COORD) { BOARD_WIDTH + 4, BOARD_HEIGHT - 1 });
    printf("HScore: %06d", highScore);
}

// REFRESH DISPLAY

// Converts cell position coordinates to console type, COORD.
COORD getDisplayPositionOfCell(int8_t xCellPos, int8_t yCellPos) {
    return (COORD) { (xCellPos * 2) + 1, yCellPos + 1 };
}

void pushToPrintBuffer(CHAR_INFO buffer[], uint16_t* bufferIndex, WCHAR letter, WORD attribute) {
    buffer[*bufferIndex].Char.UnicodeChar = letter;
    buffer[*bufferIndex].Attributes = attribute;
    (*bufferIndex)++;
    buffer[*bufferIndex].Char.UnicodeChar = letter;
    buffer[*bufferIndex].Attributes = attribute;
    (*bufferIndex)++;
}

// Reads entire current board (2D array) and prints colors based on value.
void refreshBoardDisplay(Terminal* terminal, const int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {

    CHAR_INFO buffer[TERMINAL_ROWS * TERMINAL_COLS];
    SMALL_RECT printArea = { 1, 1, TERMINAL_ROWS, TERMINAL_COLS };

    uint16_t bufferIndex = 0;

    for (uint8_t row = 0; row < TETROMINO_CELL_ROWS; row++) {
        for (uint8_t col = 0; col < TETROMINO_CELL_COLS; col++) {

            int8_t currentCell = boardLayer[row][col];

            if (currentCell > 0) {
                pushToPrintBuffer(buffer, &bufferIndex, HALF_CELL, _colors[currentCell - 1]);
            }
            else {
                pushToPrintBuffer(buffer, &bufferIndex, EMPTY_HALF_CELL, terminal->baseConsoleAttributes);
            }
        }
    }

    WriteConsoleOutputW(terminal->h, buffer, (COORD) { TERMINAL_COLS, TERMINAL_ROWS }, (COORD) { 0, 0 }, &printArea);
}

// Writes to bool array[4] where tetromino col with at least one cell is true
void getShapeFootprint(const Shape* shape, bool output[4]) {
    for (uint8_t col = 0; col < 4; col++) {
        for (uint8_t row = 0; row < 4; row++) {
            if (_tetrominoes[shape->shape][shape->rotation][row][col]) {
                output[col] = true;
                break;
            }
        }
    }
}

// Gets footprint of shape; uses to refresh bottom border (shadow indicator)
void updateShapeShadow(Terminal* terminal, Shape* shape) {

    CHAR_INFO buffer[TERMINAL_COLS];
    uint16_t bufferIndex = 0;

    bool footprint[4] = { false };
    getShapeFootprint(shape, footprint);

    for (uint8_t x = 0; x < TETROMINO_CELL_COLS; x++) {
        WCHAR letter = L'-';

        if (x >= shape->x && x < shape->x + 4) {
            if (footprint[x - shape->x]) {
                letter = L'#';
            }
        }
        pushToPrintBuffer(buffer, &bufferIndex, letter, terminal->baseConsoleAttributes);
    }

    SMALL_RECT printArea = { 1, TERMINAL_ROWS + 1, TERMINAL_COLS, TERMINAL_ROWS + 1 };
    WriteConsoleOutputA(terminal->h, buffer, (COORD) { TERMINAL_COLS, 1 }, (COORD) { 0, 0 }, &printArea);
}

// Prints shape displayed in specified side box
void changeSideBoxDisplayShape(Terminal* terminal, int8_t shape, int8_t mode) {

    uint16_t bufferIndex = 0;
    CHAR_INFO buffer[32];

    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {

            int8_t currentCell = _tetrominoes[shape][0][row][col];

            if (currentCell > 0) {
                pushToPrintBuffer(buffer, &bufferIndex, HALF_CELL, _colors[shape]);
            }
            else if (currentCell == 0) {
                pushToPrintBuffer(buffer, &bufferIndex, EMPTY_HALF_CELL, terminal->baseConsoleAttributes);
            }
            else {
                printf("EROR");
            }
        }
    }

    SMALL_RECT printArea = { SHAPE_PREVIEW_OFFSET_X, SHAPE_PREVIEW_OFFSET_Y + mode, SHAPE_PREVIEW_OFFSET_X + 6, SHAPE_PREVIEW_OFFSET_Y + 3 + mode};
    WriteConsoleOutputW(terminal->h, buffer, (COORD) { 8, 4 }, (COORD) { 0, 0 }, & printArea);
}

void updateScoreDisplay(Terminal* terminal, uint32_t score) {
    SetConsoleCursorPosition(terminal->h, (COORD) { BOARD_WIDTH + 12, BOARD_HEIGHT - 2 });
    printf("%06d", score);
}

void updateHighScoreDisplay(Terminal* terminal, uint32_t highScore) {
    SetConsoleCursorPosition(terminal->h, (COORD) { BOARD_WIDTH + 12, BOARD_HEIGHT - 1 });
    printf("%06d", highScore);
}

void showLoseScreen(Terminal* terminal, uint32_t score, bool newHighScore) {
    SetConsoleCursorPosition(terminal->h, (COORD) { 0, BOARD_HEIGHT - 1 });

    for (uint8_t i = 0; i < ESCAPE_NEWLINES; i++) {
        printf("\n");
    }

    printf("==========================================\n");

    if (newHighScore)
        printf("New high score: %d.\n", score);
    else
        printf("Score: %d.\n", score);

    printf("Press any key to exit.\n");

    printf("==========================================\n");
}