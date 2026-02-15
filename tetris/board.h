#pragma once

#include "tetris.h"

void writeShapeToBoard(const Shape* shape, int8_t mode, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);

PlacementValidity simulateShapePlacement(const Shape* shape, int8_t testPlaceX, int8_t testPlaceY);
int8_t getLowestValidPlace(const Shape* shape);

void fallShape(const Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);
bool placeShape(const Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);
bool moveShape(ActionInput direction, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);
bool rotateShape(ActionInput rotateTowards, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);

MoveDownStatus checkMoveDownFailure(uint32_t* timeOfLastMoveDown, Shape* shape, int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);

bool rowIsFilled(int8_t row[TETROMINO_CELL_COLS]);
uint8_t getRowsCleared(int8_t boardLayer[TETROMINO_CELL_ROWS][TETROMINO_CELL_COLS]);