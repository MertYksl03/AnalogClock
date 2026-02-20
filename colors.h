typedef struct Color {
    u8 R;
    u8 G;
    u8 B;
    u8 alpha;
}Color;

// Helper function to convert the colors from rgb to hex
unsigned int ColorToHex(Color color) {
    return ((unsigned int)color.R << 24) |
           ((unsigned int)color.G << 16) |
           ((unsigned int)color.B << 8)  |
           ((unsigned int)color.alpha);
}

// Color COLOR_NAME = {RED, GREEN, BLUE, OPACITY};
Color TRANSPARENT = {0, 0, 0, 0};
Color COLOR_BLACK = {0, 0 , 0, 255};
Color COLOR_WHITE = {255, 255, 255, 255};
Color COLOR_RAYWHITE = {245, 245, 245, 255};
Color COLOR_RED = {255, 0, 0, 255};
Color COLOR_GREEN = {0, 255, 0, 255};
Color COLOR_BLUE = {0, 0, 255, 255};
