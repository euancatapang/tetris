#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#include <windows.h>

#include "tetris.h"

// FUNCTIONS

void writeShapeToBoard(const Shape* shape, const int8_t mode, int8_t boardLayer[CELLROWS][CELLCOLS]) {

    for (int row = shape->y; row < shape->y + 4; row++) {
        for (int col = shape->x; col < shape->x + 4; col++) {

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
void setupVirtualBoardWithBorders(int8_t destination[CELLROWS + 2][CELLCOLS + 2], const int8_t source[CELLROWS][CELLCOLS]) {

    for (int row = 0; row < CELLROWS + 2; row++) {
        for (int col = 0; col < CELLCOLS + 2; col++) {

            if (row == 0 ||
                col == 0 ||
                row == CELLROWS + 1 ||
                col == CELLCOLS + 1
                ) destination[row][col] = 1;

            else destination[row][col] = source[row - 1][col - 1];
        }
    }
}

int8_t simulateShapePlacement(const Shape* shape, int8_t mode, int8_t virtualPlaceX, int8_t virtualPlaceY) {

    // Constrain starts and ends to between 0 and CELLCOLS + 2
    int8_t startX = (virtualPlaceX < 0) ? 0 : virtualPlaceX;
    int8_t endX = (startX + 4 > CELLCOLS + 2) ? CELLCOLS + 2 : startX + 4;

    int8_t startY = (virtualPlaceY < 0) ? 0 : virtualPlaceY;
    int8_t endY = (startY + 4 > CELLROWS + 2) ? CELLROWS + 2 : startY + 4;

    int8_t needsKick = 0;
    
    for (int virtualRow = startY; virtualRow < endY; virtualRow++) {
        for (int virtualCol = startX; virtualCol < endX; virtualCol++) {

            int8_t currentCellSum = shape->borderedBoardWithoutShape[virtualRow][virtualCol] + 1;

            int8_t tetrominoCellX = virtualCol - virtualPlaceX;
            int8_t tetrominoCellY = virtualRow - virtualPlaceY;
            int8_t currentCellIsFilled = _tetrominoes[shape->shape][shape->rotation][tetrominoCellY][tetrominoCellX];

            if (currentCellIsFilled && currentCellSum > 1) {

                if (mode == ROTATE && tetrominoCellX >= 2) {
                    needsKick = NEEDS_KICK_FROM_RIGHT;
                }
                else if (mode == ROTATE && tetrominoCellX <= 1) {
                    needsKick = NEEDS_KICK_FROM_LEFT;
                }
                else {
                    return 0;
                }
            }
        }
    }

    // Only return kick values after cycling through to ensure that no cell is invalid
    if (needsKick == NEEDS_KICK_FROM_LEFT || needsKick == NEEDS_KICK_FROM_RIGHT) {
        return needsKick;
    }

    return 1;
}

// Test layer is a board without the piece we're trying to move.
int8_t testIsPlaceValid(const Shape* shape, int8_t mode) {

    // To compensate for offset caused by border
    int8_t virtualPlaceX = shape->x + 1;
    int8_t virtualPlaceY = shape->y + 1;

    return simulateShapePlacement(shape, mode, virtualPlaceX, virtualPlaceY);
}

// Returns greatest Y value (lowest place on the board) the shape can be placed; Returns -1 if no valid placement
int8_t getLowestValidPlace(const Shape* shape) {

    // To compensate for offset caused by border
    int8_t testPlaceX = shape->x + 1;
    int8_t testPlaceY = shape->y + 1;

    int8_t previousY = -1;

    for (; testPlaceY <= CELLROWS; testPlaceY++) {
        
        if (simulateShapePlacement(shape, PLACE_SHAPE, testPlaceX, testPlaceY) == 0) {
            return previousY;
        }
        previousY = testPlaceY - 1;
    }
}

void fallShape(const Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS]) {

    int8_t lowestPossiblePlaceY = getLowestValidPlace(shape);

    if (lowestPossiblePlaceY != -1) {

        Shape fellShape = *shape;
        fellShape.y = lowestPossiblePlaceY;

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&fellShape, PLACE_SHAPE, boardLayer);
    }
}

bool placeShape(Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS]) {
    bool placeIsValid = testIsPlaceValid(shape, MOVE);

    if (placeIsValid) {
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);
    }

    return placeIsValid;
}

