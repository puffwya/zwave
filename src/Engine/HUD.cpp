#include "HUD.h"
#include <SDL2/SDL_image.h>
#include <iostream>

bool HUD::init(SDL_Renderer* renderer) {
    // Load digits 0-9 + "/"
    for (int i = 0; i < 11; ++i) {
        std::string path = "Assets/pixDigit/pixelDigit-" + std::to_string(i) + ".png";

        SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
        if (!tex) {
            std::cerr << "Failed to load digit " << i
                      << " from path: " << path
                      << " | SDL_image error: " << IMG_GetError() << "\n";
            return false;
        }
        digitTextures[i] = tex;
    }

    // Query width/height from first digit for layout
    if (digitTextures[0]) {
        SDL_QueryTexture(digitTextures[0], nullptr, nullptr, &digitW, &digitH);
    }

    // Load Wave PNG
    SDL_Surface* surface = IMG_Load("assets/pixWords/wave.png");
    if (!surface) {
        std::cerr << "Failed to load wave.png: " << IMG_GetError() << std::endl;
        return false;
    }
    waveTextTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!waveTextTexture) {
        std::cerr << "Failed to create wave texture\n";
        return false;
    }

    // Load Enemies Left PNG
    surface = IMG_Load("assets/pixWords/enemiesLeft.png");
    if (!surface) {
        std::cerr << "Failed to load enemiesLeft.png: " << IMG_GetError() << std::endl;
        return false;
    }
    enemiesLeftTextTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!enemiesLeftTextTexture) {
        std::cerr << "Failed to create enemiesLeft texture\n";
        return false;
    }

    // Load Wave Starting In PNG
    surface = IMG_Load("assets/pixWords/waveStarting.png"); 
    if (!surface) {
        std::cerr << "Failed to load waveStarting.png: " << IMG_GetError() << std::endl;
        return false;    
    }
    waveStartingTextTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!waveStartingTextTexture) {
        std::cerr << "Failed to create waveStarting texture\n";
        return false;
    }

    return true;
}

void HUD::drawAmmoTicks(SDL_Renderer* renderer,
                        int clip,
                        int clipSize,
                        int x,     // RIGHT EDGE of ticks
                        int y,
                        int scale,
                        const AmmoTickStyle& style)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int w = style.tickW * scale;
    int h = style.tickH * scale;
    int spacing = style.spacing * scale;

    int totalTicksW =
        clipSize * w +
        (clipSize - 1) * spacing;

    int startX = x - totalTicksW;

    for (int i = 0; i < clipSize; i++) {
        bool filled = i < clip;

        SDL_Color col = filled
            ? SDL_Color{ 230, 230, 235, 255 }
            : SDL_Color{ 120, 120, 125, 120 };

        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);

        SDL_Rect tick{
            startX + i * (w + spacing),
            y,
            w,
            h
        };

        SDL_RenderFillRect(renderer, &tick);
    }
}

