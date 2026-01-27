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

    if (!hud.init(renderer.getSDLRenderer())) {
        std::cerr << "Failed to initialize HUD\n";
    }

    // Wave defs (temp)
    waves.push_back({
        6.0f, // spawn interval
        {
            EnemyType::Base
        }
    });

    waves.push_back({
        5.0f, // spawn interval
        {
            EnemyType::Base, EnemyType::Base, EnemyType::Base,
            EnemyType::Shooter, EnemyType::Shooter, EnemyType::Shooter,
            EnemyType::Base, EnemyType::Base, EnemyType::Base,
            EnemyType::Tank
        }
    });

    // Initialize enemy assets
    enemyManager.loadEnemyAssets();

    // Init pickup assets
    pickupManager.loadPickupAssets();

    pickupManager.addPickup(23.5f, 2.5f, 0.0f, PickupType::Health, WeaponType::None);

    pickupManager.addPickup(5.5f, 2.5f, 0.0f, PickupType::Weapon, WeaponType::Pistol);

    doomRenderer->setPickupManager(pickupManager);

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

void GameSession::startWaveWallAnimations(int waveIndex) {
    // Wave 0 (wave one)
    if (waveIndex == 0) {

        // Tiles to animate
        const std::vector<std::pair<int,int>> tiles = {
            {14, 4},
            {14, 5},
            {15, 4},
            {15, 5}
        };

        for (auto& [x, y] : tiles) {
            auto& tile = worldMap.get(x, y);

            WallHeightAnim anim;
            anim.x = x;
            anim.y = y;
            anim.startHeight = tile.height;   // should be 1.0f
            anim.targetHeight = 0.0f;         // slide down
            anim.progress = 0.0f;
            anim.speed = 0.75f;
            anim.finished = false;

            wallAnims.push_back(anim);
        }
    }
    // Wave 1 (wave two)
    else if (waveIndex == 1) {
        
        // Tiles to animate
        const std::vector<std::pair<int,int>> tiles = {
            {24, 7},
            {24, 8},  
            {24, 9}
        };
        
        for (auto& [x, y] : tiles) {
            auto& tile = worldMap.get(x, y);
            
            WallHeightAnim anim;
            anim.x = x;
            anim.y = y;
            anim.startHeight = tile.height;   // should be 1.0f
            anim.targetHeight = 0.0f;         // slide down
            anim.progress = 0.0f;
            anim.speed = 0.75f;
            anim.finished = false;
      
            wallAnims.push_back(anim);

            pickupManager.addPickup(25.5f, 8.5f, 0.0f, PickupType::Weapon, WeaponType::Shotgun);
            doomRenderer->setPickupManager(pickupManager);
        }
    }
    // Special case for when leaving spawn, walls go back up
    else if (waveIndex == 100) {
    
        // Tiles to animate
        const std::vector<std::pair<int,int>> tiles = {
            {14, 4},
            {14, 5},
            {15, 4},
            {15, 5}
        };
            
        for (auto& [x, y] : tiles) {
            auto& tile = worldMap.get(x, y);
        
            WallHeightAnim anim;
            anim.x = x;
            anim.y = y;
            anim.startHeight = tile.height;   // should be 0.0f
            anim.targetHeight = 1.0f;         // slide up
            anim.progress = 0.0f;
            anim.speed = 0.75f;
            anim.finished = false;
        
            wallAnims.push_back(anim);        
        }   
    }
}

void GameSession::updateWallAnimations(float dt) {
    for (auto& anim : wallAnims) {
        if (anim.finished)
            continue;

        anim.progress += anim.speed * dt;
        anim.progress = std::min(anim.progress, 1.0f);

        // Smooth interpolation (linear for now)
        float t = anim.progress;
        float height =
            anim.startHeight +
            (anim.targetHeight - anim.startHeight) * t;

        worldMap.get(anim.x, anim.y).height = height;

        if (anim.progress >= 1.0f) {
            anim.finished = true;
        }
    }

    // cleanup
    wallAnims.erase(
        std::remove_if(wallAnims.begin(), wallAnims.end(),
                       [](const WallHeightAnim& a) { return a.finished; }),
        wallAnims.end()
    );
}

void GameSession::update(float dt, const Uint8* keys, GameState& gameState) {
    player.update(dt, keys, worldMap, enemyManager, weaponManager, weapon, gameState);
    enemyManager.update(dt, player, worldMap);
    pickupManager.update(player, dt, weapon);
    weaponManager.update(dt, player);
    updateWallAnimations(dt);

    if (currentWaveIndex >= (int)waves.size())
        return;

    if (player.y > 10) {
        startWaveWallAnimations(100);
    }

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

                startWaveWallAnimations(currentWaveIndex);

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

    int safeCurrentWave = std::max(0, currentWaveIndex); // never negative
    int totalWaves = (int)waves.size();
    int enemiesRemaining = 0;
 
    // Only calculate if there is a valid wave
    if (currentWaveIndex >= 0 && currentWaveIndex < totalWaves) {
        Wave& wave = waves[currentWaveIndex];
        int enemiesLeftToSpawn = std::max(0, (int)wave.enemies.size() - (int)enemiesSpawned);
        int activeEnemies = enemyManager.getActiveEnemyCount();
        enemiesRemaining = enemiesLeftToSpawn + activeEnemies;
    }
 
    // Always pass safe values to HUD
    hud.render(renderer.getSDLRenderer(), player, w, h, weapon,
           safeCurrentWave,    // display wave number starting at 1
           totalWaves,
           enemiesRemaining);

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

    int safeCurrentWave = std::max(0, currentWaveIndex); // never negative
    int totalWaves = (int)waves.size();
    int enemiesRemaining = 0;

    // Only calculate if there is a valid wave
    if (currentWaveIndex >= 0 && currentWaveIndex < totalWaves) {
        Wave& wave = waves[currentWaveIndex];
        int enemiesLeftToSpawn = std::max(0, (int)wave.enemies.size() - (int)enemiesSpawned);
        int activeEnemies = enemyManager.getActiveEnemyCount();
        enemiesRemaining = enemiesLeftToSpawn + activeEnemies;
    }

    // Always pass safe values to HUD
    hud.render(renderer.getSDLRenderer(), player, w, h, weapon,
           safeCurrentWave,    // display wave number starting at 1
           totalWaves,
           enemiesRemaining);
}
