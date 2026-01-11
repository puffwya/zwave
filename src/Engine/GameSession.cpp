#include "GameSession.h"
#include "Renderer.h"
#include "pItemRenderer.h"
#include <algorithm>
#include <iostream>

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

    // Starting enemies (wave system to be added)
    enemyManager.spawnEnemy(EnemyType::Base);
    enemyManager.spawnEnemy(EnemyType::Tank);
    enemyManager.initialize();
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

void GameSession::update(float dt, const Uint8* keys, GameState& gameState) {
    player.update(dt, keys, worldMap, enemyManager, weaponManager, weapon, gameState);
    enemyManager.update(dt, player, worldMap);
    weaponManager.update(dt, player);
}

void GameSession::render(Renderer& renderer, uint32_t* pixels, int w, int h) {
    std::fill(pixels, pixels + w * h, 0xFF202020);

    doomRenderer->render(
        pixels, w, h,
        player, worldMap, zBuffer, enemyManager
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
    float pauseT
) {
    doomRenderer->render(
        pixels, w, h,
        player, worldMap, zBuffer, enemyManager
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
