// SPDX-License-Identifier: WTFPL

#include "chip8.h"
#include "log.h"

#include <array>
#include <raylib.h>
#include <string>

#define RAYGUI_IMPLEMENTATION
#include "../styles/dark/style_dark.h"

#include <raygui.h>

int const WINDOW_WIDTH = 800;
int const WINDOW_HEIGHT = 600;

std::vector<uint8_t> loadRom(std::string const& path)
{
    std::vector<uint8_t> romData;

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        loge("Failed to open ROM file: %s", path.c_str());
        return {};
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    romData.resize(fileSize);
    fread(romData.data(), 1, fileSize, file);
    fclose(file);

    return romData;
}

std::array<char const*, 24> ROMS = {
    "Maze",           "Particle Demo", "Sierpinski",        "Trip8 Demo",  "Stars",
    "pumpkindressup", "br8kout",       "octopeg",           "snake",       "RPS",
    "flightrunner",   "rockto",        "spacejam",          "test_opcode", "1-chip8-logo",
    "2-ibm-logo",     "3-corax+",      "4-flags",           "4-flags (1)", "5-quirks",
    "6-keypad",       "8-scrolling",   "random_number_test"};

std::array<std::string, 24> romPaths{
    "/Users/dmitry/Downloads/Maze (alt) [David Winter, 199x].ch8",
    "/Users/dmitry/Downloads/Particle Demo [zeroZshadow, 2008].ch8",
    "/Users/dmitry/Downloads/Sierpinski [Sergey Naydenov, 2010].ch8",
    "/Users/dmitry/Downloads/Trip8 Demo (2008) [Revival Studios].ch8",
    "/Users/dmitry/Downloads/Stars [Sergey Naydenov, 2010].ch8",
    "/Users/dmitry/Downloads/pumpkindressup.ch8",
    "/Users/dmitry/Downloads/br8kout.ch8",
    "/Users/dmitry/Downloads/octopeg.ch8",
    "/Users/dmitry/Downloads/snake.ch8",
    "/Users/dmitry/Downloads/RPS.ch8",
    "/Users/dmitry/Downloads/flightrunner.ch8",
    "/Users/dmitry/Downloads/rockto.ch8",
    "/Users/dmitry/Downloads/spacejam.ch8",
    "/Users/dmitry/Downloads/test_opcode.ch8",
    "/Users/dmitry/Downloads/1-chip8-logo.ch8",
    "/Users/dmitry/Downloads/2-ibm-logo.ch8",
    "/Users/dmitry/Downloads/3-corax+.ch8",
    "/Users/dmitry/Downloads/4-flags.ch8",
    "/Users/dmitry/Downloads/4-flags (1).ch8",
    "/Users/dmitry/Downloads/5-quirks.ch8",
    "/Users/dmitry/Downloads/6-keypad.ch8",
    "/Users/dmitry/Downloads/8-scrolling.ch8",
    "/Users/dmitry/Downloads/random_number_test.ch8"};

int keyMap[16] = {
    KEY_X,     // 0
    KEY_ONE,   // 1
    KEY_TWO,   // 2
    KEY_THREE, // 3
    KEY_Q,     // 4
    KEY_W,     // 5
    KEY_E,     // 6
    KEY_A,     // 7
    KEY_S,     // 8
    KEY_D,     // 9
    KEY_Z,     // A
    KEY_C,     // B
    KEY_FOUR,  // C
    KEY_R,     // D
    KEY_F,     // E
    KEY_V      // F
};

void loadRom(chipate::Chip8& chip8, int idx)
{
    if (idx < 0 || idx >= ROMS.size()) {
        loge("Invalid ROM index: %d", idx);
        return;
    }

    std::string romPath = romPaths[idx];
    chip8.init(loadRom(romPath), chipate::Quirks{});
}

void drawDisplay(chipate::Chip8& chip8, size_t x, size_t y, size_t width, size_t height)
{
    size_t vscale = width / (chip8.hiRes() ? 128 : 64);
    size_t hscale = height / (chip8.hiRes() ? 64 : 32);

    for (int i = 0; i < 16; ++i)
        chip8.setKey(i, IsKeyDown(keyMap[i]));

    auto const fb = chip8.fb();
    if (chip8.hiRes()) {
        for (int col = 0; col < 128; ++col) {
            for (int row = 0; row < 64; ++row)
                if (fb[col][row])
                    DrawRectangle(col * vscale + x, row * hscale + y, vscale, hscale, BLACK);
        }
    }
    else {
        for (int col = 0; col < 64; ++col) {
            for (int row = 0; row < 32; ++row)
                if (fb[col][row])
                    DrawRectangle(col * vscale + x, row * hscale + y, vscale, hscale, BLACK);
        }
    }
}

