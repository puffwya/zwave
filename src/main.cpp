// Draw world (walls) first using a raycaster then draw billboarded sprites

#include "Engine/Renderer.h"
#include "Engine/Raycaster.h"
#include "Engine/Player.h"
#include "Engine/Map.h"
#include "Engine/EnemyManager.h"
#include "Engine/SpriteRenderer.h"
#include "Engine/pItemRenderer.h"
#include "Engine/WeaponManager.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

int main() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Raycaster raycaster;
    Player player;
    Map worldMap;
    EnemyManager enemyManager;
    WeaponManager weaponManager;
    enemyManager.initialize(renderer.getSDLRenderer());
    enemyManager.scanMapForSpawnPoints(worldMap);

    // Give player guns for testing
    player.giveItem(ItemType::Pistol);   // give the player a pistol
    player.giveItem(ItemType::Shotgun);  // give the player a shotgun

    // Initialize SDL_image (before loading any textures)
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init error: %s\n", IMG_GetError());
    }

    uint32_t* pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];

    bool running = true;
    Uint32 last = SDL_GetTicks();

    float* zBuffer = new float[SCREEN_WIDTH]; // allocate once outside game loop ideally

    enemyManager.spawnEnemy(EnemyType::Base);
    enemyManager.spawnEnemy(EnemyType::Base);
    enemyManager.initialize(renderer.getSDLRenderer());

    weaponManager.loadAssets(renderer.getSDLRenderer());

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
        player.update(dt, keys, worldMap, enemyManager, weaponManager);

        enemyManager.update(dt, player, worldMap);
        
        weaponManager.update(dt);

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

        // Determine what weapon the player is holding
        WeaponType wType = WeaponType::None;

        if (player.currentItem == ItemType::Pistol)
            wType = WeaponType::Pistol;
        else if (player.currentItem == ItemType::Shotgun)
            wType = WeaponType::Shotgun;

        // Get the current animation frame for that weapon
        SDL_Texture* itemTex = weaponManager.getCurrentFrame(wType);

        pItemRenderer::renderPItem(renderer.getSDLRenderer(), itemTex, SCREEN_WIDTH, SCREEN_HEIGHT, player.currentItem);

        renderer.present();
    }

    delete[] zBuffer;
}

