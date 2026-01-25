#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "tetris.h"

void writeShapeToBoard(const Shape* shape, const int8_t mode, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    for (int8_t row = shape->y; row < shape->y + 4; row++) {
        for (int8_t col = shape->x; col < shape->x + 4; col++) {

            int8_t cellIsFilled = _tetrominoes[shape->shape][shape->rotation][row - shape->y][col - shape->x];

            if (cellIsFilled) {

                if (mode == PLACE_SHAPE) {
                    boardLayer[row][col] += (shape->shape + 1); // Add shape index offset by 1 because 0 (aka I piece) conflicts with empty
                }

                else if (mode == CLEAR) {
                    boardLayer[row][col] = 0;
                }
            }
        }
    }
}

// Source does not contain current piece being controlled.
void setupVirtualBoardWithBorders(int8_t destination[TETROMINO_CELL_ROWS + 2][TETROMINO_CELL_COLS + 2], const int8_t source[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    for (uint8_t row = 0; row < TETROMINO_CELL_ROWS + 2; row++) {
        for (uint8_t col = 0; col < TETROMINO_CELL_COLS + 2; col++) {
            if (row == 0 ||
                col == 0 ||
                row == TETROMINO_CELL_ROWS + 1 ||
                col == TETROMINO_CELL_COLS + 1
                ) destination[row][col] = 1;

            else destination[row][col] = source[row - 1][col - 1];
        }
    }
}

int8_t simulateShapePlacement(const Shape* shape, int8_t mode, int8_t testPlaceX, int8_t testPlaceY) {

    // Constrain starts and ends to between 0 and TETROMINO_CELL_COLS + 2
    int8_t startX = (testPlaceX < 0) ? 0 : testPlaceX;
    int8_t endX = (startX + 4 > TETROMINO_CELL_COLS + 2) ? TETROMINO_CELL_COLS + 2 : startX + 4;

    int8_t startY = (testPlaceY < 0) ? 0 : testPlaceY;
    int8_t endY = (startY + 4 > TETROMINO_CELL_ROWS + 2) ? TETROMINO_CELL_ROWS + 2 : startY + 4;

    int8_t needsKick = 0;

    for (int8_t virtualRow = startY; virtualRow < endY; virtualRow++) {
        for (int8_t virtualCol = startX; virtualCol < endX; virtualCol++) {

            int8_t currentCellSum = shape->borderedBoardWithoutShape[virtualRow][virtualCol] + 1;

            int8_t tetrominoCellX = virtualCol - testPlaceX;
            int8_t tetrominoCellY = virtualRow - testPlaceY;
            int8_t currentCellIsFilled = _tetrominoes[shape->shape][shape->rotation][tetrominoCellY][tetrominoCellX];

            if (currentCellIsFilled && currentCellSum > 1) {

                if (mode == ROTATE && tetrominoCellX >= 2) {
                    needsKick = NEEDS_KICK_FROM_RIGHT;
                }
                else if (mode == ROTATE && tetrominoCellX <= 1) {
                    needsKick = NEEDS_KICK_FROM_LEFT;
                }
                else {
                    return 0; // Means placement is invalid
                }
            }
        }
    }

    // Only return kick values after cycling through to ensure that no cell is invalid
    if (needsKick == NEEDS_KICK_FROM_LEFT || needsKick == NEEDS_KICK_FROM_RIGHT) {
        return needsKick;
    }

    // Reaching this point means no collisions; placement is valid
    return 1;
}

// Test layer is a board without the piece we're trying to move.
int8_t testPlaceValidity(const Shape* shape, int8_t mode) {

    // To compensate for offset caused by border
    int8_t testPlaceX = shape->x + 1;
    int8_t testPlaceY = shape->y + 1;

    return simulateShapePlacement(shape, mode, testPlaceX, testPlaceY);
}

// Returns greatest Y value (lowest place on the board) the shape can be placed; Returns -1 if no valid placement
int8_t getLowestValidPlace(const Shape* shape) {

    // To compensate for offset caused by border
    int8_t testPlaceX = shape->x + 1;
    int8_t testPlaceY = shape->y + 1;

    int8_t previousY = -1;

    for (; testPlaceY <= TETROMINO_CELL_ROWS; testPlaceY++) {

        if (simulateShapePlacement(shape, PLACE_SHAPE, testPlaceX, testPlaceY) == 0) {
            return previousY;
        }
        previousY = testPlaceY - 1;
    }
}

void fallShape(const Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {

    int8_t lowestPossiblePlaceY = getLowestValidPlace(shape);

    if (lowestPossiblePlaceY != -1) {

        Shape fellShape = *shape;
        fellShape.y = lowestPossiblePlaceY;

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&fellShape, PLACE_SHAPE, boardLayer);
    }
}

bool placeShape(Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    bool placeIsValid = testPlaceValidity(shape, MOVE);

    if (placeIsValid) {
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);
    }

    return placeIsValid;
}

