#include "main.h"

#define WebAssembly 0

#if webAssembly
#include <emscripten.h>
#endif

#define TRUE 1
#define FALSE 0

#define DEFAULT_RADIUS 350

#define FPS 30
#define FRAME_TARGET_TIME 1000 / FPS

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *shapeSurface = NULL;
int last_frame_time = 0;

TTF_Font* clockFont = NULL;
SDL_Texture* numberTextures[12];
int numberWidths[12];
int numberHeights[12];

u8 is_running = FALSE;
u8 is_number_visible = TRUE;

Circle clockCircle;
MarkerPosition MarkerPositions[60]; //We know that there are 60 markers
Time currentTime;

float hourHandLength;
float minuteHandLength;
float secondHandLength;

Color BackgroundColor;
Color hourMarkerColor;
Color minuteMarkerColor;
Color hourHandColor;
Color minuteHandColor;
Color secondHandColor;

# if WebAssembly
// SDL main loop
void main_loop() {
    // to handle the input
    SDL_Event event;   

    process_input(event);
    update();
    render();
    
}
#endif

int main(int argc, char *argv[]) {
    
    // Defaults
    int radius = DEFAULT_RADIUS;
    u8 is_dark_theme = TRUE;
    u8 is_daemon = FALSE;

    #if WebAssembly
    // Default colors for web (light mode)
    is_dark_theme = FALSE;
    BackgroundColor = COLOR_RAYWHITE;
    hourMarkerColor = COLOR_BLACK;
    minuteMarkerColor = COLOR_RED;
    hourHandColor = COLOR_BLACK;
    minuteHandColor = COLOR_BLACK;
    secondHandColor = COLOR_RED;
    #else 
    BackgroundColor = COLOR_BLACK;
    hourMarkerColor = COLOR_RAYWHITE;
    minuteMarkerColor = COLOR_RED;
    hourHandColor = COLOR_RAYWHITE;
    minuteHandColor = COLOR_RAYWHITE;
    secondHandColor = COLOR_RED;
    #endif
    
#if !WebAssembly // disable command line options for now for web demo
    int opt;
    while ((opt = getopt(argc, argv, "s:ldbthn")) != -1) {
        switch (opt) {
            case 's':
            radius = atoi(optarg); 
            break;
            case 'l':
            is_dark_theme = FALSE;
            break;
            case 'd':
            is_dark_theme = TRUE;
            break;
            case 'b':
            is_daemon = TRUE;
            break;
            case 't':
            print_current_time();
            return 0;
            case 'n':
            is_number_visible = FALSE;
            break;
            case 'h':
            fprintf(stderr, "Usage: %s [-s radius] [-d] [-l] [-b] [-t] [-n]\n", argv[0]);
            fprintf(stderr, "  -s radius : Set the diameter of the clock (default: %d)\n", DEFAULT_RADIUS);
            fprintf(stderr, "  -d        : Enable dark mode (default)\n");
            fprintf(stderr, "  -l        : Enable light mode\n");
            fprintf(stderr, "  -b        : Run as a background daemon\n");
            fprintf(stderr, "  -t        : Print the current time and exit\n");
            fprintf(stderr, "  -n        : Hide numbers on the clock face\n");
            return 0;
            case '?': // getopt returns '?' for unknown options
            fprintf(stderr, "Usage: %s [-s radius] [-d] [-l] [-b] [-t] [-n]\n", argv[0]);
            return 1;
        }
    }

    if (is_daemon) {
        // Daemonize the process
        if (daemon(1, 0) == -1) {
            return 1; //return fork failed
        }
    }

    if (radius < 50) {
        radius = 50;
        printf("Radius too small, setting to minimum of 50.\n");
    }

    if (radius > 1000) {
        radius = 1000;
        printf("Radius too large, setting to maximum of 1000.\n");
    }
    
    printf("Clock initialized: Diameter %d, Dark Mode: %s\n", 
        radius, is_dark_theme ? "Enabled" : "Disabled");
        
    if (!is_dark_theme) {
        BackgroundColor = COLOR_WHITE;
        hourMarkerColor = COLOR_BLACK;
        minuteMarkerColor = COLOR_RED;
        hourHandColor = COLOR_BLACK;
        minuteHandColor = COLOR_BLACK;
        secondHandColor = COLOR_RED;
    }
    #endif 

    hourHandLength = radius * 0.5f;
    minuteHandLength = radius * 0.75f;
    secondHandLength = radius * 0.9f;
    
    clockCircle = (Circle){radius, radius, radius};
    
    calculateMarkerPositions(MarkerPositions, clockCircle);

    // Change the number size based on the clock radius
    int fontSize = (int)(radius * 0.15f); // 15% of the radius
    if (fontSize < 12) fontSize = 12;
    if (fontSize > 72) fontSize = 72;
    
    is_running = initialize_window(is_dark_theme, fontSize);
    
    #if WebAssembly
        emscripten_set_main_loop(main_loop, 0, 1);
    #endif

    #if !WebAssembly
    SDL_Event event;    

    while (is_running) {
        u32 current_time = SDL_GetTicks();
        process_input(event);
        update();
        render();
    
        u32 target_time = last_frame_time + FRAME_TARGET_TIME;

        if (!SDL_TICKS_PASSED(current_time, target_time)) {
            // Calculate milliseconds remaining and sleep!
            SDL_Delay(target_time - current_time); 
        }

        last_frame_time = SDL_GetTicks();
    }

    QUIT();
    #endif
    return 0;
}

