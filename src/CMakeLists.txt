set(SOURCES
    main.cpp
    Engine/Renderer.cpp
    Engine/Player.cpp
    Engine/Enemy.cpp
    Engine/EnemyManager.cpp
    Engine/SpriteRenderer.cpp
    Engine/pItemRenderer.cpp
    Engine/WeaponManager.cpp
    Engine/MapToSegments.cpp
    Engine/BSP.cpp
    Engine/DoomRenderer.cpp
)

add_executable(RaycasterGame ${SOURCES})

# SDL2 include directory
target_include_directories(RaycasterGame PRIVATE /opt/homebrew/include)

# SDL2 libraries (Homebrew)
find_library(SDL2_LIBRARY SDL2 PATHS /opt/homebrew/lib)
find_library(SDL2MAIN_LIBRARY SDL2main PATHS /opt/homebrew/lib)
find_library(SDL2IMAGE_LIBRARY SDL2_image PATHS /opt/homebrew/lib)  # <-- Add this

if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "SDL2 library not found. Install with: brew install sdl2")
endif()
if(NOT SDL2IMAGE_LIBRARY)
    message(FATAL_ERROR "SDL2_image library not found. Install with: brew install sdl2_image")
endif()

target_link_libraries(RaycasterGame PRIVATE ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${SDL2IMAGE_LIBRARY})

# Define source and destination Assets folder
set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/src/Assets")
set(ASSETS_DST_DIR "${CMAKE_BINARY_DIR}/Assets")

# Create the destination directory in the build folder, only create if missing
if(NOT EXISTS ${ASSETS_DST_DIR})
    file(MAKE_DIRECTORY ${ASSETS_DST_DIR})
endif()

# Copy all files inside the Assets folder
file(GLOB ASSET_FILES "${ASSETS_SRC_DIR}/*")
foreach(asset ${ASSET_FILES})
    configure_file(${asset} ${ASSETS_DST_DIR} COPYONLY)
endforeach()

