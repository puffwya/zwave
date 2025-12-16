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
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,2,0,0,0,1},

        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,1,1,1,1,1,1,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,0, 0,0,0,0,0,0,0,0,0,1},

        {1,0,0,0,1,1,0,0,1,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,1,0,0,1,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,1,0,0,1,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,1,1,0,0,1,0, 0,0,0,1,1,1,0,0,0,1},
        {1,0,0,0,0,2,0,0,1,0, 0,0,0,1,0,0,0,0,0,1},

        {1,0,0,0,0,0,0,0,1,0, 0,0,0,1,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,1,1,1,0,1},
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

        data[8][3].height = 0.5f;
        data[8][4].height = 0.5f;
        data[8][5].height = 0.5f;
        data[8][6].height = 0.25f;
        data[8][7].height = 0.25f;

        data[10][4].height = -1.0f;
        data[10][5].height = -1.0f;
        data[11][4].height = -1.0f;
        data[11][5].height = -1.0f;
        data[12][4].height = -1.0f;
        data[12][5].height = -1.0f;
        data[13][4].height = -1.0f;
        data[13][5].height = -1.0f;
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