void HUD::drawWaveBanner(SDL_Renderer* renderer,
                         int screenW,
                         int screenH,
                         int currentWave,
                         int totalWaves,
                         int enemiesRemaining,
                         float postWaveTimer)
{
    constexpr int WAVE_START_DELAY = 5;

    int countdownSeconds = 0;
    bool drawCountdown = postWaveTimer > 0.0f;

    if (drawCountdown) {
        countdownSeconds = WAVE_START_DELAY - (int)postWaveTimer;
        countdownSeconds = std::max(0, countdownSeconds);
    }

    int y = 20; // top margin
    int spacing = 2; // space between digits

    int texW, texH;

    // Measure all elements to compute total width
    int totalWidth = 0;

    // Wave word
    SDL_QueryTexture(waveTextTexture, nullptr, nullptr, &texW, &texH);
    int waveH = texH; // scale digits to this height
    totalWidth += texW + spacing;

    // Current wave digits
    std::string currentWaveStr = std::to_string(currentWave + 1);
    totalWidth += currentWaveStr.size() * waveH; // use square based on word height
    totalWidth += (currentWaveStr.size() - 1) * spacing;

    // "/" separator
    totalWidth += waveH + spacing;

    // Total waves digits
    std::string totalWavesStr = std::to_string(totalWaves);
    totalWidth += totalWavesStr.size() * waveH + (totalWavesStr.size() - 1) * spacing;

    // Enemies Left word
    SDL_QueryTexture(enemiesLeftTextTexture, nullptr, nullptr, &texW, &texH);
    totalWidth += 10 + texW + spacing; // 10 px extra padding

    // Enemies remaining digits
    std::string enemiesStr = std::to_string(enemiesRemaining);
    totalWidth += enemiesStr.size() * waveH + (enemiesStr.size() - 1) * spacing;

    // Starting X for perfect center
    int x = (screenW - totalWidth) / 2;

    // Draw Wave word
    SDL_QueryTexture(waveTextTexture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstWave { x, y, texW, texH };
    SDL_RenderCopy(renderer, waveTextTexture, nullptr, &dstWave);
    x += texW + spacing;

    // Draw current wave digits
    if (currentWave <= totalWaves - 1) {
        for (char c : currentWaveStr) {
            int d = c - '0';
            SDL_Rect dstDigit { x, y, waveH, waveH }; // scale digits to match Wave text height
            SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dstDigit);
            x += waveH + spacing;
        }
    }
    else {
        SDL_Rect dstDigit { x, y, waveH, waveH }; // scale digits to match Wave text height
        SDL_RenderCopy(renderer, digitTextures[5], nullptr, &dstDigit);
        x += waveH + spacing;
    }

    // Draw "/" separator
    SDL_Rect dstSlash { x, y, waveH, waveH };
    SDL_RenderCopy(renderer, digitTextures[10], nullptr, &dstSlash);
    x += waveH + spacing;

    // Draw total waves digits
    for (char c : totalWavesStr) {
        int d = c - '0';
        SDL_Rect dstDigit { x, y, waveH, waveH };
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dstDigit);
        x += waveH + spacing;
    }

    // Draw "Enemies Left"
    SDL_QueryTexture(enemiesLeftTextTexture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstEnemies { x + 10, y, texW, texH };
    SDL_RenderCopy(renderer, enemiesLeftTextTexture, nullptr, &dstEnemies);
    x = dstEnemies.x + dstEnemies.w + 5;

    // Draw enemies remaining digits
    for (char c : enemiesStr) {
        int d = c - '0';
        SDL_Rect dstDigit { x, y, texH, texH }; // match word height
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dstDigit);
        x += texH + spacing;
    }
    int secondLineY = y + waveH + 6; // small gap below banner

    if (drawCountdown) {

        // Measure countdown line
        int lineWidth = 0;

        // "Wave starting in:" text
        SDL_QueryTexture(waveStartingTextTexture, nullptr, nullptr, &texW, &texH);
        int textH = texH;
        lineWidth += texW + spacing;

        // Countdown digits
        std::string countdownStr = std::to_string(countdownSeconds);
        lineWidth += countdownStr.size() * textH;
        lineWidth += (countdownStr.size() - 1) * spacing;

        // Center X for second line
        int cx = (screenW - lineWidth) / 2;

        // Draw text
        SDL_Rect dstText { cx, secondLineY, texW, texH };
        SDL_RenderCopy(renderer, waveStartingTextTexture, nullptr, &dstText);
        cx += texW + spacing;

        // Draw countdown digits
        for (char c : countdownStr) {
            int d = c - '0';
            SDL_Rect dstDigit { cx, secondLineY, textH, textH };
            SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dstDigit);
            cx += textH + spacing;
        }
    }
}

void HUD::drawReserveAmmo(SDL_Renderer* renderer,
                          int reserve,
                          int x,
                          int y,
                          int scale)
{
    std::string reserveStr = std::to_string(reserve);

    int spacing = 2 * scale;

    int totalW =
        reserveStr.size() * (digitW * scale) +
        (reserveStr.size() - 1) * spacing;

    int drawX = x - totalW;

    for (char c : reserveStr) {
        int d = c - '0';
        SDL_Rect dst{
            drawX,
            y,
            digitW * scale,
            digitH * scale
        };
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dst);
        drawX += digitW * scale + spacing;
    }
}

SDL_Color HUD::getHealthColor(int value, int maxValue)
{
    float pct = (float)value / (float)maxValue;

    if (pct > 0.6f)
        return { 40, 220, 90, 255 };   // green
    else if (pct > 0.35f)
        return { 230, 200, 40, 255 };  // yellow
    else if (pct > 0.15f)
        return { 230, 120, 40, 255 };  // orange
    else
        return { 220, 40, 40, 255 };   // red
}

