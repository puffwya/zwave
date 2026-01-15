#ifndef MAP_H
#define MAP_H

#include <algorithm>

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
        float height;     // 0 = floor, 1 = normal wall, 0> & <1 = step, <0 = pit
        float cHeight;    // Ceiling Height 
    };

    // -----------------------------
    // Editable raw tile map
    // -----------------------------
    int rawMap[SIZE][SIZE] = {

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,1,0,1,0,0,0,0, 0,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,0,1,0,0,0,0, 0,0,0,0,1,0,1,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,1,1,0,2,0,0, 0,0,0,0,1,1,1,0,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,0,0,0,0,0,1},

        {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,0,0,0,0,1,0,0,0,1,0,1,1,1,1, 1,1,1,1,0,1,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},

        {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,1,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,1,0,1,1,1,1, 1,1,1,1,0,1,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},

        {1,0,0,0,0,0,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,2,0,1,1,1,0,0,0,0, 0,0,2,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,1,1,0,0,0,0, 0,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0, 0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0, 0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0, 0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0, 0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
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

