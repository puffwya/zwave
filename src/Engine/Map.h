#ifndef MAP_H
#define MAP_H

#include <algorithm>

struct RayHit {
    int tileX;
    int tileY;
    float hitX;        // world X coordinate of ray intersection
    float hitY;        // world Y coordinate of ray intersection
    bool vertical;     // true if hit vertical wall
    float hitFraction; // 0-1 along the tile (horizontal or vertical)
    float hitHeight;   // world Z coordinate of impact
};

class Map {
public:
    static const int SIZE = 30;
    static const int CHUNK_SIZE = 10;

    // -----------------------------
    // Tile type for readability
    // -----------------------------
    enum class TileType {
        Empty = 0,
        Wall  = 1,
        Spawn = 2
    };

    // -----------------------------
    // Full cell data structure
    // -----------------------------
    struct Cell {
        TileType type;
        float height; // 0 = floor, 1 = normal wall, 0> & <1 = step, <0 = pit
        float cHeight; // Ceiling Height 
        bool isExit = false;
        bool isEscape = false;
        bool isSliding = false; // for sliding walls
        bool isLava = false; // For lava pit
    };

    // -----------------------------
    // Editable raw tile map
    // -----------------------------
    int rawMap[SIZE][SIZE] = {

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,0,0,0,1,1,1,1,0,0,0, 0,0,0,1,1,1,1,0,0,0,1,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,0,0,0,1,1,1,1,0,0,0, 0,0,0,1,1,1,1,0,0,0,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,0,0,1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1,0,0,1,1,1},
        {1,1,1,0,0,1,0,0,1,1,1,0,2,0,0, 0,0,0,0,1,1,1,0,2,1,0,0,1,1,1},
        {1,1,1,0,0,1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1,0,0,1,1,1},

        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,1,0,1,1,1,1, 1,1,1,1,0,1,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,1,1,1,1,1},

        {1,1,1,1,1,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,1,0,1,1,1,1, 1,1,1,1,0,1,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},

        {1,1,1,0,0,1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1,0,0,1,1,1},
        {1,1,1,0,0,1,2,0,1,1,1,0,0,1,0, 0,1,2,0,1,1,1,0,0,1,0,0,1,1,1},
        {1,1,1,0,0,1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1,0,0,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,0, 0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // -----------------------------
    // Struct-based map
    // -----------------------------
    Cell data[SIZE][SIZE];

    // -----------------------------
    // Construct: auto-build struct map
    // -----------------------------
    Map() {
        initializeCells();

        // Top left raised platform 
        data[7][8].height = 0.5f;
        data[8][8].height = 0.5f;
        data[9][8].height = 0.5f;
        data[7][9].height = 0.5f;
        data[8][9].height = 0.5f;
        data[9][9].height = 0.5f;
        data[7][10].height = 0.5f;  
        data[8][10].height = 0.5f;
        data[9][10].height = 0.5f;  

        // Top right raised platform
        data[7][19].height = 0.5f;
        data[8][19].height = 0.5f;
        data[9][19].height = 0.5f;
        data[7][20].height = 0.5f;
        data[8][20].height = 0.5f;
        data[9][20].height = 0.5f;
        data[7][21].height = 0.5f;
        data[8][21].height = 0.5f;
        data[9][21].height = 0.5f;

        // Bottom left raised platform
        data[20][8].height = 0.5f;
        data[21][8].height = 0.5f;
        data[22][8].height = 0.5f;  
        data[20][9].height = 0.5f;
        data[21][9].height = 0.5f;  
        data[22][9].height = 0.5f;
        data[20][10].height = 0.5f;
        data[21][10].height = 0.5f;
        data[22][10].height = 0.5f;  

        // Bottom right raised platform
        data[20][19].height = 0.5f;
        data[21][19].height = 0.5f;
        data[22][19].height = 0.5f;
        data[20][20].height = 0.5f;
        data[21][20].height = 0.5f; 
        data[22][20].height = 0.5f;
        data[20][21].height = 0.5f; 
        data[21][21].height = 0.5f;
        data[22][21].height = 0.5f;         

        // Cover walls
        data[12][9].height = 0.25f;
        data[17][9].height = 0.25f;
        data[12][20].height = 0.25f;
        data[17][20].height = 0.25f;

        // Pit walls
        data[12][11].height = -0.25f; 
        data[12][12].height = -0.25f;
        data[12][13].height = -0.25f;   
        data[12][14].height = -0.25f;
        data[12][15].height = -0.25f;
        data[12][16].height = -0.25f;
        data[12][17].height = -0.25f;
        data[12][18].height = -0.25f;
        
        data[13][11].height = -0.25f;
        data[14][11].height = -0.25f;
        data[15][11].height = -0.25f;
        data[16][11].height = -0.25f;
        
        data[13][18].height = -0.25f;
        data[14][18].height = -0.25f;
        data[15][18].height = -0.25f;
        data[16][18].height = -0.25f;
        
        data[17][11].height = -0.25f;
        data[17][12].height = -0.25f;
        data[17][13].height = -0.25f;
        data[17][14].height = -0.25f;
        data[17][15].height = -0.25f;
        data[17][16].height = -0.25f;
        data[17][17].height = -0.25f;
        data[17][18].height = -0.25f;

        // Starting room sliding walls
        data[4][14].isSliding = true;
        data[5][14].isSliding = true;
        data[4][15].isSliding = true;
        data[5][15].isSliding = true;

        // Wave 1 sliding walls
        data[7][24].isSliding = true;
        data[8][24].isSliding = true;
        data[9][24].isSliding = true;

        // Wave 2 sliding walls
        data[20][24].isSliding = true;
        data[21][24].isSliding = true;
        data[22][24].isSliding = true;

        // Wave 3 sliding walls
        data[7][5].isSliding = true;
        data[8][5].isSliding = true;
        data[9][5].isSliding = true;

        // Wave 4 sliding walls
        data[20][5].isSliding = true;
        data[21][5].isSliding = true;
        data[22][5].isSliding = true;

        // Wave 5 sliding walls
        //data[24][14].isSliding = true;
        data[24][15].isSliding = true;

        // LevelEnd Doors
        data[28][14].isExit = true;
        data[28][15].isExit = true;

        // LevelEnd Tiles
        data[27][14].isEscape = true;
        data[27][15].isEscape = true;

        // Lava
        data[12][11].isLava = true;
        data[12][12].isLava = true;
        data[12][13].isLava = true;
        data[12][14].isLava = true;
        data[12][15].isLava = true;
        data[12][16].isLava = true;
        data[12][17].isLava = true;
        data[12][18].isLava = true;
        
        data[13][11].isLava = true; 
        data[14][11].isLava = true;  
        data[15][11].isLava = true;
        data[16][11].isLava = true;
        
        data[13][18].isLava = true;
        data[14][18].isLava = true;
        data[15][18].isLava = true;
        data[16][18].isLava = true;
        
        data[17][11].isLava = true;
        data[17][12].isLava = true;
        data[17][13].isLava = true;
        data[17][14].isLava = true;
        data[17][15].isLava = true; 
        data[17][16].isLava = true; 
        data[17][17].isLava = true;
        data[17][18].isLava = true;
    }

    // Convert raw ints to structured Cell data
    void initializeCells() {
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {

                int v = rawMap[y][x];

                switch (v) {
                    case 0:
                        data[y][x] = {TileType::Empty, 0.0f, 1.0f};
                        break;

                    case 1:
                        data[y][x] = {TileType::Wall, 1.0f, 1.0f}; // default wall and heights
                        break;

                    case 2:
                        data[y][x] = {TileType::Spawn, 0.0f, 1.0f};
                        break;
                }
            }
        }
    }

    // Safe accessor (read-only)
    inline const Cell& get(int x, int y) const {
        x = std::max(0, std::min(x, SIZE - 1));
        y = std::max(0, std::min(y, SIZE - 1));
        return data[y][x];
    }

    // Safe accessor (mutable)
    inline Cell& get(int x, int y) {
        x = std::max(0, std::min(x, SIZE - 1));
        y = std::max(0, std::min(y, SIZE - 1));
        return data[y][x];
    }

    // -----------------------------
    // Chunk querying
    // -----------------------------
    inline int getChunkID(int x, int y) const {
        x = std::max(0, std::min(x, SIZE - 1));
        y = std::max(0, std::min(y, SIZE - 1));
        int chunkX = x / CHUNK_SIZE;
        int chunkY = y / CHUNK_SIZE;
        return chunkY * 2 + chunkX;
    }

    // Simple 2D grid DDA
    bool raycastWall(float x, float y, float dirX, float dirY, float maxRange, RayHit& outHit) const {
        int mapX = int(x);
        int mapY = int(y);

        float deltaDistX = (dirX != 0) ? std::abs(1.0f / dirX) : 1e30f;
        float deltaDistY = (dirY != 0) ? std::abs(1.0f / dirY) : 1e30f;

        int stepX = (dirX < 0) ? -1 : 1;
        int stepY = (dirY < 0) ? -1 : 1;

        float sideDistX = (dirX < 0) ? (x - mapX) * deltaDistX : (mapX + 1.0f - x) * deltaDistX;
        float sideDistY = (dirY < 0) ? (y - mapY) * deltaDistY : (mapY + 1.0f - y) * deltaDistY;

        bool hit = false;
        bool verticalHit = false;

        float distance = 0.0f;

        while (!hit && distance < maxRange) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                verticalHit = true;
                distance = sideDistX - deltaDistX;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                verticalHit = false;
                distance = sideDistY - deltaDistY;
            }

            if (mapY >= 0 && mapY < SIZE && mapX >= 0 && mapX < SIZE && rawMap[mapY][mapX] != 0) {
                hit = true;
            }
        }

        if (!hit) return false;

        // Compute exact intersection position in world space
        float hitX = x + dirX * distance;
        float hitY = y + dirY * distance;

        outHit.tileX = mapX;
        outHit.tileY = mapY;
        outHit.vertical = verticalHit;
        if (verticalHit) {
            outHit.hitFraction = hitY - mapY;  // normal
            if (dirX > 0) outHit.hitFraction = 1.0f - outHit.hitFraction; // flip if coming from right
        } else {
            outHit.hitFraction = hitX - mapX;  // normal
            if (dirY > 0) outHit.hitFraction = 1.0f - outHit.hitFraction; // flip if coming from bottom
        }
        outHit.hitHeight = 0.5f; // just use player height for now

        return true;
    }

    // Map.h (inside class Map)
    inline bool isVerticalWall(int tileX, int tileY, float hitFraction, float rayDirX, float rayDirY) const {
        // Basic approach: compare delta along X vs Y
        // If the ray hits closer to a vertical edge, treat as vertical wall
        // If the ray hits closer to a horizontal edge, treat as horizontal wall

        // If hitFraction is near 0 or 1 along X/Y we can decide
        // A more robust approach: check which neighbor tile is empty to determine orientation
        int left  = std::max(0, tileX-1);
        int right = std::min(SIZE-1, tileX+1);
        int top   = std::max(0, tileY-1);
        int bottom= std::min(SIZE-1, tileY+1);

        // Check neighbors to see which side has a wall
        if (rawMap[tileY][left] == 0 || rawMap[tileY][right] == 0) return true;  // vertical wall
        if (rawMap[top][tileX] == 0 || rawMap[bottom][tileX] == 0) return false; // horizontal wall

        // fallback: pick based on ray direction
        return std::fabs(rayDirX) > std::fabs(rayDirY);
    }
};

#endif