SDL_Color HUD::getArmorColor(int value, int maxValue)
{
    float pct = (float)value / (float)maxValue;

    Uint8 intensity = (Uint8)(180 + 75 * pct);
    return { 60, 140, intensity, 255 };
}

void HUD::drawDiamond(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Four diamond points
    SDL_Point top    = { cx, cy - size };
    SDL_Point right  = { cx + size, cy };
    SDL_Point bottom = { cx, cy + size };
    SDL_Point left   = { cx - size, cy };

    // Draw lines connecting points
    for (int t = 0; t < thickness; t++) {
        SDL_RenderDrawLine(renderer, top.x, top.y - t, right.x, right.y - t);
        SDL_RenderDrawLine(renderer, right.x, right.y + t, bottom.x, bottom.y + t);
        SDL_RenderDrawLine(renderer, bottom.x, bottom.y + t, left.x, left.y + t);
        SDL_RenderDrawLine(renderer, left.x, left.y - t, top.x, top.y - t);
    }
}

void HUD::drawHealthCross(SDL_Renderer* renderer, int cx, int cy,
                          int width, int height, SDL_Color color, int thickness)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int tHalf = thickness / 2; // half thickness for centering

    int halfWidth  = width / 2;
    int halfHeight = height / 2;

    // Vertical line: expand horizontally by thickness
    for (int xOff = -tHalf; xOff <= tHalf; xOff++)
    {
        SDL_RenderDrawLine(renderer, cx + xOff, cy - halfHeight, cx + xOff, cy + halfHeight);
    }

    // Horizontal line: expand vertically by thickness
    for (int yOff = -tHalf; yOff <= tHalf; yOff++)
    {
        SDL_RenderDrawLine(renderer, cx - halfWidth, cy + yOff, cx + halfWidth, cy + yOff);
    }
}

void HUD::drawArmorShield(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int half = size / 2;
    int quarter = size / 4;

    // Points for a medieval "heater" shield
    SDL_Point topLeft     = { cx - half, cy - half };         // top-left
    SDL_Point topRight    = { cx + half, cy - half };         // top-right
    SDL_Point bottomPoint = { cx, cy + half };               // bottom tip
    SDL_Point leftCurve   = { cx - quarter, cy + quarter };  // left inward
    SDL_Point rightCurve  = { cx + quarter, cy + quarter };  // right inward

    // Connect points clockwise: top-left → top-right → right-curve → bottom → left-curve → top-left
    SDL_Point points[6] = { topLeft, topRight, rightCurve, bottomPoint, leftCurve, topLeft };

    // Draw shield edges with thickness
    for (int t = 0; t < thickness; t++)
    {
        for (int i = 0; i < 5; i++)
        {
            SDL_RenderDrawLine(renderer,
                               points[i].x, points[i].y + t,
                               points[i+1].x, points[i+1].y + t);
        }
    }
}

