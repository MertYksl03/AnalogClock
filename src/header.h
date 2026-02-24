#include <SDL2/SDL.h>
#include <SDL2/SDL_shape.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "draw_circle.h"

#define PI 3.14159265359
#define DEG2RAD  (PI / 180.0f)
#define RAD2DEG  (180.0f / PI)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int32_t i32;

typedef Uint8 u8;
typedef Uint16 u16;
typedef Uint32 u32;
typedef Uint64 u64;

typedef struct Circle{
    float x;
    float y;
    float radius;
}Circle;

typedef struct MarkerPosition{
    float x_inner;
    float y_inner;
    float x_outer;
    float y_outer;
}MarkerPosition;

typedef struct Time {
    u8 hour;
    u8 minute;
    u8 second;
}Time;


