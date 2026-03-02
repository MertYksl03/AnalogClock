#ifndef MAIN_H
#define MAIN_H

#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>
#endif

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdlib.h>

#include "header.h"
#include "colors.h"


//FUNCTION PROTOTYPES
void process_input(SDL_Event event);
int initialize_window(u8 is_dark_theme, int fontSize);
void update();
void render();
void QUIT();

void drawNumbers(Circle clockCircle);
void print_current_time();
void drawMarkers(MarkerPosition* MarkerPositions);
void calculateMarkerPositions(MarkerPosition* MarkerPositions, Circle clockCircle);
void drawClockHands(Circle clockCircle);
void calculateHourHandPosition(float* x_end, float* y_end, Circle clockCircle);
void calculateMinuteHandPosition(float* x_end, float* y_end, Circle clockCircle);
void calculateSecondHandPosition(float* x_end, float* y_end, Circle clockCircle);
void DrawThickLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, float thickness, Color color);

#endif