// Creates diamond shaped health and armor bars
void HUD::drawDiamondBar(SDL_Renderer* renderer,
                         int cx, int cy, int size,
                         int value, int maxValue,
                         SDL_Color barColor,
                         SDL_Color frameColor,
                         int thickness)
{
    float pct = (float)value / (float)maxValue;
    pct = std::clamp(pct, 0.0f, 1.0f);
        
    // Diamond points
    SDL_Point top = { cx, cy - size };
    SDL_Point right = { cx + size, cy };
    SDL_Point bottom = { cx, cy + size };
    SDL_Point left = { cx - size, cy };
        
    // Draw inner background first
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Color innerBg = { 40, 40, 45, 160 }; // semi-transparent charcoal
    SDL_SetRenderDrawColor(renderer, innerBg.r, innerBg.g, innerBg.b, innerBg.a);

    // Fill diamond using horizontal scanlines across the full diamond shape
    for (int y = top.y + 1; y < bottom.y; y++)
    {
        int xLeft, xRight;
        if (y < cy) // upper half of diamond
        {
            float t = (float)(y - top.y) / (cy - top.y);
            xLeft  = (int)((1.0f - t) * top.x + t * left.x);
            xRight = (int)((1.0f - t) * top.x + t * right.x);
        }
        else // lower half of diamond
        {
            float t = (float)(y - cy) / (bottom.y - cy);
            xLeft  = (int)((1.0f - t) * left.x + t * bottom.x);
            xRight = (int)((1.0f - t) * right.x + t * bottom.x);
        }
        if (xRight < xLeft) std::swap(xLeft, xRight);
        SDL_RenderDrawLine(renderer, xLeft, y, xRight, y);
    }
        
    // Draw full outline first
    SDL_SetRenderDrawColor(renderer, frameColor.r, frameColor.g, frameColor.b, frameColor.a);
        
    auto drawLineThick = [&](SDL_Point a, SDL_Point b) {
        for (int tOff = 0; tOff < thickness; tOff++) {
            SDL_RenderDrawLine(renderer, a.x, a.y + tOff, b.x, b.y + tOff);
        }
    };   
     
    drawLineThick(top, right);
    drawLineThick(right, bottom);
    drawLineThick(bottom, left);
    drawLineThick(left, top);

    // Draw inner bar along perimeter
    SDL_SetRenderDrawColor(renderer, barColor.r, barColor.g, barColor.b, barColor.a);
                         
    // Perimeter length approximation (each side = size * sqrt(2))
    float sideLength = std::sqrtf(2.0f * size * size);
    float totalLength = sideLength * 4.0f;
    float filledLength = totalLength * pct;
 
    auto drawSegment = [&](SDL_Point a, SDL_Point b, float &remaining) {
        if (remaining <= 0) return;   
        
        float dx = (float)(b.x - a.x);
        float dy = (float)(b.y - a.y);
        float len = std::sqrtf(dx*dx + dy*dy);
    
        float t = std::min(1.0f, remaining / len);
        
        int xEnd = (int)(a.x + dx * t);
        int yEnd = (int)(a.y + dy * t);
        
        for (int tOff = 0; tOff < thickness; tOff++) {  
            SDL_RenderDrawLine(renderer, a.x, a.y + tOff, xEnd, yEnd + tOff);
        }
         
        remaining -= len * t;
    };
    
    float remaining = filledLength;
    drawSegment(top, right, remaining);
    drawSegment(right, bottom, remaining);
    drawSegment(bottom, left, remaining);
    drawSegment(left, top, remaining);
}

