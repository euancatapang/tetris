#include <stdint.h>
#include <stdbool.h>
#include <windows.h>

#include "tetris.h"

void writeShapeToBoard(const Shape* shape, int8_t mode, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    for (int8_t row = shape->y; row < shape->y + 4; row++) {
        for (int8_t col = shape->x; col < shape->x + 4; col++) {
            
            int8_t cellIsFilled = _tetrominoes[shape->shape][shape->rotation][row - shape->y][col - shape->x];
            if (cellIsFilled && mode == PLACE_SHAPE)
                boardLayer[row][col] += (shape->shape + 1); // Add shape index offset by 1 because 0 (aka I piece) conflicts with empty
            else if (cellIsFilled && mode == CLEAR)
                boardLayer[row][col] = 0;
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

PlacementValidity simulateShapePlacement(const Shape* shape, int8_t testPlaceX, int8_t testPlaceY) {
    // Constrain starts and ends to between 0 and TETROMINO_CELL_COLS + 2
    int8_t startX = (testPlaceX < 0) ? 0 : testPlaceX;
    int8_t endX = (startX + 4 > TETROMINO_CELL_COLS + 2) ? TETROMINO_CELL_COLS + 2 : startX + 4;

    int8_t startY = (testPlaceY < 0) ? 0 : testPlaceY;
    int8_t endY = (startY + 4 > TETROMINO_CELL_ROWS + 2) ? TETROMINO_CELL_ROWS + 2 : startY + 4;

    PlacementValidity validity = PLACE_VALID; // Assume placement is valid; can be invalidated during board check

    for (int8_t virtualRow = startY; virtualRow < endY; virtualRow++) {
        for (int8_t virtualCol = startX; virtualCol < endX; virtualCol++) {

            int8_t currentCellSum = shape->borderedBoardWithoutShape[virtualRow][virtualCol] + 1;

            int8_t tetrominoCellX = virtualCol - testPlaceX;
            int8_t tetrominoCellY = virtualRow - testPlaceY;
            int8_t currentCellIsFilled = _tetrominoes[shape->shape][shape->rotation][tetrominoCellY][tetrominoCellX];

            if (currentCellIsFilled && currentCellSum > 1)
                return PLACE_INVALID;
        }
    }
    return validity;
}

// Test layer is a board without the piece we're trying to move.
PlacementValidity testPlaceValidity(const Shape* shape) {
    // To compensate for offset caused by border
    int8_t testPlaceX = shape->x + 1;
    int8_t testPlaceY = shape->y + 1;

    return simulateShapePlacement(shape, testPlaceX, testPlaceY);
}

// Returns greatest Y value (lowest place on the board) the shape can be placed; Returns -1 if no valid placement
int8_t getLowestValidPlace(const Shape* shape) {
    // To compensate for offset caused by border
    int8_t testPlaceX = shape->x + 1;
    int8_t testPlaceY = shape->y + 1;

    int8_t previousY = -1;

    for (; testPlaceY <= TETROMINO_CELL_ROWS; testPlaceY++) {

        if (simulateShapePlacement(shape, testPlaceX, testPlaceY) == PLACE_INVALID) {
            return previousY;
        }
        previousY = testPlaceY - 1;
    }

    return -1;
}

void fallShape(Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    int8_t lowestPossiblePlaceY = getLowestValidPlace(shape);

    if (lowestPossiblePlaceY != -1) {
        writeShapeToBoard(shape, CLEAR, boardLayer);

        shape->y = lowestPossiblePlaceY;
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);
    }
}

// Returns true if place succeeded
bool placeShape(const Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    if (testPlaceValidity(shape) == PLACE_VALID) {
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);
        return true;
    }
    return false;
}

// Returns true if move succeeded
bool moveShape(ActionInput direction, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    int8_t xOffset = 0;
    int8_t yOffset = 0;

    switch (direction) {
        case INPUT_LEFT: xOffset = -1; break;
        case INPUT_RIGHT: xOffset = 1; break;
        case INPUT_DOWN: yOffset = 1; break;
    }

    // Move shape to coords to check
    shape->x += xOffset;
    shape->y += yOffset;

    PlacementValidity validity = testPlaceValidity(shape);

    if (validity == PLACE_VALID) {
        // Move shape to original place then clear it
        shape->x -= xOffset;
        shape->y -= yOffset;
        writeShapeToBoard(shape, CLEAR, boardLayer);

        // Move shape to new place then write it
        shape->x += xOffset;
        shape->y += yOffset;
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);

        return true;
    }

    // If move is invalid, move shape to original place
    shape->x -= xOffset;
    shape->y -= yOffset;
    return false;
}