// Returns if move succeeded
bool moveShape(int8_t direction, Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS]) {
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

    int8_t isValidMove = testIsPlaceValid(&movedShape, MOVE);

    if (isValidMove) {

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&movedShape, PLACE_SHAPE, boardLayer);

        shape->x += xOffset;
        shape->y += yOffset;
    }

    return isValidMove;
}

bool rotateShape(int8_t rotateTowards, Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS]) {
    int8_t resultingRotation;
    
    if (rotateTowards == CLOCKWISE)
        resultingRotation = (shape->rotation + 1 + 4) % 4;
    else
        resultingRotation = (shape->rotation - 1 + 4) % 4;

    Shape rotatedShape = *shape;
    rotatedShape.rotation = resultingRotation;

    int8_t isValidMove = testIsPlaceValid(&rotatedShape, ROTATE);

    if (isValidMove == 0) {
        return false;
    }

    else if (isValidMove == 1) {

        writeShapeToBoard(shape, CLEAR, boardLayer);
        writeShapeToBoard(&rotatedShape, PLACE_SHAPE, boardLayer);

        shape->rotation = resultingRotation;

        return true;
    }

    else {

        // To handle I shape edge case
        if (shape->shape == _I && shape->x == -2 && isValidMove == NEEDS_KICK_FROM_LEFT)       
            rotatedShape.x += 2; 

        else if (isValidMove == NEEDS_KICK_FROM_RIGHT)  
            rotatedShape.x -= 1;

        else if (isValidMove == NEEDS_KICK_FROM_LEFT)   
            rotatedShape.x += 1;

        if (testIsPlaceValid(&rotatedShape, PLACE_SHAPE)) {
            
            // Clear previous shape from board
            writeShapeToBoard(shape, CLEAR, boardLayer);
            writeShapeToBoard(&rotatedShape, PLACE_SHAPE, boardLayer);

            shape->x = rotatedShape.x;
            shape->rotation = resultingRotation;
        }
        
        return true;
    }
}

bool checkMoveShapeDown(time_t* timeOfLastMoveDown, Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS], HANDLE* h, WORD baseConsoleAttributes) {
    time_t currentTime = time(NULL);
    int elapsedTime = (int)difftime(currentTime, *timeOfLastMoveDown);
    static int8_t moveDownFails = 0;

    if (elapsedTime) {
        bool succeeded = moveShape(DOWN, shape, boardLayer);
        *timeOfLastMoveDown = currentTime;

        if (!succeeded) {
            moveDownFails += 1;
        }
        else {
            refreshBoardDisplay(h, boardLayer, baseConsoleAttributes);
        }

        if (moveDownFails >= 1) {
            moveDownFails = 0;
            return true;
        }
    }

    return false;
}

bool rowIsFilled(int8_t row[CELLCOLS]) {
    for (int i = 0; i < CELLCOLS; i++) {
        if (row[i] == 0) {
            return false;
        }
    }

    return true;
}

// Clears rows filled upon committing a shape; Returns how many rows cleared
int8_t getClearableRows(int8_t boardLayer[CELLROWS][CELLCOLS]) {
    int8_t rowsCleared = 0;

    for (int row = 0; row < CELLROWS; row++) {

        if (rowIsFilled(boardLayer[row])) {
            for (int rowToFill = row; rowToFill > 0; rowToFill--) {

                // Fill rowToFill with the contents of the row above it (rowToFill - 1)
                for (int col = 0; col < CELLCOLS; col++) {
                    boardLayer[rowToFill][col] = boardLayer[rowToFill - 1][col];
                }
            }
            rowsCleared++;
        }
    }

    return rowsCleared;
}

// Calculates score gained based on rows cleared
int16_t calculateScore(int8_t rowsCleared) {
    int16_t scores[5] = { 0, 40, 100, 300, 1200 };

    return scores[rowsCleared];
}

void ESCAPEFROMBOARD() {
    for (int i = 0; i < BOARD_HEIGHT + 1; i++) {
        printf("\n");
    }
}

