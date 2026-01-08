// (OLD) Draw world (walls) first using a raycaster then draw billboarded sprites

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
#include "Menu/PauseMenu.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unordered_map>
#include <vector> 
#include <iostream>
#include "audio/AudioManager.h"

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
    enemyManager.scanMapForSpawnPoints(worldMap);
    MainMenu mainMenu;
    mainMenu.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);
    PauseMenu pauseMenu;
    pauseMenu.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);
    AudioManager audio;
    audio.init();

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

    static float pauseT = 0.0f;

    bool mainRun = true;

    while (mainRun) {
        running = true;
        if (gameState == GameState::MainMenu) {

            Uint32 now = SDL_GetTicks();
            // Delta Time
            float dt = (now - last) / 1000.f;
            last = now;

            audio.stopMusic();
            audio.playMusic("Assets/audio/FurySyrgeMainTheme.mp3", true);

            while(running && gameState == GameState::MainMenu) {
                // handle events
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        running = false;
                        mainRun = false;
                    }

                    mainMenu.handleInput(e, gameState, running, mainRun);
                }
                mainMenu.updateCursor(dt);
                mainMenu.render(renderer.getSDLRenderer());

                renderer.present();
            }
        }

        else if (gameState == GameState::Playing) {

            audio.stopMusic();

            while (running) {
                Uint32 now = SDL_GetTicks();
                // Delta Time
                float dt = (now - last) / 1000.f;
                last = now;

                if (gameState == GameState::Paused) {
                    pauseT = std::min(pauseT + 0.1f, 1.0f);

                    doomRenderer.render(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, player, worldMap, zBuffer, enemyManager);

                    renderer.updateTexture(pixels);

                    renderer.beginFrame();

                    renderer.drawScreenTexture();

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

                    SDL_Renderer* sdl = renderer.getSDLRenderer();
                    SDL_Texture* screen = renderer.getScreenTexture();

                    // Enable blending
                    SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_BLEND);

                    // Subtle blur
                    int blurRadius = (int)(pauseT * 3);
                    Uint8 blurAlpha = (Uint8)(pauseT * 12);

                    for (int x = -blurRadius; x <= blurRadius; x += 2) {
                        for (int y = -blurRadius; y <= blurRadius; y += 2) {
                            if (x == 0 && y == 0) continue; // keep center sharp
                            SDL_Rect dst = { x, y, SCREEN_WIDTH, SCREEN_HEIGHT };
                            SDL_SetTextureAlphaMod(screen, blurAlpha);
                            SDL_RenderCopy(sdl, screen, nullptr, &dst);
                        }
                    }

                    // Reset texture alpha
                    SDL_SetTextureAlphaMod(screen, 255);

                    // Dim overlay
                    Uint8 dimAlpha = (Uint8)(pauseT * 120);
                    SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(sdl, 0, 0, 0, dimAlpha);
                    SDL_RenderFillRect(sdl, nullptr);

                    // handle events
                    SDL_Event e;
                    while (SDL_PollEvent(&e)) { 
                        if (e.type == SDL_QUIT) {
                            running = false;
                            mainRun = false;
                        }
                        pauseMenu.handleInput(e, gameState, running);
                    }
                    pauseMenu.updateCursor();
                    pauseMenu.render(renderer.getSDLRenderer());

                    renderer.present();
                }
                else {
                    pauseT = 0.0f;
                    // handle events
                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT) {
                            running = false;
                            mainRun = false;
                        }
                    }

                    const Uint8* keys = SDL_GetKeyboardState(NULL);
                    player.update(dt, keys, worldMap, enemyManager, weaponManager, weapon, gameState);

                    enemyManager.update(dt, player, worldMap);
    
                    weaponManager.update(dt, player);

                    // clear buffer
                    for (int i = 0; i < 800*600; i++)
                        pixels[i] = 0xFF202020;
            
                    doomRenderer.render(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, player, worldMap, zBuffer, enemyManager);

                    renderer.updateTexture(pixels);

                    renderer.beginFrame();

                    renderer.drawScreenTexture();

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
        }
    }
    delete[] zBuffer;
    audio.shutdown();
}

