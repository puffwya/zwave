#include "Engine/Renderer.h"
#include "Engine/GameSession.h"
#include "Engine/GameState.h"
#include "Engine/TextureManager.h"

#include "Menu/MainMenu.h"
#include "Menu/PauseMenu.h"
#include "Intro/StudioIntro.h"

#include "audio/AudioManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <memory>

int SCREEN_WIDTH = 1200;
int SCREEN_HEIGHT = 900;

int main() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    // UI / Engine lifetime
    MainMenu mainMenu;
    mainMenu.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);

    PauseMenu pauseMenu;
    pauseMenu.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);

    StudioIntro studioIntro;
    studioIntro.init(renderer.getSDLRenderer(), SCREEN_WIDTH, SCREEN_HEIGHT);

    AudioManager audio;
    audio.init();
    audio.loadSFX("wall_slide", "Assets/audio/wall_slide.mp3");
    audio.loadSFX("walk", "Assets/audio/walk1.mp3");
    audio.loadSFX("jump", "Assets/audio/jump.mp3");
    audio.loadSFX("gun_pickup", "Assets/audio/gun_pickup.mp3");
    audio.loadSFX("heal_pickup", "Assets/audio/heal_pickup.mp3");
    audio.loadSFX("armor_pickup", "Assets/audio/armor_pickup.mp3");
    audio.loadSFX("gun_click", "Assets/audio/gun_click.mp3");
    audio.loadSFX("item_swap", "Assets/audio/item_swap.mp3");
    audio.loadSFX("pistol_shoot", "Assets/audio/pistol_shoot.mp3");
    audio.loadSFX("pistol_reload", "Assets/audio/pistol_reload.mp3");
    audio.loadSFX("shotgun_shoot", "Assets/audio/shotgun_shoot.mp3");
    audio.loadSFX("shotgun_reload", "Assets/audio/shotgun_reload.mp3");

    // Load geometry textures
    TextureManager textures;

    textures.load("floor1", "Assets/geometry_textures/floor1.png");
    textures.load("wall1", "Assets/geometry_textures/wall1.png");
    textures.load("lava1", "Assets/geometry_textures/lava1.png");
    textures.load("wallTop1", "Assets/geometry_textures/wallTop1.png");

    // SDL_image init
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init error: " << IMG_GetError() << std::endl;
        return -1;
    }

    uint32_t* pixels = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];

    // Game state
    GameState gameState = GameState::StudioIntro;
    std::unique_ptr<GameSession> session = nullptr;

    bool running = true;
    bool mainRun = true;

    Uint32 last = SDL_GetTicks();
    float pauseT = 0.0f;

    // =========================
    // Main game loop
    // =========================
    while (mainRun) {
        running = true;

        // -------------------------
        // INTRO
        // -------------------------
        if (gameState == GameState::StudioIntro) {

            audio.stopMusic();
            audio.playMusic("Assets/audio/IntroJingle.mp3", false);

            studioIntro.start();

            while (running && gameState == GameState::StudioIntro) {
                Uint32 now = SDL_GetTicks();
                float dt = (now - last) / 1000.f;
                last = now;

                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        running = false;
                        mainRun = false;
                    }
                    studioIntro.handleEvent(e, gameState);
                }

                studioIntro.update(dt, gameState);
                studioIntro.render(renderer.getSDLRenderer());
                renderer.present();
            }
        }

        // -------------------------
        // MAIN MENU
        // -------------------------
        if (gameState == GameState::MainMenu) {

            audio.stopMusic();
            audio.playMusic("Assets/audio/FurySyrgeMainTheme.mp3", true);

            while (running && gameState == GameState::MainMenu) {
                Uint32 now = SDL_GetTicks();
                float dt = (now - last) / 1000.f;
                last = now;

                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) {
                        running = false;
                        mainRun = false;
                    }

                    mainMenu.handleInput(e, gameState, running, mainRun);
                }

                // Start game and create new game session
                if (gameState == GameState::Playing && !session) {
                    session = std::make_unique<GameSession>(
                        renderer,
                        SCREEN_WIDTH,
                        SCREEN_HEIGHT
                    );
                }

                mainMenu.updateCursor(dt);
                mainMenu.render(renderer.getSDLRenderer());
                renderer.present();
            }
        }

        // -------------------------
        // GAMEPLAY
        // -------------------------
        else if (gameState == GameState::Playing && session) {

            audio.stopMusic();
            audio.playMusic("Assets/audio/FurySyrgeLVL1Theme.mp3", true);

            while (running && gameState != GameState::MainMenu) {
                Uint32 now = SDL_GetTicks();
                float dt = (now - last) / 1000.f;
                last = now;

                // ======================
                // PAUSED
                // ======================
                if (gameState == GameState::Paused) {
                    pauseT = std::min(pauseT + 0.1f, 1.0f);

                    // Render frozen world
                    session->renderPaused(
                        renderer,
                        pixels,
                        SCREEN_WIDTH,
                        SCREEN_HEIGHT,
                        pauseT,
                        textures
                    );

                    SDL_Renderer* sdl = renderer.getSDLRenderer();
                    SDL_Texture* screen = renderer.getScreenTexture();

                    SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_BLEND);

                    int blurRadius = (int)(pauseT * 3);
                    Uint8 blurAlpha = (Uint8)(pauseT * 12);

                    for (int x = -blurRadius; x <= blurRadius; x += 2) {
                        for (int y = -blurRadius; y <= blurRadius; y += 2) {
                            if (x == 0 && y == 0) continue;
                            SDL_Rect dst = { x, y, SCREEN_WIDTH, SCREEN_HEIGHT };
                            SDL_SetTextureAlphaMod(screen, blurAlpha);
                            SDL_RenderCopy(sdl, screen, nullptr, &dst);
                        }
                    }

                    SDL_SetTextureAlphaMod(screen, 255);

                    Uint8 dimAlpha = (Uint8)(pauseT * 120);
                    SDL_SetRenderDrawBlendMode(sdl, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(sdl, 0, 0, 0, dimAlpha);
                    SDL_RenderFillRect(sdl, nullptr);

                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT) {
                            running = false;
                            mainRun = false;
                        }
                        pauseMenu.handleInput(e, gameState, running);
                    }

                    // Exit to menu, destroy current game session
                    if (gameState == GameState::MainMenu) {
                        session.reset();
                        pauseT = 0.0f;
                        break;
                    }

                    pauseMenu.updateCursor();
                    pauseMenu.render(renderer.getSDLRenderer());
                    renderer.present();
                }

                // ======================
                // GAME RUNNING
                // ======================
                else {
                    pauseT = 0.0f;

                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT) {
                            running = false;
                            mainRun = false;
                        }
                    }

                    const Uint8* keys = SDL_GetKeyboardState(nullptr);

                    session->update(dt, keys, gameState, audio);
                    session->render(renderer, pixels, SCREEN_WIDTH, SCREEN_HEIGHT, textures);
                }
            }
        }
    }

    delete[] pixels;
    audio.shutdown();
    IMG_Quit();
    studioIntro.cleanup();
    return 0;
}
