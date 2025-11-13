#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "tetris.h"

// Placeholder parameter for printing cell
DWORD _written;

// INITIAL SETUP

// Expand display area to fit board
void initializeDisplay() {
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        printf("\n");
    }
}

void printBorder(HANDLE* h) {

    // Print top border (bottom border is updated per refresh)
    SetConsoleCursorPosition(*h, (COORD) { 0, 0 });
    for (int j = 0; j < BOARD_WIDTH; j++) {
        printf("-");
    }

    // Print left and right border
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < BOARD_HEIGHT; j++) {
            SetConsoleCursorPosition(*h, (COORD) { i* (BOARD_WIDTH - 1), j });
            printf("|");
        }
    }
}

void printSideBoxesBorder(HANDLE* h, int8_t mode) {

    // Print top and bottom box border
    for (int i = 0; i < 2; i++) {
        SetConsoleCursorPosition(*h, (COORD) { BOARD_WIDTH + 3, (i * 7) + mode });

        for (int j = 0; j < 16; j++) {
            printf("-");
        }
    }

    // Print left and right box border
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            SetConsoleCursorPosition(*h, (COORD) { (i * 15) + (BOARD_WIDTH + 3), j + mode });
            printf("|");
        }
    }
}

void printControls(HANDLE* h) {
    SetConsoleCursorPosition(*h, (COORD) { 0, BOARD_HEIGHT + CONTROLS_OFFSET });
    printf("A - Left\n");
    printf("D - Right\n");
    printf("S - Down\n");
    printf("space - Hard Down\n");
    printf("E - Rotate Clockwise\n");
    printf("Q - Rotate Counterclockwise\n");
    printf("R - Switch Piece\n");
}

void initializeScoreDisplay(HANDLE* h) {
    SetConsoleCursorPosition(*h, (COORD) { BOARD_WIDTH + 4, BOARD_HEIGHT - 2 });
    printf("Score: ");
}

// REFRESH DISPLAY

// Converts cell position coordinates to console type, COORD.
COORD getDisplayPositionOfCell(int8_t xCellPos, int8_t yCellPos) {
    return (COORD) { (xCellPos * 2) + 1, yCellPos + 1 };
}

// Reads entire current board (2D array) and prints colors based on value.
void refreshBoardDisplay(HANDLE* h, int8_t boardLayer[CELLROWS][CELLCOLS], WORD baseConsoleAttributes) {

    for (int row = 0; row < CELLROWS; row++) {
        for (int col = 0; col < CELLCOLS; col++) {
            int8_t currentCell = boardLayer[row][col];

            // If cell is filled
            if (currentCell == -1) {
                WriteConsoleOutputCharacterW(*h, FILLED_CELL, 2, getDisplayPositionOfCell(col, row), &_written);
            }
            else if (currentCell > 0) {
                WriteConsoleOutputCharacterW(*h, FILLED_CELL, 2, getDisplayPositionOfCell(col, row), &_written);

                // Set color based on value of cell
                WriteConsoleOutputAttribute(*h, &_colors[currentCell - 1], 2, getDisplayPositionOfCell(col, row), &_written);
            }
            else if (currentCell == 0) {
                WriteConsoleOutputCharacterW(*h, L"  ", 2, getDisplayPositionOfCell(col, row), &_written);

                // Clear cell
                FillConsoleOutputAttribute(*h, baseConsoleAttributes, 2, getDisplayPositionOfCell(col, row), &_written);
            }
            else {
                printf("EROR");
            }
        }
    }
}

// Writes to bool array[4] where tetromino col with at least one cell is true
void getShapeFootprint(const Shape* shape, bool output[4]) {

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            if (_tetrominoes[shape->shape][shape->rotation][row][col]) {
                output[col] = true;
                break;
            }
        }
    }
}

// Gets footprint of shape; uses to refresh bottom border (shadow indicator)
void updateShapeShadow(Shape* shape, HANDLE* h) {

    bool shadow[4] = { false };
    getShapeFootprint(shape, shadow);

    int8_t offset = shape->x;

    for (int x = 0; x < CELLCOLS; x++) {
        COORD displayPosition = { BORDER_WIDTH + (x * 2), BOARD_HEIGHT - 1 };

        if (x >= offset && x < offset + 4) {
            if (shadow[x - offset]) {
                WriteConsoleOutputCharacterW(*h, L"##", 2, displayPosition, &_written);
                continue;
            }
        }
        WriteConsoleOutputCharacterW(*h, L"--", 2, displayPosition, &_written);
    }
}

// Prints shape displayed in specified side box
void changeSideBoxDisplayShape(int8_t shape, int8_t mode, HANDLE* h, WORD baseConsoleAttributes) {

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {

            int8_t currentCell = _tetrominoes[shape][0][row][col];
            COORD displayPosition = getDisplayPositionOfCell(col, row);

            displayPosition.X += SHAPE_PREVIEW_OFFSET_X;
            displayPosition.Y += SHAPE_PREVIEW_OFFSET_Y + mode;

            if (currentCell) {
                WriteConsoleOutputCharacterW(*h, FILLED_CELL, 2, displayPosition, &_written);
                WriteConsoleOutputAttribute(*h, &_colors[shape], 2, displayPosition, &_written);
            }
            else {
                WriteConsoleOutputCharacterW(*h, L"  ", 2, displayPosition, &_written);
                FillConsoleOutputAttribute(*h, baseConsoleAttributes, 2, displayPosition, &_written);
            }
        }
    }
}

void updateScoreDisplay(int score, HANDLE* h) {
    SetConsoleCursorPosition(*h, (COORD) { BOARD_WIDTH + 11, BOARD_HEIGHT - 2 });
    printf("%07d", score);
}