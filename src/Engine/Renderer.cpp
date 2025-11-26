#include "Renderer.h"
#include <stdexcept>

Renderer::Renderer(int w, int h) : width(w), height(h) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
    window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

Renderer::~Renderer() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::beginFrame() {
    SDL_RenderClear(renderer);
}

void Renderer::updateTexture(uint32_t* pixels) {
    SDL_UpdateTexture(texture, NULL, pixels, width * 4);
}

void Renderer::drawScreenTexture() {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

