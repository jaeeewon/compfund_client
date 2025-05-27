#pragma once

#include "util/common.h"

void moveCursorTo(int x, int y);

void clearCurrentLine();

void moveCursorHome();

void moveCursorUp(int lines);

void moveCursorDown(int lines);

void moveCursorToStart();

void moveCursorToBottom();

void clearConsole();

bool isCrazyConsole();