int main()
{

    logi("Initializing...");

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "chipate");
    SetTargetFPS(60);
    int tickRate = 10;

    chipate::Chip8 chip8;

    chipate::Quirks chip_8 = {.shiftVxOnly = false,
                              .loadStoreIAdd = false,
                              .jumpWithVx = false,
                              .logicNoVF = false,
                              .spriteWrap = false,
                              .legacySchipScroll = false};

    chipate::Quirks schip_1_0 = {.shiftVxOnly = true,
                                 .loadStoreIAdd = true,
                                 .jumpWithVx = true,
                                 .logicNoVF = true,
                                 .spriteWrap = true,
                                 .legacySchipScroll = true};

    chipate::Quirks schip_modern = {.shiftVxOnly = true,
                                    .loadStoreIAdd = true,
                                    .jumpWithVx = true,
                                    .logicNoVF = true,
                                    .spriteWrap = true,
                                    .legacySchipScroll = false};

    // Quirks preset selector
    int quirkPreset = 1; // 0 = CHIP-8, 1 = SCHIP 1.0, 2 = SCHIP Modern
    chipate::Quirks* currentQuirks = &schip_1_0;
    bool romLoaded = false;

    chip8.init(loadRom("/Users/dmitry/Downloads/Maze (alt) [David Winter, 199x].ch8"),
               *currentQuirks);
    romLoaded = true;

    bool quirkSelectorEditMode = false;
    int quirkSelectorActive = 0;

    bool spinnerEditMode = false;

    // ROM selector
    int romsScrollIndex = 0;
    int romsActive = 2;
    int romsFocus = -1;

    GuiLoadStyleDark();
    while (!WindowShouldClose()) {
        if (romLoaded) {
            chip8.tock();
            for (int i = 0; i < tickRate; ++i)
                chip8.tick();
        }

        if (IsFileDropped()) {
            int count = 0;
            auto droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".ch8")) {
                chip8.init(loadRom(droppedFiles.paths[0]), *currentQuirks);
                romLoaded = true;
            }
            UnloadDroppedFiles(droppedFiles);
        }

        BeginDrawing();

        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        if (quirkSelectorEditMode)
            GuiLock();

        GuiLabel((Rectangle){15, 10, 150, 20}, "Machine:");

        GuiLabel((Rectangle){15, 65, 150, 20}, "Tick rate:");

        if (GuiSpinner((Rectangle){15, 90, 150, 20}, nullptr, &tickRate, 1, 1000, spinnerEditMode))
            spinnerEditMode = !spinnerEditMode;

        GuiListViewEx((Rectangle){175, 10, 320, 110}, ROMS.data(), ROMS.size(), &romsScrollIndex,
                      &romsActive, &romsFocus);
        if (GuiButton((Rectangle){175, 130, 320, 20}, "LOAD")) {
            if (romsActive >= 0 && romsActive < ROMS.size()) {
                std::string romPath = romPaths[romsActive];
                chip8.init(loadRom(romPath), *currentQuirks);
                romLoaded = true;
            }
        }

        int displayWidth = (WINDOW_WIDTH - 30) / 128 * 128;
        int displayHeight = ((WINDOW_HEIGHT - 170 - 20) / 128) * 128;
        int displayX = 15;
        int displayY = 170;

        DrawRectangle(displayX - 1, displayY - 1, displayWidth + 2, displayHeight + 2, BLACK);
        DrawRectangle(displayX, displayY, displayWidth, displayHeight, LIGHTGRAY);
        drawDisplay(chip8, displayX, displayY, displayWidth, displayHeight);

        int prevPreset = quirkSelectorActive;
        if (GuiDropdownBox((Rectangle){15, 35, 150, 20}, "CHIP-8;SCHIP 1.0;SCHIP Modern",
                           &quirkSelectorActive, quirkSelectorEditMode))
            quirkSelectorEditMode = !quirkSelectorEditMode;
        if (prevPreset != quirkSelectorActive) {
            switch (quirkSelectorActive) {
            case 0:
                currentQuirks = &chip_8;
                break;
            case 1:
                currentQuirks = &schip_1_0;
                break;
            case 2:
                currentQuirks = &schip_modern;
                break;
            }
            if (romLoaded)
                chip8.setQuirks(*currentQuirks);
        }

        GuiUnlock();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
