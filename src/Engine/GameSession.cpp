#include "GameSession.h"
#include "Renderer.h"
#include "pItemRenderer.h"
#include <algorithm>
#include <iostream>
#include "TextureManager.h"

GameSession::GameSession(Renderer& renderer, int screenW, int screenH) {
    initWorld(renderer, screenW);

    zBuffer = new float[screenW];

    weaponManager.loadAssets(renderer.getSDLRenderer());

    if (!hud.loadDigitTextures(renderer.getSDLRenderer())) {
        std::cerr << "Failed to initialize HUD\n";
    }

    // Player starting inventory
    player.giveItem(ItemType::Pistol);
    player.giveItem(ItemType::Shotgun);

    // Wave defs (temp)
    waves.push_back({
        6.0f, // spawn interval
        {
            EnemyType::Base,
            EnemyType::Base,
            EnemyType::Base,
            EnemyType::Base,
            EnemyType::Base
        }
    });

    waves.push_back({
        5.0f, // spawn interval
        {
            EnemyType::Base, EnemyType::Base, EnemyType::Base,
            EnemyType::Base, EnemyType::Base, EnemyType::Base,
            EnemyType::Base, EnemyType::Base, EnemyType::Base,
            EnemyType::Tank
        }
    });

    // Initialize enemy assets
    enemyManager.loadEnemyAssets();

    // Start in post-wave delay so wave 1 begins after 8s
    waveState = WaveState::PostWaveDelay;
    postWaveTimer = 0.0f;
}

GameSession::~GameSession() {
    delete[] zBuffer;
}

void GameSession::initWorld(Renderer& renderer, int screenW) {
    enemyManager.scanMapForSpawnPoints(worldMap);

    segments = buildSegmentsFromGrid(worldMap);
    bspRoot = buildBSP(segments);
    doomRenderer = std::make_unique<DoomRenderer>(segments, std::move(bspRoot));
}

void GameSession::startWave(int index) {
    currentWaveIndex = index;
    enemiesSpawned = 0;
    spawnTimer = 0.0f;
    waveState = WaveState::Spawning;

    std::cout << "Wave " << index + 1 << " started\n";
}

void GameSession::update(float dt, const Uint8* keys, GameState& gameState) {
    player.update(dt, keys, worldMap, enemyManager, weaponManager, weapon, gameState);
    enemyManager.update(dt, player, worldMap);
    weaponManager.update(dt, player);

    if (currentWaveIndex >= (int)waves.size())
        return;

    Wave& wave = waves[currentWaveIndex];

    switch (waveState) {

        case WaveState::Spawning: {
            spawnTimer += dt;

            if (spawnTimer >= wave.spawnInterval &&
                enemiesSpawned < wave.enemies.size()) {

                enemyManager.spawnEnemy(wave.enemies[enemiesSpawned]);
                enemiesSpawned++;

                spawnTimer = 0.0f;
            }

            // Finished spawning, wait until cleared
            if (enemiesSpawned == wave.enemies.size()) {
                waveState = WaveState::WaitingForClear;
            }
            break;
        }

        case WaveState::WaitingForClear: {
            if (!enemyManager.hasActiveEnemies()) {
                postWaveTimer = 0.0f;
                waveState = WaveState::PostWaveDelay;
            }
            break;
        }

        case WaveState::PostWaveDelay: {
            postWaveTimer += dt;

            if (postWaveTimer >= 8.0f) {
                currentWaveIndex++;
                enemiesSpawned = 0;
                spawnTimer = 0.0f;

                waveState = WaveState::Spawning;

                std::cout << "Wave " << currentWaveIndex + 1 << " started\n";
            }
            break;
        }
    }
}

void GameSession::render(Renderer& renderer, uint32_t* pixels, int w, int h, TextureManager& textureManager) {
    std::fill(pixels, pixels + w * h, 0xFF202020);

    doomRenderer->render(
        pixels, w, h,
        player, worldMap, zBuffer, enemyManager, textureManager
    );

    renderer.updateTexture(pixels);
    renderer.beginFrame();
    renderer.drawScreenTexture();

    WeaponType wType = WeaponType::None;
    if (player.currentItem == ItemType::Pistol) wType = WeaponType::Pistol;
    else if (player.currentItem == ItemType::Shotgun) wType = WeaponType::Shotgun;

    SDL_Texture* itemTex = weaponManager.getCurrentFrame(wType);

    pItemRenderer::renderPItem(
        renderer.getSDLRenderer(),
        itemTex,
        w, h,
        player.currentItem,
        weaponManager
    );

    hud.render(renderer.getSDLRenderer(), player, w, h, weapon);

    renderer.present();
}

void GameSession::renderPaused(
    Renderer& renderer,
    uint32_t* pixels,
    int w,
    int h,
    float pauseT,
    TextureManager& textureManager
) {
    doomRenderer->render(
        pixels, w, h,
        player, worldMap, zBuffer, enemyManager, textureManager
    );

    renderer.updateTexture(pixels);
    renderer.beginFrame();
    renderer.drawScreenTexture();

    // draw weapon + HUD (same as normal)
    WeaponType wType = WeaponType::None;
    if (player.currentItem == ItemType::Pistol) wType = WeaponType::Pistol;
    else if (player.currentItem == ItemType::Shotgun) wType = WeaponType::Shotgun;

    SDL_Texture* itemTex = weaponManager.getCurrentFrame(wType);
    pItemRenderer::renderPItem(
        renderer.getSDLRenderer(),
        itemTex,
        w, h,
        player.currentItem,
        weaponManager
    );

    hud.render(renderer.getSDLRenderer(), player, w, h, weapon);
}
