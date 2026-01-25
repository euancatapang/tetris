#pragma once

#define SCORE_FILE "score.txt"

void writeHighScore(const char* fileName, uint32_t score);
uint32_t readHighScore(const char* fileName);