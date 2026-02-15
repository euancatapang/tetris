#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

#include "tetris.h"
#include "board.h"
#include "score.h"
#include "display.h"

// Calculates score gained based on rows cleared
int16_t calculateScore(uint8_t rowsCleared) {
    int16_t scores[5] = { 0, 40, 100, 300, 1200 };
    return scores[rowsCleared];
}

void populateBag(Bag* bag) {
    // Create unshuffled bag
    for (uint8_t i = 0; i < BAG_SIZE; i++) {
        bag->shapes[i] = i % 7;
    }

    // Shuffle bag
    for (uint8_t i = 0; i < BAG_SIZE - 1; i++) {
        uint8_t swapWithPlace = i + rand() % (BAG_SIZE - i);

        uint8_t temp = bag->shapes[i];
        bag->shapes[i] = bag->shapes[swapWithPlace];
        bag->shapes[swapWithPlace] = temp;
    }
}

int8_t getShapeFromBag(Bag* bag) {
    int8_t newShape = bag->shapes[bag->index];
    bag->index += 1;

    if (bag->index == BAG_SIZE) {
        populateBag(bag);
        bag->index = 0;
    }
    return newShape;
}

bool translateInput(char input, Action* action) {
    switch (input) {
        case 'a': *action = (Action){ INPUT_LEFT, ACTION_TYPE_MOVE }; break;
        case 'd': *action = (Action){ INPUT_RIGHT, ACTION_TYPE_MOVE }; break;
        case 's': *action = (Action){ INPUT_DOWN, ACTION_TYPE_MOVE }; break;
        case ' ': *action = (Action){ INPUT_HARD_DOWN, ACTION_TYPE_MOVE }; break;
        case 'e': *action = (Action){ INPUT_CLOCKWISE, ACTION_TYPE_ROTATE }; break;
        case 'q': *action = (Action){ INPUT_A_CLOCKWISE, ACTION_TYPE_ROTATE }; break;
        case 'r': *action = (Action){ INPUT_CACHE_PIECE, ACTION_TYPE_SPECIAL }; break;
        case 'p': exit(0); break;
        default: return false;
    }
    return true;
}

void createShape(Shape* dest, int8_t x, int8_t y, int8_t shape, int8_t rotation) {
    dest->x = x;
    dest->y = y;
    dest->shape = shape;
    dest->rotation = rotation;
}

void updateScores(Terminal* terminal, Scores* scores, uint8_t rowsCleared) {
    scores->score += calculateScore(rowsCleared);

    if (scores->score >= scores->previousScore) {
        updateScoreDisplay(terminal, scores->score);
        scores->previousScore = scores->score;

        if (scores->score > scores->highScore) {
            updateHighScoreDisplay(terminal, scores->score);
        }
    }
}

int main() {
    // Initialization
    Terminal terminal;
    initializeDisplay(&terminal); // windows.h terminal setup
    setupGameBoard(&terminal);

    Scores scores = { 0, 0, readHighScore(SCORE_FILE) };
    initializeScoreDisplay(&terminal, scores.highScore);

    // Starting shape
    int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS] = { 0 }; // [ROW][COL] = 20, 10
    Shape currentShape = { 0 };

    // Bag randomization
    srand((uint32_t)GetTickCount());

    Bag bag = { 0, 0 };
    populateBag(&bag);
    int8_t nextShape = getShapeFromBag(&bag);

    // Input and time variables 
    uint32_t timeOfLastMoveDown = GetTickCount();
    MoveDownStatus moveDownStatus = MOVE_DOWN_FAIL;

    int8_t cachedPiece = -1; // Indicating no piece has been cached
    bool cachePieceAllowed = false;

    while (1) {
        Sleep(10);

        // If shape is committed to place point
        if (moveDownStatus == MOVE_DOWN_FAIL) {
            updateScores(&terminal, &scores, getRowsCleared(boardLayer));
            
            // Commits previous shape and creates a new one
            memcpy(currentShape.borderedBoardWithoutShape, boardLayer, TETROMINO_CELL_ROWS * TETROMINO_CELL_COLS * sizeof(int8_t));
            int8_t newShape = nextShape;
            nextShape = getShapeFromBag(&bag);

            // Check if shape can be placed. If not, you've lost.
            createShape(&currentShape, DEFAULT_X, DEFAULT_Y, newShape, DEFAULT_ROTATION);
            if (simulateShapePlacement(&currentShape, currentShape.x, currentShape.y) == PLACE_INVALID)
                break;

            // Update visuals
            placeShape(&currentShape, boardLayer);
            updateShapeShadow(&terminal, &currentShape);
            refreshBoardDisplay(&terminal, boardLayer);
            changeSideBoxDisplayShape(&terminal, nextShape, NEXT_PIECE);

            cachePieceAllowed = true;
        }
        else if (moveDownStatus == MOVE_DOWN_SUCCESS) {
            refreshBoardDisplay(&terminal, boardLayer);
        }
        moveDownStatus = checkMoveDownFailure(&timeOfLastMoveDown, &currentShape, boardLayer);

        if (_kbhit()) {
            char input = _getch();
            Action action;
            
            if (!translateInput(input, &action)) continue;

            // If made past this point, it means input has valid action.
            if (action.type == ACTION_TYPE_MOVE) {
                if (action.input == INPUT_HARD_DOWN) {
                    fallShape(&currentShape, boardLayer);
                    moveDownStatus = MOVE_DOWN_FAIL;
                }
                else {
                    moveShape(action.input, &currentShape, boardLayer);
                }        
            }

            else if (action.type == ACTION_TYPE_ROTATE) {
                rotateShape(action.input, &currentShape, boardLayer);
            }

            else if (action.input == INPUT_CACHE_PIECE && cachePieceAllowed == true) {
                writeShapeToBoard(&currentShape, CLEAR, boardLayer);

                int8_t newShape;
                // If cache slot is empty
                if (cachedPiece == -1) {
                    cachedPiece = currentShape.shape;
                    newShape = nextShape;
                    nextShape = getShapeFromBag(&bag);
                    
                    changeSideBoxDisplayShape(&terminal, nextShape, NEXT_PIECE);
                }
                else {
                    int8_t temp = currentShape.shape;
                    newShape = cachedPiece;
                    cachedPiece = temp;
                }

                createShape(&currentShape, DEFAULT_X, DEFAULT_Y, newShape, DEFAULT_ROTATION);

                placeShape(&currentShape, boardLayer);
                changeSideBoxDisplayShape(&terminal, cachedPiece, CACHE_BOX);

                cachePieceAllowed = false;
            }

            // Refresh board display because reaching this point means a valid action was done.
            refreshBoardDisplay(&terminal, boardLayer);
            if (action.input != INPUT_DOWN) {
                updateShapeShadow(&terminal, &currentShape);
            }
        }
    }
    
    bool gotNewHighScore = scores.score > scores.highScore;
    if (gotNewHighScore) 
        writeHighScore(SCORE_FILE, scores.score);

    showLoseScreen(&terminal, scores.score, gotNewHighScore);

    _getch();
    return 0;
}