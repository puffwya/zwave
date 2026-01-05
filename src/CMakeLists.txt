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
    Engine/HUD.cpp
    Menu/MainMenu.cpp
    Menu/PauseMenu.cpp
    audio/AudioManager.cpp
)

add_executable(RaycasterGame ${SOURCES})

# SDL2 include directory
target_include_directories(RaycasterGame PRIVATE /opt/homebrew/include)

# SDL2 libraries (Homebrew)
find_library(SDL2_LIBRARY SDL2 PATHS /opt/homebrew/lib)
find_library(SDL2MAIN_LIBRARY SDL2main PATHS /opt/homebrew/lib)
find_library(SDL2IMAGE_LIBRARY SDL2_image PATHS /opt/homebrew/lib)

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

# Copy entire Assets directory (including subfolders)
file(COPY ${ASSETS_SRC_DIR}
     DESTINATION ${CMAKE_BINARY_DIR})

