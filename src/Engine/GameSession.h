#pragma once

#include "Player.h"
#include "Map.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include "WeaponTypes.h"
#include "HUD.h"
#include "MapToSegments.h"
#include "BSP.h"
#include "DoomRenderer.h"
#include "GameState.h"

#include <memory>
#include <vector>
#include <SDL2/SDL.h>

class Renderer;

class TextureManager;

struct Wave {
    float spawnInterval;
    std::vector<EnemyType> enemies;
};

class GameSession {
public:
    GameSession(Renderer& renderer, int screenW, int screenH);
    ~GameSession();

    void update(float dt, const Uint8* keys, GameState& gameState);
    void render(Renderer& renderer, uint32_t* pixels, int screenW, int screenH, TextureManager& textureManager);

    void renderPaused(Renderer& renderer, uint32_t* pixels, int screenW, int screenH, float pauseT, TextureManager& textureManager);

    std::vector<Wave> waves;
    int currentWaveIndex = -1;

private:
    // Core gameplay state
    Player player;
    Map worldMap;
    EnemyManager enemyManager;
    WeaponManager weaponManager;
    Weapon weapon;
    HUD hud;

    // BSP world rendering
    std::vector<GridSegment> segments;
    std::unique_ptr<BSPNode> bspRoot;
    std::unique_ptr<DoomRenderer> doomRenderer;

    float* zBuffer = nullptr;

    void initWorld(Renderer& renderer, int screenW);

    // Wave control 
    void startWave(int index);

    float spawnTimer = 0.0f;
    float postWaveTimer = 0.0f;

    size_t enemiesSpawned = 0;

    enum class WaveState {
        Spawning,
        WaitingForClear,
        PostWaveDelay
    };

    WaveState waveState = WaveState::PostWaveDelay;
};
