// SPDX-License-Identifier: WTFPL

#include "chip8.h"
#include "log.h"

#include <array>
#include <raylib.h>
#include <string>

#define RAYGUI_IMPLEMENTATION
#include <raygui/raygui.h>
#include <raygui/styles/dark/style_dark.h>
#include <nlohmann/json.hpp>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(chip8archive);

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

struct RomInfo {
    std::string name;
    std::string path;
    std::string title;
    std::string authors;
    std::string release;
    std::string event;
    std::string platform;
    std::string desc;
};

std::vector<RomInfo> ROMS;

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

void loadRom(chipate::Chip8& chip8, RomInfo const& rom)
{
    auto fs = cmrc::chip8archive::get_filesystem();
    auto file = fs.open(rom.path);
    chip8.init(std::vector<uint8_t>(file.begin(), file.end()), chipate::Quirks{});
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

    logi("Loading resources...");
    logi("Loading Chip-8 archive...");
    auto fs = cmrc::chip8archive::get_filesystem();
    auto file = fs.open("programs.json");
    std::vector<const char*> romNameSelectorData;


    nlohmann::json programsJson;
    try {
        programsJson = nlohmann::json::parse(file);
        for (auto& [program, info] : programsJson.items()) {

            if (info["platform"] != "chip8" && info["platform"] != "schip") {
                logi("Skipping non-Chip-8 program: %s", program.c_str());
                continue;
            }
            std::string name = program;
            std::string title = info["title"];
            std::string desc = info["desc"];
            std::string release = info["release"];
            std::string event = info.contains("event") ? info["event"] : "";
            std::string platform = info["platform"];
            std::string authors;
            for (auto& author : info["authors"]) {
                if (!authors.empty())
                    authors += ", ";
                authors += author.get<std::string>();
            }
            logi("Found program: %s %s (%s)", name.c_str(), title.c_str(), desc.c_str());

            ROMS.push_back({
                .name = name,
                .path = "roms/" + name + ".ch8",
                .title = title,
                .authors = authors,
                .release = release,
                .event = event,
                .platform = platform,
                .desc = desc
            });

        }
    }
    catch (nlohmann::json::parse_error& e) {
        loge("Failed to parse programs.json: %s", e.what());
        return -1;
    }

    for (const auto& rom : ROMS) {
        romNameSelectorData.push_back(rom.title.c_str());
    }

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
            for (int i = 0; i < tickRate; ++i) {
                chip8.tick();
            }

            chip8.tock();
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

        GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

        GuiLabel({15, 10, 150, 20}, "Machine:");

        GuiLabel({15, 65, 150, 20}, "Tick rate:");

        if (GuiSpinner({15, 90, 150, 20}, nullptr, &tickRate, 1, 100000, spinnerEditMode))
            spinnerEditMode = !spinnerEditMode;

        GuiSetStyle(LISTVIEW, LIST_ITEMS_SPACING, 3);
        GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 17);
        GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
        GuiListViewEx({175, 10, 320, 110}, romNameSelectorData.data(), romNameSelectorData.size(), &romsScrollIndex,
                      &romsActive, &romsFocus);

        romsActive = std::clamp(romsActive, 0, static_cast<int>(ROMS.size()) - 1);
        const auto& selectedRom = ROMS[romsActive];

        if (GuiButton({175, 130, 320, 20}, "LOAD")) {
            if (romsActive >= 0 && romsActive < ROMS.size()) {
                const auto& rom = ROMS[romsActive];
                loadRom(chip8,rom);
                romLoaded = true;
            }
        }


        GuiLabel( {500, 10, 80, 20}, "Author: ");
        GuiLabel( {580, 10, 200, 20}, selectedRom.authors.c_str());

        GuiLabel( {500, 35, 80, 20}, "Release: ");
        GuiLabel( {580, 35, 200, 20}, selectedRom.release.c_str());

        GuiLabel( {500, 60, 80, 20}, "Event: ");
        GuiLabel( {580, 60, 200, 20}, selectedRom.event.c_str());

        GuiLabel( {500, 85, 80, 20}, "Platform: ");
        GuiLabel( {580, 85, 200, 20}, selectedRom.platform.c_str());

        auto prevWrapMode = GuiGetStyle(DEFAULT, TEXT_WRAP_MODE);
        auto prevAlignment = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
        auto prevLineSpacing = GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);
        GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, TEXT_WRAP_WORD);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_TOP);
        GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, 17);
        GuiLabel({500, 110, 280, 70}, selectedRom.desc.c_str());

        GuiSetStyle(DEFAULT, TEXT_WRAP_MODE, prevWrapMode);
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, prevAlignment);
        GuiSetStyle(DEFAULT, TEXT_LINE_SPACING, prevLineSpacing);

        int displayWidth = (WINDOW_WIDTH - 30) / 128 * 128;
        int displayHeight = ((WINDOW_HEIGHT - 170 - 20) / 128) * 128;
        int displayX = 15;
        int displayY = 170;

        DrawRectangle(displayX - 1, displayY - 1, displayWidth + 2, displayHeight + 2, BLACK);
        DrawRectangle(displayX, displayY, displayWidth, displayHeight, LIGHTGRAY);
        drawDisplay(chip8, displayX, displayY, displayWidth, displayHeight);

        int prevPreset = quirkSelectorActive;
        if (GuiDropdownBox({15, 35, 150, 20}, "CHIP-8;SCHIP 1.0;SCHIP Modern",
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
