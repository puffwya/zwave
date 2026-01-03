#ifndef MAP_H
#define MAP_H

#include <algorithm>

class Map {
public:
    static const int SIZE = 20;
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
        float height;     // 0 = floor, 1 = normal wall, 0> & <1 = step, <0 = pit
    };

    // -----------------------------
    // Editable raw tile map
    // -----------------------------
    int rawMap[SIZE][SIZE] = {
        {1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1},
        {1,0,0,1,0,1,0,0,0,0, 0,0,0,0,1,0,1,0,0,1},
        {1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1},

        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0, 0,0,0,0,0,1,0,0,0,1},
        {1,0,0,0,0,0,0,1,1,1, 1,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,1,1, 1,1,1,0,0,0,0,0,0,1},

        {1,0,0,0,0,0,0,1,1,1, 1,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,1,1, 1,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0, 0,0,0,0,0,1,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},

        {1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1},
        {1,0,0,1,0,1,0,0,0,0, 0,0,0,0,1,0,1,0,0,1},
        {1,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1}
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
        data[2][3].height = 0.5f;
        data[2][4].height = 0.5f;
        data[2][5].height = 0.5f;
        data[3][3].height = 0.5f;
        data[3][5].height = 0.5f;
        data[4][3].height = 0.5f;
        data[4][4].height = 0.5f;
        data[4][5].height = 0.5f;

        // Top right raised platform
        data[2][14].height = 0.5f;
        data[2][15].height = 0.5f;
        data[2][16].height = 0.5f;
        data[3][14].height = 0.5f;
        data[3][16].height = 0.5f;
        data[4][14].height = 0.5f;
        data[4][15].height = 0.5f;   
        data[4][16].height = 0.5f;

        // Bottom left raised platform
        data[15][3].height = 0.5f;
        data[15][4].height = 0.5f;
        data[15][5].height = 0.5f;
        data[16][3].height = 0.5f;
        data[16][5].height = 0.5f;
        data[17][3].height = 0.5f;
        data[17][4].height = 0.5f;   
        data[17][5].height = 0.5f;

        // Bottom right raised platform
        data[15][14].height = 0.5f;
        data[15][15].height = 0.5f;
        data[15][16].height = 0.5f;
        data[16][14].height = 0.5f;
        data[16][16].height = 0.5f;
        data[17][14].height = 0.5f;
        data[17][15].height = 0.5f;
        data[17][16].height = 0.5f;

        // Cover walls
        data[7][4].height = 0.25f;
        data[12][4].height = 0.25f;
        data[7][15].height = 0.25f;
        data[12][15].height = 0.25f;

        // Pit walls
        data[8][7].height = -0.25f;
        data[8][8].height = -0.25f;
        data[8][9].height = -0.25f;
        data[8][10].height = -0.25f;
        data[8][11].height = -0.25f;
        data[8][12].height = -0.25f;
        data[9][7].height = -0.25f;
        data[9][8].height = -0.25f;
        data[9][9].height = -0.25f;
        data[9][10].height = -0.25f;
        data[9][11].height = -0.25f;
        data[9][12].height = -0.25f;
        data[10][7].height = -0.25f;
        data[10][8].height = -0.25f;
        data[10][9].height = -0.25f;
        data[10][10].height = -0.25f;
        data[10][11].height = -0.25f;
        data[10][12].height = -0.25f;
        data[11][7].height = -0.25f;
        data[11][8].height = -0.25f;
        data[11][9].height = -0.25f;
        data[11][10].height = -0.25f;
        data[11][11].height = -0.25f;
        data[11][12].height = -0.25f;
    }

    // Convert raw ints to structured Cell data
    void initializeCells() {
        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {

                int v = rawMap[y][x];

                switch (v) {
                    case 0:
                        data[y][x] = { TileType::Empty, 0.0f };
                        break;

                    case 1:
                        data[y][x] = { TileType::Wall,  1.0f }; // default wall height
                        break;

                    case 2:
                        data[y][x] = { TileType::Spawn, 0.0f };
                        break;
                }
            }
        }
    }

    // -----------------------------
    // Safe accessor
    // -----------------------------
    inline const Cell& get(int x, int y) const {
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
};

#endif