// Returns if move succeeded
bool moveShape(int8_t direction, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    int8_t xOffset = 0;
    int8_t yOffset = 0;

    switch (direction) {
        case LEFT: xOffset = -1; break;
        case RIGHT: xOffset = 1; break;
        case DOWN: yOffset = 1; break;
    }

    Shape movedShape = *shape;
    movedShape.x += xOffset;
    movedShape.y += yOffset;

    int8_t moveValidity = testPlaceValidity(&movedShape, MOVE);

    if (moveValidity == 1) {

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&movedShape, PLACE_SHAPE, boardLayer);

        shape->x += xOffset;
        shape->y += yOffset;
    }

    return moveValidity;
}

bool rotateShape(int8_t rotateTowards, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    int8_t resultingRotation;

    if (rotateTowards == CLOCKWISE)
        resultingRotation = (shape->rotation + 1 + 4) % 4;
    else
        resultingRotation = (shape->rotation - 1 + 4) % 4;

    Shape rotatedShape = *shape;
    rotatedShape.rotation = resultingRotation;

    int8_t moveValidity = testPlaceValidity(&rotatedShape, ROTATE);

    if (moveValidity == 0) {
        return false;
    }

    else if (moveValidity == 1) {

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&rotatedShape, PLACE_SHAPE, boardLayer);

        shape->rotation = resultingRotation;

        return true;
    }

    else {

        // To handle I shape edge case
        if (shape->shape == _I && shape->x == -2 && moveValidity == NEEDS_KICK_FROM_LEFT)
            rotatedShape.x += 2;

        else if (moveValidity == NEEDS_KICK_FROM_RIGHT)
            rotatedShape.x -= 1;

        else if (moveValidity == NEEDS_KICK_FROM_LEFT)
            rotatedShape.x += 1;

        if (testPlaceValidity(&rotatedShape, PLACE_SHAPE)) {

            // Clear previous shape from board
            writeShapeToBoard(shape, CLEAR, boardLayer);
            writeShapeToBoard(&rotatedShape, PLACE_SHAPE, boardLayer);

            shape->x = rotatedShape.x;
            shape->rotation = resultingRotation;
        }
        return true;
    }
}

int8_t checkMoveDownFailure(time_t* timeOfLastMoveDown, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    time_t currentTime = time(NULL);
    int elapsedTime = (int)difftime(currentTime, *timeOfLastMoveDown);

    if (elapsedTime) {
        bool movedDown = moveShape(DOWN, shape, boardLayer);
        *timeOfLastMoveDown = currentTime;

        if (!movedDown)
            return 1;
        else
            return 2;
    }

    return 0;
}

bool rowIsFilled(int8_t row[TETROMINO_CELL_COLS]) {
    for (uint8_t i = 0; i < TETROMINO_CELL_COLS; i++) {
        if (row[i] == 0) {
            return false;
        }
    }
    return true;
}

// Clears rows filled upon committing a shape; Returns how many rows cleared
int8_t getRowsCleared(int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    uint8_t rowsCleared = 0;

    for (uint8_t row = 0; row < TETROMINO_CELL_ROWS; row++) {

        if (rowIsFilled(boardLayer[row])) {
            for (uint8_t rowToFill = row; rowToFill > 0; rowToFill--) {
                // Fill rowToFill with the contents of the row above it (rowToFill - 1)
                for (uint8_t col = 0; col < TETROMINO_CELL_COLS; col++) {
                    boardLayer[rowToFill][col] = boardLayer[rowToFill - 1][col];
                }
            }
            rowsCleared++;
        }
    }
    return rowsCleared;
}