void print_current_time() {
    time_t now = time(NULL);
    struct tm* current_time = localtime(&now);
    printf("Current Time: %02d:%02d:%02d\n", 
        current_time->tm_hour, 
        current_time->tm_min, 
        current_time->tm_sec);
}

void DrawThickLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, float thickness, Color color) {
    // 1. Get the direction of the line
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);

    if (length == 0) return; // Can't draw a line with no length!

    // 2. Get the "Normal" (perpendicular direction) and normalize it
    // Then scale it by half the thickness
    float nx = -dy / length * (thickness / 2.0f);
    float ny = dx / length * (thickness / 2.0f);

    // 3. Define the 4 corners of our "thick line" rectangle
    SDL_Vertex vertices[4];
    Color c = color;

    // Top Left
    vertices[0] = (SDL_Vertex){{x1 + nx, y1 + ny}, {c.R, c.G, c.B, c.alpha}, {0, 0}};
    // Bottom Left
    vertices[1] = (SDL_Vertex){{x1 - nx, y1 - ny}, {c.R, c.G, c.B, c.alpha}, {0, 0}};
    // Top Right
    vertices[2] = (SDL_Vertex){{x2 + nx, y2 + ny}, {c.R, c.G, c.B, c.alpha}, {0, 0}};
    // Bottom Right
    vertices[3] = (SDL_Vertex){{x2 - nx, y2 - ny}, {c.R, c.G, c.B, c.alpha}, {0, 0}};

    // 4. Tell SDL which vertices make up our two triangles
    // Triangle 1: 0, 1, 2 | Triangle 2: 1, 2, 3
    int indices[] = {0, 1, 2, 1, 2, 3};

    SDL_RenderGeometry(renderer, NULL, vertices, 4, indices, 6);

}

#if !WebAssembly // disable command line options for now for web demo --- IGNORE ---
SDL_Surface* CreateCircularMask(int width, int height) {
    // Create a blank 32-bit surface with an alpha channel
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;

    // Lock the surface to directly modify its pixels
    SDL_LockSurface(surface);
    Uint32* pixels = (Uint32*)surface->pixels;

    int radius = width / 2;
    int cx = width / 2;
    int cy = height / 2;

    // Loop through every pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate distance from the center
            int dx = x - cx;
            int dy = y - cy;
            
            // If the pixel is inside the circle, make it opaque white. 
            // If outside, make it completely transparent.
            if ((dx * dx) + (dy * dy) <= (radius * radius)) {
                pixels[y * width + x] = ColorToHex(COLOR_WHITE); 
            } else {
                pixels[y * width + x] = ColorToHex(TRANSPARENT); 
            }
        }
    }

    SDL_UnlockSurface(surface);
    return surface;
}
#endif

int initialize_window(u8 is_dark_theme, int fontSize)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return FALSE;
    }

    int diameter =  clockCircle.radius * 2;
    
#if WebAssembly
    // The web canvas does not support OS-level shaped windows
    window = SDL_CreateWindow(
        "Clock", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        diameter, diameter,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
    );
#else
    window = SDL_CreateShapedWindow(
        "Clock", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        diameter, diameter,
        SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS
    );
#endif

    if(!window) {
        fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
        return FALSE;
    }

