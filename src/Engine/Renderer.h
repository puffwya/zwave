#pragma once

// Includes the SDL2 library which gives access to windows, rendering, textures, keyboard input, and timing 
// functions
#include <SDL2/SDL.h>

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    void beginFrame();
    void updateTexture(uint32_t* pixels); // upload to GPU texture (does not present)
    void drawScreenTexture();             // copy the screen texture to the renderer
    void present();                       // SDL_RenderPresent

    SDL_Renderer* getSDLRenderer() const { return renderer; }
    SDL_Texture* getScreenTexture() const { return texture; }

    int width;
    int height;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};