void HUD::render(SDL_Renderer* renderer,
                 const Player& player,
                 int screenW,
                 int screenH, Weapon& weapon,
                 int currentWave,
                 int totalWaves,
                 int enemiesRemaining,
                 float postWaveTimer)
{
    // Draw current wave and enemies left
    drawWaveBanner(renderer, screenW, screenH,
               currentWave,
               totalWaves,
               enemiesRemaining,
               postWaveTimer);

    // Health and Armor bar pos
    int paddingX = screenW / 40;   // ~2.5% of screen width
    int paddingY = screenH / 40;   // ~2.5% of screen height

    // Health diamond
    int healthSize = screenH / 12;
    int healthThickness = healthSize / 6;
    int healthCx = paddingX + healthSize;
    int healthCy = screenH - paddingY - healthSize;

    SDL_Color hpColor = getHealthColor(player.health, player.maxHealth);
    SDL_Color hpFrame = { 200, 200, 200, 255 };
    drawDiamondBar(renderer, healthCx, healthCy, healthSize, player.health, player.maxHealth, hpColor, hpFrame, healthThickness);

    // Draw cross inside of health diamond
    int crossWidth  = healthSize;
    int crossHeight = healthSize;
    drawHealthCross(renderer, healthCx, healthCy, crossWidth, crossHeight, hpFrame, healthThickness);

    // Armor diamond
    int armorSize = screenH / 16;
    int armorThickness = armorSize / 5;
    int armorCx = (healthCx + healthSize + armorSize / 2) + 20;
    int armorCy = healthCy - healthSize / 2;

    SDL_Color armorColor = getArmorColor(player.armor, player.maxArmor);
    SDL_Color armorFrame = { 150, 180, 255, 255 };
    drawDiamondBar(renderer, armorCx, armorCy, armorSize, player.armor, player.maxArmor, armorColor, armorFrame, armorThickness);

    // Draw plate inside of armor diamond
    int armorPlateSize = (int)(armorSize * 0.65f);
    drawArmorShield(renderer, armorCx, armorCy, armorPlateSize, armorFrame, armorThickness);

    // Ammo panel dimensions
    int ammoWidth  = screenW / 6;
    int ammoHeight = screenH / 12;

    // Base positions (bottom-right)
    int ammoX = screenW - ammoWidth;
    int ammoY = screenH - ammoHeight;

    // Ammo screen scaling
    float hudScaleF = (float)screenH / 720.0f;
    hudScaleF *= 0.75f;
    int hudScale = std::max((int)hudScaleF, 1);

    // Padding inside panel
    int ammoTextPaddingX = paddingX;
    int ammoTextPaddingY = paddingY;

    // Right-aligned reserve digits
    int ammoTextX = screenW - ammoTextPaddingX;
    int ammoTextY = ammoY + ammoTextPaddingY;

    int digitSpacing = 2 * hudScale;

    int reserve = 3;

    int styleH = 18;

    int styleW = 5;

    int styleSpacing = 4;

    int clipSize = 30;

    if (player.currentItem == ItemType::Pistol) {
        reserve  = weapon.pReserveAmmo;
        styleH = pistolTicks.tickH;
        styleW = pistolTicks.tickW;
        styleSpacing = pistolTicks.spacing;
        clipSize = 8;
    }
    else if (player.currentItem == ItemType::Shotgun) {
        reserve  = weapon.sReserveAmmo;
        styleH = shotgunTicks.tickH;
        styleW = shotgunTicks.tickW;
        styleSpacing = shotgunTicks.spacing;
        clipSize = 2;
    }
    else if (player.currentItem == ItemType::Mg) {
        reserve  = weapon.mgReserveAmmo;
        styleH = mgTicks.tickH;
        styleW = mgTicks.tickW;
        styleSpacing = mgTicks.spacing;
        clipSize = 30;
    }

    std::string reserveStr = std::to_string(reserve);

    int reserveDigitsW =
        reserveStr.size() * (digitW * hudScale) +
        (reserveStr.size() - 1) * digitSpacing;

    // Tick origin (left side of panel)
    int tickGap = 6 * hudScale;

    int tickX = ammoTextX - reserveDigitsW - tickGap;

    int tickY = ammoTextY + (digitH * hudScale - styleH * hudScale) / 2;

    // Tick width
    int tickW = styleW * hudScale;
    int tickSpacing = styleSpacing * hudScale;

    int ticksW =
        clipSize * tickW +
        (clipSize - 1) * tickSpacing;

    // Padding
    int padX = 10 * hudScale;
    int padY = 6 * hudScale;
    int gapBetween = 8 * hudScale;

    // Final panel size
    int panelW = padX + ticksW + gapBetween + reserveDigitsW + padX + 20 * hudScale;
    int tickH = styleH * hudScale;
    int contentH = std::max(digitH * hudScale, tickH);
    int panelH = contentH + padY * 6;

    int panelX = screenW - panelW;
    int panelY = screenH - panelH;

    // Only draw if holding a gun with ammo
    if (player.currentItem == ItemType::Pistol ||
        player.currentItem == ItemType::Shotgun || player.currentItem == ItemType::Mg)
    {
        // Background panel
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 25, 25, 30, 110);
        SDL_Rect ammoBg{ panelX, panelY, panelW, panelH };
        SDL_RenderFillRect(renderer, &ammoBg);

        if (player.currentItem == ItemType::Pistol)
        {
            // Clip ticks
            drawAmmoTicks(
                renderer,
                weapon.pClipAmmo,
                weapon.pClipSize,
                tickX,
                tickY,
                hudScale,
                pistolTicks
            );

            // Reserve digits
            drawReserveAmmo(
                renderer,
                weapon.pReserveAmmo,
                ammoTextX,
                ammoTextY,
                hudScale
            );
        }
        else if (player.currentItem == ItemType::Shotgun)
        {
            drawAmmoTicks(
                renderer,
                weapon.sClipAmmo,
                weapon.sClipSize,
                tickX,
                tickY,
                hudScale,
                shotgunTicks
            );

            drawReserveAmmo(
                renderer,
                weapon.sReserveAmmo,
                ammoTextX,
                ammoTextY,
                hudScale
            );
        }
        else if (player.currentItem == ItemType::Mg)
        {
            drawAmmoTicks(
                renderer,
                weapon.mgClipAmmo,
                weapon.mgClipSize,
                tickX,
                tickY,
                hudScale,
                mgTicks
            );  
                
            drawReserveAmmo(
                renderer,
                weapon.mgReserveAmmo,
                ammoTextX,
                ammoTextY,
                hudScale
            );
        }
    }
}