#if !WebAssembly 
    shapeSurface = CreateCircularMask(400, 400);
    if (!shapeSurface) {
        printf("Failed to load shape mask!\n");
        return 1;
    }

    SDL_WindowShapeMode shapeMode;
    shapeMode.mode = ShapeModeBinarizeAlpha; 
    shapeMode.parameters.binarizationCutoff = 1; 

    SDL_SetWindowShape(window, shapeSurface, &shapeMode);
    SDL_FreeSurface(shapeSurface);
#endif
    
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if(!renderer) {
        fprintf(stderr,"Error creating SDL Renderer\n");
        return FALSE;
    }

    // IMPORTANT: Enable alpha blending so your transparent background 
    // actually renders as transparent on the web canvas!
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // If numbers are visible, we need to initialize SDL_ttf and load the font
    if (is_number_visible) {
        if (TTF_Init() == -1) {
            fprintf(stderr, "Error initializing SDL_ttf: %s\n", TTF_GetError());
            return FALSE;
        }
    
        clockFont = TTF_OpenFont("fonts/arial.ttf", fontSize);
        if (!clockFont) {
            fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
            return FALSE;
        }
    
        // Pre-render numbers 1-12 into textures
        SDL_Color textColor = {255, 255, 255, 255}; // White text
        if (!is_dark_theme) {
            textColor = (SDL_Color){0, 0, 0, 255};  // Black text for light mode
        }
    
        for (int i = 0; i < 12; i++) {
            char numStr[3];
            sprintf(numStr, "%d", i + 1); // Numbers 1 to 12
    
            // Create a surface from the text, then convert to a hardware texture
            SDL_Surface* textSurface = TTF_RenderText_Blended(clockFont, numStr, textColor);
            numberTextures[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
            
            // Save the width and height so we can center them later
            numberWidths[i] = textSurface->w;
            numberHeights[i] = textSurface->h;
    
            SDL_FreeSurface(textSurface); // Free the surface, we only need the texture
        }
    }

    return TRUE;
}

void process_input(SDL_Event event)  
{ 
    while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
              case SDL_QUIT:
                    is_running = FALSE;
                    break;
                case SDL_KEYUP:
                    break;
                case SDL_KEYDOWN:
                    break;
            }
        }
}

void update() 
{ 
    time_t now = time(NULL);
    struct tm* current_time = localtime(&now);

    currentTime.hour = current_time->tm_hour % 12;
    currentTime.minute = current_time->tm_min;
    currentTime.second = current_time->tm_sec;
}

void SDL_SetRenderDrawColorV(SDL_Renderer* renderer,struct Color color){
    SDL_SetRenderDrawColor(renderer, color.R, color.G, color.B, color.alpha);
}

void render() 
{
    SDL_SetRenderDrawColorV(renderer, TRANSPARENT);
    SDL_RenderClear(renderer);


    //draw a white cirlce
    SDL_SetRenderDrawColorV(renderer, BackgroundColor);
    SDL_RenderFillCircle(
        renderer,
        clockCircle.x,
        clockCircle.y,
        clockCircle.radius
    );

    drawNumbers(clockCircle);
    drawMarkers(MarkerPositions);
    drawClockHands(clockCircle);


    SDL_RenderPresent(renderer);

}

void calculateMarkerPositions(MarkerPosition* MarkerPositions, Circle clockCircle){
    float center_x = clockCircle.x;
    float center_y = clockCircle.y;
    float radius = clockCircle.radius;
    int alpha_deg = 0;

    float x_inner, y_inner, x_outer, y_outer;

    for (u8 i = 0; i < 60; i++) {
        // Draw hour markers
        if (i %  (60 / 12) == 0) {
            x_inner = center_x + radius * 0.80 * sinf(alpha_deg * DEG2RAD);
            y_inner = center_y + radius * 0.80 * cosf(alpha_deg * DEG2RAD);

        } else { // Draw minute markers
            x_inner = center_x + radius * 0.9 * sinf(alpha_deg * DEG2RAD);
            y_inner = center_y + radius * 0.9 * cosf(alpha_deg * DEG2RAD);
        }

        x_outer = center_x + radius * 0.98 * sinf(alpha_deg * DEG2RAD);
        y_outer = center_y + radius * 0.98 * cosf(alpha_deg * DEG2RAD);

        MarkerPositions[i].x_inner = x_inner;
        MarkerPositions[i].y_inner = y_inner;
        MarkerPositions[i].x_outer = x_outer;
        MarkerPositions[i].y_outer = y_outer;

        alpha_deg += 360 / 60; // 360 degree divided by 60 minutes
    }
}