// Returns true if rotate succeeded
bool rotateShape(ActionInput rotateTowards, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    int8_t newRotation;

    if (rotateTowards == INPUT_CLOCKWISE)
        newRotation = (shape->rotation + 1 + 4) % 4;
    else
        newRotation = (shape->rotation - 1 + 4) % 4;

    int8_t originalRotation = shape->rotation;
    int8_t originalX = shape->x;
    int8_t originalY = shape->y;

    shape->rotation = newRotation;

    uint8_t kickTestRowIndex;

    // Determine kick tests to use based on source to dest rotation
    switch (KICK_TEST(originalRotation, newRotation)) {
        case KICK_TEST(0, 1): kickTestRowIndex = 0; break;
        case KICK_TEST(1, 0): kickTestRowIndex = 1; break;
        case KICK_TEST(1, 2): kickTestRowIndex = 2; break;
        case KICK_TEST(2, 1): kickTestRowIndex = 3; break;
        case KICK_TEST(2, 3): kickTestRowIndex = 4; break;
        case KICK_TEST(3, 2): kickTestRowIndex = 5; break;
        case KICK_TEST(3, 0): kickTestRowIndex = 6; break;
        case KICK_TEST(0, 3): kickTestRowIndex = 7; break;
    }

    PlacementValidity validity = PLACE_INVALID;

    for (uint8_t i = 0; i < KICK_TEST_COUNT && validity == PLACE_INVALID; i++) {
        Coord currentOffset;
        
        if (shape->shape == _I)
            currentOffset = _kickTable[1][kickTestRowIndex][i];
        else
            currentOffset = _kickTable[0][kickTestRowIndex][i];

        shape->x = originalX + currentOffset.x;
        shape->y = originalY + currentOffset.y;

        validity = testPlaceValidity(shape);
    }

    if (validity == PLACE_VALID) {
        int8_t newX = shape->x;
        int8_t newY = shape->y;

        shape->rotation = originalRotation;
        shape->x = originalX;
        shape->y = originalY;
        writeShapeToBoard(shape, CLEAR, boardLayer);

        shape->rotation = newRotation;
        shape->x = newX;
        shape->y = newY;
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);

        return true;
    }

    //else if (validity == PLACE_KICK_FROM_LEFT || validity == PLACE_KICK_FROM_RIGHT) {
    //    // To handle I shape edge case
    //    if (shape->shape == _I && shape->x == -2 && validity == PLACE_KICK_FROM_LEFT)
    //        shape->x += 2;
    //    else if (shape->shape == _I && shape->x == TETROMINO_CELL_COLS - 2 && validity == PLACE_KICK_FROM_RIGHT)
    //        shape->x -= 2;
    //    else if (validity == PLACE_KICK_FROM_RIGHT)
    //        shape->x -= 1;
    //    else if (validity == PLACE_KICK_FROM_LEFT)
    //        shape->x += 1;

    //    if (testPlaceValidity(shape, ACTION_TYPE_MOVE)) {
    //        int8_t newX = shape->x;
    //        
    //        shape->rotation = originalRotation;
    //        shape->x = originalX;
    //        writeShapeToBoard(shape, CLEAR, boardLayer);

    //        shape->rotation = newRotation;
    //        shape->x = newX;
    //        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);

    //        return true;
    //    }
    //}

    // If rotation is invalid
    shape->rotation = originalRotation;
    shape->x = originalX;
    shape->y = originalY;
    return false;
}

MoveDownStatus checkMoveDownFailure(uint32_t* timeOfLastMoveDown, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
    uint32_t currentTime = GetTickCount();
    uint32_t elapsedTime = currentTime - *timeOfLastMoveDown;

    if (elapsedTime >= MOVE_DOWN_INTERVAL_MS) {
        *timeOfLastMoveDown = currentTime;

        if (!moveShape(INPUT_DOWN, shape, boardLayer))
            return MOVE_DOWN_FAIL;
        else
            return MOVE_DOWN_SUCCESS;
    }
    return MOVE_DOWN_NO_CHANGE;
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
uint8_t getRowsCleared(int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]) {
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