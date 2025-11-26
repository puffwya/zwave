// Draw world (walls) first using a raycaster then draw billboarded sprites

#include "Engine/Renderer.h"
#include "Engine/Raycaster.h"
#include "Engine/Player.h"
#include "Engine/Map.h"
#include "Engine/EnemyManager.h"
#include "Engine/SpriteRenderer.h"
#include "Engine/pItemRenderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

// Declare the texture before loading
SDL_Texture* enemyTex = nullptr;
SDL_Texture* pistolTex = nullptr;
SDL_Texture* shotgunTex = nullptr;

std::unordered_map<ItemType, SDL_Texture*> weaponTextures;

int main() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Raycaster raycaster;
    Player player;
    Map worldMap;
    EnemyManager enemyManager;
    enemyManager.initialize(renderer.getSDLRenderer());
    enemyManager.scanMapForSpawnPoints(worldMap);

    // Give player guns for testing
    player.giveItem(ItemType::Pistol);   // give the player a pistol
    player.giveItem(ItemType::Shotgun);  // give the player a shotgun

    // Initialize SDL_image (before loading any textures)
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
    }

    // Load the enemy image as a surface
    SDL_Surface* surf = IMG_Load("enemy.png");
    if (!surf) {
        printf("IMG_Load error: %s\n", IMG_GetError());
    } else {
        // Create the texture using the underlying SDL_Renderer*
        enemyTex = SDL_CreateTextureFromSurface(renderer.getSDLRenderer(), surf);
        SDL_FreeSurface(surf); // free the surface after creating the texture

        if (!enemyTex) {
            printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        }
    }

    pistolTex = IMG_LoadTexture(renderer.getSDLRenderer(), "Pistol.png");

    if (!pistolTex) {
        printf("Failed to load pistol: %s\n", IMG_GetError());
    } else {
        int w, h;
        SDL_QueryTexture(pistolTex, NULL, NULL, &w, &h);
        printf("Loaded pistol texture: %d x %d\n", w, h);
    }

    weaponTextures[ItemType::Pistol] = pistolTex;

    shotgunTex = IMG_LoadTexture(renderer.getSDLRenderer(), "Shotgun.png");

    if (!shotgunTex) {
        printf("Failed to load shotgun: %s\n", IMG_GetError());
    } else {
        int w, h;
        SDL_QueryTexture(shotgunTex, NULL, NULL, &w, &h);
        printf("Loaded shotgun texture: %d x %d\n", w, h);
    }
    
    weaponTextures[ItemType::Shotgun] = shotgunTex;

    uint32_t* pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];

    bool running = true;
    Uint32 last = SDL_GetTicks();

    float* zBuffer = new float[SCREEN_WIDTH]; // allocate once outside game loop ideally

    enemyManager.spawnEnemy(EnemyType::Base);

    while (running) {
        Uint32 now = SDL_GetTicks();
        // Delta Time
        float dt = (now - last) / 1000.f;
        last = now;


        // handle events
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                running = false;

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        player.update(dt, keys, worldMap);

        enemyManager.update(dt, player, worldMap);
        
        // clear buffer
        for (int i = 0; i < 800*600; i++)
            pixels[i] = 0xFF202020;

        // render world
        raycaster.render(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, player, worldMap, zBuffer);

        renderer.updateTexture(pixels);

        renderer.beginFrame();

        renderer.drawScreenTexture();

        SpriteRenderer::renderEnemies(renderer.getSDLRenderer(), enemyManager, player, zBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

        // draw player item
        SDL_Texture* itemTex = weaponTextures[player.currentItem];
        pItemRenderer::renderPItem(renderer.getSDLRenderer(), itemTex, SCREEN_WIDTH, SCREEN_HEIGHT, player.currentItem);

        renderer.present();
    }

    delete[] zBuffer;
}