void drawMarkers(MarkerPosition* MarkerPositions){
    float x_inner, y_inner, x_outer, y_outer;
    for (u8 i = 0; i < 60; i++) {
        x_inner = MarkerPositions[i].x_inner;
        y_outer = MarkerPositions[i].y_outer;
        y_inner = MarkerPositions[i].y_inner;
        x_outer = MarkerPositions[i].x_outer;
        if (i %  (60 / 12) == 0) {
            DrawThickLine(renderer, x_inner, y_inner, x_outer, y_outer, 4.0f, hourMarkerColor);
        } else {
            DrawThickLine(renderer, x_inner, y_inner, x_outer, y_outer, 2.0f, minuteMarkerColor);
        }
    }

}

void calculateHandPositions(float* x_end, float* y_end, Circle clockCircle, float handLength, float angleDeg){
    float center_x = clockCircle.x;
    float center_y = clockCircle.y;

    *x_end = center_x + handLength * sinf(angleDeg * DEG2RAD);
    *y_end = center_y - handLength * cosf(angleDeg * DEG2RAD);
}

void calculateHourHandPosition(float* x_end, float* y_end, Circle clockCircle){
    float hour_angle = (currentTime.hour + currentTime.minute / 60.0f) * 30.0f; // 360 degrees / 12 hours = 30 degrees per hour
    calculateHandPositions(x_end, y_end, clockCircle, hourHandLength, hour_angle);
}

void calculateMinuteHandPosition(float* x_end, float* y_end, Circle clockCircle){
    float minute_angle = (currentTime.minute + currentTime.second / 60.0f) * 6.0f; // 360 degrees / 60 minutes = 6 degrees per minute
    calculateHandPositions(x_end, y_end, clockCircle, minuteHandLength, minute_angle);
}

void calculateSecondHandPosition(float* x_end, float* y_end, Circle clockCircle){
    float second_angle = currentTime.second * 6.0f; // 360 degrees / 60 seconds = 6 degrees per second
    calculateHandPositions(x_end, y_end, clockCircle, secondHandLength, second_angle);
}

void drawClockHands(Circle clockCircle){
    float hour_x, hour_y, minute_x, minute_y, second_x, second_y;

    calculateHourHandPosition(&hour_x, &hour_y, clockCircle);
    calculateMinuteHandPosition(&minute_x, &minute_y, clockCircle);
    calculateSecondHandPosition(&second_x, &second_y, clockCircle);

    DrawThickLine(renderer, clockCircle.x, clockCircle.y, hour_x, hour_y, 6.0f, hourHandColor);
    DrawThickLine(renderer, clockCircle.x, clockCircle.y, minute_x, minute_y, 4.0f, minuteHandColor);
    DrawThickLine(renderer, clockCircle.x, clockCircle.y, second_x, second_y, 2.0f, secondHandColor);
}

void drawNumbers(Circle clockCircle) {
    float center_x = clockCircle.x;
    float center_y = clockCircle.y;
    // Place the numbers slightly closer to the center than the hour markers
    float numberRadius = clockCircle.radius * 0.70f; 

    for (int i = 0; i < 12; i++) {
        int hour = i + 1;
        // 12 is at 0 degrees, 1 is at 30 degrees, etc.
        float angle = hour * 30.0f; 

        float x = center_x + numberRadius * sinf(angle * DEG2RAD);
        float y = center_y - numberRadius * cosf(angle * DEG2RAD);

        // Define where to draw the texture. 
        // We subtract half the width/height to perfectly center the text on the x/y coordinate.
        SDL_Rect destRect = {
            (int)(x - numberWidths[i] / 2.0f),
            (int)(y - numberHeights[i] / 2.0f),
            numberWidths[i],
            numberHeights[i]
        };

        SDL_RenderCopy(renderer, numberTextures[i], NULL, &destRect);
    }
}


void QUIT() { 
    if (is_number_visible) {        
        //cleanup textures and font
        // Destroy number textures
        for (int i = 0; i < 12; i++) {
            SDL_DestroyTexture(numberTextures[i]);
        }
    
        // Destroy font
        TTF_CloseFont(clockFont);
        // Quit TTF subsystem
        TTF_Quit();
    }

    // Destroy renderer, window and quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