int main() {

    // Console display setup
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned int)time(NULL));

    // Get base console attributes
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD baseConsoleAttributes;
    GetConsoleScreenBufferInfo(h, &info);
    baseConsoleAttributes = info.wAttributes;
    
    // Set console size
    SMALL_RECT consoleArea = { 0, 0, BOARD_WIDTH * 2, BOARD_HEIGHT + CONTROLS_OFFSET + 7 };
    SetConsoleWindowInfo(h, true, &consoleArea);

    // Initialization
    initializeDisplay(); // Stretch printable area out to fit board
    printBorder(&h);
    printSideBoxesBorder(&h, CACHE_BOX);
    printSideBoxesBorder(&h, NEXT_PIECE);
    printControls(&h);
    initializeScoreDisplay(&h);

    // Starting shape
    int8_t boardLayer[CELLROWS][CELLCOLS] = { 0 }; // [ROW][COL] = 20, 10
    Shape currentShape;

    // Input and time variables
    char input = 0;

    int previousScore = -1;
    int score = 0;

    time_t timeOfLastMoveDown = time(NULL);
    bool failedMoveDown = true;

    int8_t nextShape = rand() % 7;

    int8_t cachedPiece = -1; // Indicating no piece has been cached
    int8_t cachePieceAllowed = false;

    while (1) {

        Sleep(10);

        if (failedMoveDown) {
            
            score += calculateScore(getClearableRows(boardLayer));

            if (score > previousScore) {
                updateScoreDisplay(score, &h);
                previousScore = score;
            }

            // Commits previous shape and creates a new one
            currentShape.x = 3;
            currentShape.y = 0;
            currentShape.shape = nextShape;
            currentShape.rotation = 0;
            setupVirtualBoardWithBorders(currentShape.borderedBoardWithoutShape, boardLayer);

            nextShape = rand() % 7;

            // Check if shape can be placed. If not, you've lost
            if (!testIsPlaceValid(&currentShape, MOVE)) {
                break;
            }

            placeShape(&currentShape, boardLayer);
            updateShapeShadow(&currentShape, &h);
            refreshBoardDisplay(&h, boardLayer, baseConsoleAttributes);
            changeSideBoxDisplayShape(nextShape, NEXT_PIECE, &h, baseConsoleAttributes);

            cachePieceAllowed = true;
        }
        failedMoveDown = checkMoveShapeDown(&timeOfLastMoveDown, &currentShape, boardLayer, &h, baseConsoleAttributes);

        if (_kbhit()) {

            input = _getch();
            int8_t action = 0;
            int8_t actionType = 0;

            switch (input) {
                case 'a': action = LEFT; actionType = MOVE; break;
                case 'd': action = RIGHT; actionType = MOVE; break;
                case 's': action = DOWN; actionType = MOVE; break;
                case ' ': action = HARD_DOWN; actionType = MOVE; break;

                case 'e': action = CLOCKWISE; actionType = ROTATE; break;
                case 'q': action = A_CLOCKWISE; actionType = ROTATE; break;

                case 'r': action = CACHE_PIECE; actionType = SPECIAL; break;

                case 'p': exit(0); break;
                default: continue;
            }

            // If made past this point, it means input has valid action.

            if (actionType == MOVE) {
                if (action == HARD_DOWN) {
                    fallShape(&currentShape, boardLayer);
                    failedMoveDown = true;
                }
                else {
                    moveShape(action, &currentShape, boardLayer);
                }        
            }

            else if (actionType == ROTATE) {
                rotateShape(action, &currentShape, boardLayer);
            }

            else if (action == CACHE_PIECE && cachePieceAllowed == true) {
                writeShapeToBoard(&currentShape, CLEAR, boardLayer);

                // If cache slot is empty
                if (cachedPiece == -1) {
                    cachedPiece = currentShape.shape;
                    currentShape.shape = nextShape;
                    nextShape = rand() % 7;
                }
                else {
                    int8_t temp = currentShape.shape;
                    currentShape.shape = cachedPiece;
                    cachedPiece = temp;
                }

                currentShape.x = 3;
                currentShape.y = 0;
                currentShape.rotation = 0;

                placeShape(&currentShape, boardLayer);
                changeSideBoxDisplayShape(cachedPiece, CACHE_BOX, &h, baseConsoleAttributes);
                changeSideBoxDisplayShape(nextShape, NEXT_PIECE, &h, baseConsoleAttributes);

                cachePieceAllowed = false;
            }

            // Refresh board display because reaching this point means a valid action was done.
            refreshBoardDisplay(&h, boardLayer, baseConsoleAttributes);
            if (action != DOWN) {
                updateShapeShadow(&currentShape, &h);
            }
        }
    }
    
    ESCAPEFROMBOARD();
    return 0;
}