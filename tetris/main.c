#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#include <windows.h>

// BOARD CONSTANTS
#define CELLROWS 20 // DEFAULT 20
#define CELLCOLS 10 // DEFAULT 10

#define ROWS (CELLROWS*1) // DEFAULT (CELLROWS*1)
#define COLS (CELLCOLS*2) // DEFAULT (CELLROWS*2)

#define BOARDHEIGHT (ROWS+2)
#define BOARDWIDTH (COLS+2)

#define CELL "\u2588\u2588"

// BORDERS
#define BORDER_WIDTH 1

#define NEXT_PIECE 0 
#define CACHE_BOX 11 // Y-axis offset

// SHAPE PREVIEW OFFSET
#define SHAPE_PREVIEW_OFFSET_X (BOARDWIDTH+7)
#define SHAPE_PREVIEW_OFFSET_Y 2

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

typedef struct {
    int8_t x;
    int8_t y;
    int8_t shape;
    int8_t rotation;
    int8_t borderedBoardWithoutShape[CELLROWS + 2][CELLCOLS + 2];

} Shape;

WCHAR _cell[] = L"██"; // DEFAULT L"██"
DWORD _written;

#define _I 0
#define _O 1
#define _T 2
#define _S 3
#define _Z 4
#define _J 5
#define _L 6

// Tetrominoes declared in separate file
extern const int8_t _tetrominoes[7][4][4][4];
extern const WORD _colors[7][2];

// FUNCTIONS
void initializeDisplay() {

    // Expand display area to fit board
    for (int i = 0; i < BOARDHEIGHT; i++) {
        printf("\n");
    }
}

void printBorder(HANDLE* h) {

    // Print top border
    SetConsoleCursorPosition(*h, (COORD) { 0, 0 });
    for (int j = 0; j < BOARDWIDTH; j++) {
        printf("-");
    }

    // Print left and right border
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < BOARDHEIGHT; j++) {
            SetConsoleCursorPosition(*h, (COORD) { i * (BOARDWIDTH - 1), j });
            printf("|");
        }
    }
}

void printSideBoxesBorder(HANDLE* h, int8_t mode) {

    // Print top and bottom box border
    for (int i = 0; i < 2; i++) {
        SetConsoleCursorPosition(*h, (COORD) { BOARDWIDTH + 3, (i * 7) + mode });

        for (int j = 0; j < 16; j++) {
            printf("-");
        }
    }

    // Print left and right box border
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            SetConsoleCursorPosition(*h, (COORD) { (i * 15) + (BOARDWIDTH + 3), j + mode });
            printf("|");
        }
    }
}

COORD getDisplayPositionOfCell(int8_t xCellPos, int8_t yCellPos) {
    return (COORD) { (xCellPos * 2) + 1, yCellPos + 1 };
}

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

