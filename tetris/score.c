#include <stdio.h>
#include <stdint.h>

#include "score.h"

// Functions

void writeHighScore(const char* fileName, uint32_t score) {
	FILE* scoreFile = fopen(fileName, "w");

	fprintf(scoreFile, "highScore=%d", score);
	fclose(scoreFile);
}

uint32_t readHighScore(const char* fileName) {
	uint32_t highScore;

	FILE* scoreFile = fopen(fileName, "r");

	if (!scoreFile) {
		writeHighScore(fileName, 0);
		scoreFile = fopen(fileName, "r");
	}

	int8_t status = fscanf(scoreFile, "highScore=%d", &highScore);
	fclose(scoreFile);

	if (status != 1)
		return -1;

	return highScore;
}