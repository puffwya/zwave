set(SOURCES
    main.cpp
    Engine/Renderer.cpp
    Engine/Player.cpp
    Engine/Enemy.cpp
    Engine/EnemyManager.cpp
    Engine/PickupManager.cpp
    Engine/SpriteRenderer.cpp
    Engine/pItemRenderer.cpp
    Engine/WeaponManager.cpp
    Engine/BulletHoleManager.cpp
    Engine/MapToSegments.cpp
    Engine/BSP.cpp
    Engine/DoomRenderer.cpp
    Engine/HUD.cpp
    Menu/MainMenu.cpp
    Menu/PauseMenu.cpp
    Menu/GameOver.cpp
    Menu/LevelEnd.cpp
    audio/AudioManager.cpp
    Engine/GameSession.cpp
    Engine/TextureManager.cpp
    Intro/StudioIntro.cpp
    third_party/stb_image_wrapper.cpp
    Utils/PathUtils.cpp
)

add_executable(FurySyrge ${SOURCES})

add_custom_command(TARGET FurySyrge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/Assets $<TARGET_FILE_DIR:FurySyrge>/Assets
    COMMENT "Copying Assets folder into bin..."
)

# SDL2 include directory
target_include_directories(FurySyrge PRIVATE /opt/homebrew/include)

# SDL2 libraries (Homebrew)
find_library(SDL2_LIBRARY SDL2 PATHS /opt/homebrew/lib)
find_library(SDL2MAIN_LIBRARY SDL2main PATHS /opt/homebrew/lib)

if(NOT SDL2_LIBRARY)
    message(FATAL_ERROR "SDL2 library not found. Install with: brew install sdl2")
endif()

target_link_libraries(FurySyrge PRIVATE ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY})

# Define source and destination Assets folder
set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/src/Assets")
set(ASSETS_DST_DIR "${CMAKE_BINARY_DIR}/Assets")

# Copy entire Assets directory (including subfolders)
file(COPY ${ASSETS_SRC_DIR}
     DESTINATION ${CMAKE_BINARY_DIR})