void refreshBoardDisplay(HANDLE* h, int8_t boardLayer[CELLROWS][CELLCOLS], WORD baseConsoleAttributes) {

    for (int row = 0; row < CELLROWS; row++) {
        for (int col = 0; col < CELLCOLS; col++) {
            int8_t currentCell = boardLayer[row][col];

            // If cell is filled
            if (currentCell == -1) {
                WriteConsoleOutputCharacterW(*h, _cell, 2, getDisplayPositionOfCell(col, row), &_written);
            }
            else if (currentCell > 0) {
                WriteConsoleOutputCharacterW(*h, _cell, 2, getDisplayPositionOfCell(col, row), &_written);

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

void updateShapeShadow(Shape* shape, HANDLE* h) {
    bool shadow[4] = { false };
    getShapeFootprint(shape, shadow);

    int8_t offset = shape->x;

    for (int x = 0; x < CELLCOLS; x++) {
        COORD displayPosition = { BORDER_WIDTH + (x * 2), BOARDHEIGHT - 1 };

        if (x >= offset && x < offset + 4) {
            if (shadow[x - offset]) {
                WriteConsoleOutputCharacterW(*h, L"##", 2, displayPosition, &_written);
                continue;
            }
        }
        WriteConsoleOutputCharacterW(*h, L"--", 2, displayPosition, &_written);
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

    int8_t needsKick = 0;
    
    for (int virtualRow = 0; virtualRow < CELLROWS + 2; virtualRow++) {
        for (int virtualCol = 0; virtualCol < CELLCOLS + 2; virtualCol++) {

            if ((virtualCol >= virtualPlaceX && virtualCol < virtualPlaceX + 4) && 
                (virtualRow >= virtualPlaceY && virtualRow < virtualPlaceY + 4)) {

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
    }

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

bool placeShape(Shape* shape, int8_t boardLayer[CELLROWS][CELLCOLS]) {
    bool placeIsValid = testIsPlaceValid(shape, MOVE);

    if (placeIsValid) {
        writeShapeToBoard(shape, PLACE_SHAPE, boardLayer);
    }

    return placeIsValid;
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
        resultingRotation = shape->rotation + 1;
    else
        resultingRotation = shape->rotation - 1;

    if (resultingRotation == 4)         resultingRotation = 0;
    else if (resultingRotation == -1)   resultingRotation = 3;

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

void adjustForFilledRows(int8_t boardLayer[CELLROWS][CELLCOLS]) {
    
    for (int row = 0; row < CELLROWS; row++) {

        if (rowIsFilled(boardLayer[row])) {
            for (int rowToFill = row; rowToFill > 0; rowToFill--) {

                // Fill rowToFill with the contents of the row above it (rowToFill - 1)
                for (int col = 0; col < CELLCOLS; col++) {
                    boardLayer[rowToFill][col] = boardLayer[rowToFill - 1][col];
                }
            }
        }
    }
}

void changeSideBoxDisplayShape(int8_t shape, int8_t mode, HANDLE* h, WORD baseConsoleAttributes) {

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {

            int8_t currentCell = _tetrominoes[shape][0][row][col];
            COORD displayPosition = getDisplayPositionOfCell(col, row);

            displayPosition.X += SHAPE_PREVIEW_OFFSET_X;
            displayPosition.Y += SHAPE_PREVIEW_OFFSET_Y + mode;

            if (currentCell) {
                WriteConsoleOutputCharacterW(*h, _cell, 2, displayPosition, &_written);
                WriteConsoleOutputAttribute(*h, &_colors[shape], 2, displayPosition, &_written);
            }
            else {
                WriteConsoleOutputCharacterW(*h, L"  ", 2, displayPosition, &_written);
                FillConsoleOutputAttribute(*h, baseConsoleAttributes, 2, displayPosition, &_written);
            }
        }
    }
}

void ESCAPEFROMBOARD() {
    for (int i = 0; i < BOARDHEIGHT + 1; i++) {
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

    // Initialization
    initializeDisplay(); // Stretch printable area out to fit board
    printBorder(&h);
    printSideBoxesBorder(&h, CACHE_BOX);
    printSideBoxesBorder(&h, NEXT_PIECE);

    // Starting shape
    int8_t boardLayer[CELLROWS][CELLCOLS] = { 0 }; // [ROW][COL] = 20, 10
    Shape currentShape;

    // Input and time variables
    char input = 0;

    time_t timeOfLastMoveDown = time(NULL);
    bool failedMoveDown = true;

    int8_t nextShape = rand() % 7;
    int8_t cachedPiece = -1; // Indicating no piece has been cached
    int8_t cachePieceAllowed = -1;

    while (1) {
        if (failedMoveDown) {
            
            adjustForFilledRows(boardLayer);

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
            }

            if (actionType == MOVE) {
                if (action == HARD_DOWN) {
                    fallShape(&currentShape, boardLayer);
                    failedMoveDown = true;
                }
                else {
                    moveShape(action, &currentShape, boardLayer);
                }

                refreshBoardDisplay(&h, boardLayer, baseConsoleAttributes);

                if (action == LEFT || action == RIGHT)
                    updateShapeShadow(&currentShape, &h);
            }

            else if (actionType == ROTATE) {
                rotateShape(action, &currentShape, boardLayer);
                refreshBoardDisplay(&h, boardLayer, baseConsoleAttributes);
                updateShapeShadow(&currentShape, &h);
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
                refreshBoardDisplay(&h, boardLayer, baseConsoleAttributes);
                updateShapeShadow(&currentShape, &h);
                changeSideBoxDisplayShape(cachedPiece, CACHE_BOX, &h, baseConsoleAttributes);
                changeSideBoxDisplayShape(nextShape, NEXT_PIECE, &h, baseConsoleAttributes);

                cachePieceAllowed = false;

            }
        }

        Sleep(10);
    }
    
    ESCAPEFROMBOARD();
    return 0;
}