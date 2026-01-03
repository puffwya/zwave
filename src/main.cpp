// Draw world (walls) first using a raycaster then draw billboarded sprites

#include "Engine/Renderer.h"
#include "Engine/Player.h"
#include "Engine/Map.h"
#include "Engine/EnemyManager.h"
#include "Engine/SpriteRenderer.h"
#include "Engine/pItemRenderer.h"
#include "Engine/WeaponManager.h"
#include "Engine/WeaponTypes.h"
#include "Engine/MapToSegments.h"
#include "Engine/BSP.h"
#include "Engine/DoomRenderer.h"
#include "Engine/HUD.h"
#include "Engine/GameState.h"
#include "Menu/MainMenu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <vector> 
#include <iostream>

int SCREEN_WIDTH = 1200;
int SCREEN_HEIGHT = 900;

int main() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Player player;
    Map worldMap;
    HUD hud;
    EnemyManager enemyManager;
    WeaponManager weaponManager;
    Weapon weapon;
    enemyManager.initialize(renderer.getSDLRenderer());
    enemyManager.scanMapForSpawnPoints(worldMap);
    MainMenu mainMenu;
    mainMenu.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);

    // build segments from grid map
    std::vector<GridSegment> segments = buildSegmentsFromGrid(worldMap);

    // build BSP
    std::unique_ptr<BSPNode> bspRoot = buildBSP(segments);
   
    // extract subsectors (each subsector contains the segments lying in that leaf)
    std::vector<std::vector<GridSegment>> subsectors;
    collectSubsectors(bspRoot, subsectors); 

    // store renderer as global
    DoomRenderer doomRenderer(segments, std::move(bspRoot));

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
    enemyManager.spawnEnemy(EnemyType::Tank);
    enemyManager.initialize(renderer.getSDLRenderer());

    weaponManager.loadAssets(renderer.getSDLRenderer());

    if (!hud.loadDigitTextures(renderer.getSDLRenderer())) {
        std::cerr << "Failed to initialize HUD" << std::endl;
        return -1;
    }

    GameState gameState = GameState::MainMenu;

    if (gameState == GameState::MainMenu) {

        while(running && gameState == GameState::MainMenu) {
            // handle events
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }

                mainMenu.handleInput(e, gameState, running);
            }
            mainMenu.updateCursor();
            mainMenu.render(renderer.getSDLRenderer());

            renderer.present();
        }
    }


    if (gameState == GameState::Playing) {
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
            player.update(dt, keys, worldMap, enemyManager, weaponManager, weapon);

            enemyManager.update(dt, player, worldMap);
        
            weaponManager.update(dt, player);

            // clear buffer
            for (int i = 0; i < 800*600; i++)
                pixels[i] = 0xFF202020;

            doomRenderer.render(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, player, worldMap, zBuffer);

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

            pItemRenderer::renderPItem(renderer.getSDLRenderer(), itemTex, SCREEN_WIDTH, SCREEN_HEIGHT, player.currentItem, weaponManager);

            hud.render(renderer.getSDLRenderer(), player, SCREEN_WIDTH, SCREEN_HEIGHT, weapon);

            renderer.present();
        }
    }
    delete[] zBuffer;
}

