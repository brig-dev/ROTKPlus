// ROTKPlus - each player's own controller, and the HD packs: two sections of the ROTKPlus tab.
//
// Controllers. A controller is told by a key: "S" + FNV-1a 64 of its serial number (DualSense, DualShock 4, Switch Pro),
// or "P" + that of its device path when it has none (the same as long as it stays on the same port or pairing). ROTK
// Fix makes the same key for players 1 and 2 ([Gamepad] Player1, Player2 of ROTKFix.ini; it leaves alone the controllers
// of players 3 and 4, Player3 and Player4) and ROTKPlus for players 3 and 4 ([Coop] PadKey3, PadKey4 of ROTKPlus.ini).
// A player without one gets the first free controller nobody chose, as before.
//
// HD packs. ROTK Fix reads every folder under ROTKFix\textures\replace; a pack turned off here is moved to
// ROTKFix\textures\replace_off (outside what it reads) and back.
#include "app.h"

static const ImU32 k_gem[4] = { IM_COL32(232, 140, 40, 255), IM_COL32(80, 120, 235, 255), IM_COL32(40, 170, 90, 255),
                                IM_COL32(240, 200, 60, 255) };   // the rings of the logo: amber, sapphire, emerald, gold

// a controller's name as shown: under Steam Input a controller can be named by one of Steam's own translation keys,
// "#SettingsController_SteamController" under SteamOS (owner's screenshot, 12 Nov): the words after the last
// '_' (or the '#'), apart where a capital follows a small letter, "Steam Controller"
std::string pad_shown_name(const char *n)
{
    if (!n || !n[0]) return "?";
    if (n[0] != '#') return n;
    const char *w = strrchr(n, '_');
    w = w && w[1] ? w + 1 : n + 1;
    std::string s;
    for (const char *c = w; *c; c++) {
        if (c > w && isupper((unsigned char)*c) && islower((unsigned char)c[-1])) s += ' ';
        s += *c;
    }
    return s.empty() ? std::string("Controller") : s;
}

static std::string key_of(SDL_Gamepad *pad)
{
    if (!pad) return "";
    const char *serial = SDL_GetGamepadSerial(pad), *path = SDL_GetGamepadPath(pad);
    const char *src = serial && serial[0] ? serial : path && path[0] ? path : nullptr;
    if (!src) return "";
    unsigned long long h = 1469598103934665603ull;
    for (const char *c = src; *c; c++) { h ^= (unsigned char)*c; h *= 1099511628211ull; }
    char k[32];
    snprintf(k, sizeof k, "%s%016llx", serial && serial[0] ? "S" : "P", h);
    return k;
}

static bool pad_busy(SDL_Gamepad *pad)   // a button down or a stick pushed
{
    for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; b++)
        if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)b)) return true;
    for (int x : { SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY })
        if (abs(SDL_GetGamepadAxis(pad, (SDL_GamepadAxis)x)) > 20000) return true;
    return false;
}

static std::string fix_ini_of(const App &a) { return a.dir + "\\ROTKFix.ini"; }
static std::string plus_ini_of(const App &a) { return a.dir + "\\ROTKPlus.ini"; }

static std::string player_key(const App &a, int p)
{
    char k[16], v[40] = "";
    snprintf(k, sizeof k, "Player%d", p + 1);
    GetPrivateProfileStringA("Gamepad", k, "", v, sizeof v, fix_ini_of(a).c_str());
    return v;
}

static void set_player_key(const App &a, int p, const std::string &key)
{
    // one controller, one player: the same controller is taken away from whoever had it
    for (int q = 0; q < 4; q++)
        if (q != p && !key.empty() && !_stricmp(player_key(a, q).c_str(), key.c_str())) set_player_key(a, q, "");
    char k[16];
    snprintf(k, sizeof k, "Player%d", p + 1);
    WritePrivateProfileStringA("Gamepad", k, key.empty() ? nullptr : key.c_str(), fix_ini_of(a).c_str());
    if (p >= 2) {
        snprintf(k, sizeof k, "PadKey%d", p + 1);
        WritePrivateProfileStringA("Coop", k, key.empty() ? nullptr : key.c_str(), plus_ini_of(a).c_str());
    }
}

static void gem(ImVec2 c, float r, ImU32 col)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 d[4] = { ImVec2(c.x - r, c.y), ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r) };
    dl->AddConvexPolyFilled(d, 4, col);
    dl->AddPolyline(d, 4, IM_COL32(230, 236, 248, 255), S(1.2f), ImDrawFlags_Closed);
}

static int    g_waitFor = -1;     // the player waiting for a button press, -1 none
static double g_waitSince;
static bool   g_waitReleased;     // every controller let go since the wait started (the press that clicked doesn't count)

void rotkplus_pads_section(App &a)
{
    section("Controllers of the players");
    if (a.pads.empty()) {
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(C_DIM, "No controller connected. Connect them here to choose whose each one is.");
        ImGui::PopTextWrapPos();
    }
    // the connected controllers: name, whose it is, and a light while it is being used
    std::vector<std::string> keys;
    for (SDL_Gamepad *g : a.pads) keys.push_back(key_of(g));
    for (size_t i = 0; i < a.pads.size(); i++) {
        const std::string name = pad_shown_name(SDL_GetGamepadName(a.pads[i]));
        int whose = -1;
        for (int p = 0; p < 4; p++) if (!keys[i].empty() && !_stricmp(player_key(a, p).c_str(), keys[i].c_str())) whose = p;
        ImVec2 c = ImGui::GetCursorScreenPos() + ImVec2(S(8), ImGui::GetTextLineHeight() * 0.5f + S(2));
        ImU32 col = whose >= 0 ? k_gem[whose] : IM_COL32(90, 100, 120, 255);
        if (pad_busy(a.pads[i])) ImGui::GetWindowDrawList()->AddCircleFilled(c, S(9), IM_COL32(214, 226, 246, 120), 16);
        gem(c, S(6), col);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + S(22));
        ImGui::TextUnformatted(name.c_str());
        ImGui::SameLine();
        if (whose >= 0) ImGui::TextColored(C_DIM, "%s %d", tr("player"), whose + 1);
        else ImGui::TextColored(C_DIM, "%s", tr("nobody's (goes to the first free player)"));
    }
    if (keys.size() != a.pads.size()) return;
    ImGui::Spacing();
    // waiting for a press: the first controller that presses a button after all were let go is that player's
    if (g_waitFor >= 0) {
        bool any = false;
        int pressed = -1;
        for (size_t i = 0; i < a.pads.size(); i++)
            if (pad_busy(a.pads[i])) { any = true; if (pressed < 0) pressed = (int)i; }
        if (!any) g_waitReleased = true;
        else if (g_waitReleased && pressed >= 0 && !keys[pressed].empty()) {
            set_player_key(a, g_waitFor, keys[pressed]);
            g_waitFor = -1;
        }
        if (ImGui::GetTime() - g_waitSince > 15) g_waitFor = -1;
    }
    for (int p = 0; p < 4; p++) {
        ImGui::PushID(p);
        const std::string k = player_key(a, p);
        int connected = -1;
        for (size_t i = 0; i < keys.size(); i++) if (!k.empty() && !_stricmp(keys[i].c_str(), k.c_str())) connected = (int)i;
        char label[32];
        snprintf(label, sizeof label, "%s %d", tr("Player"), p + 1);
        row_begin(label, p < 2 ? "Players 1 and 2: ROTK Fix opens their controllers. Automatic: the first free ones."
                               : "Players 3 and 4 (ROTKPlus). Automatic: the first free controllers nobody chose.");
        gem(ImGui::GetCursorScreenPos() + ImVec2(-S(14), ImGui::GetFrameHeight() * 0.5f), S(5), k_gem[p]);
        std::string shown = g_waitFor == p ? tr("Press a button on its controller...")
                          : k.empty() ? tr("Automatic")
                          : connected >= 0 ? pad_shown_name(SDL_GetGamepadName(a.pads[connected]))
                                           : std::string(tr("its controller (not connected)"));
        const char *btn = g_waitFor == p ? tr("Cancel") : tr("Press to choose");
        const float bw = ImGui::CalcTextSize(btn).x + ImGui::GetStyle().FramePadding.x * 2;
        ImGui::SetNextItemWidth(std::max(S(120), std::min(S(260), ImGui::GetContentRegionAvail().x - bw - S(10))));
        if (ImGui::BeginCombo("##pad", shown.c_str())) {
            if (ImGui::Selectable(tr("Automatic"), k.empty())) set_player_key(a, p, "");
            for (size_t i = 0; i < a.pads.size(); i++) {
                if (keys[i].empty()) continue;
                const std::string n = pad_shown_name(SDL_GetGamepadName(a.pads[i]));
                ImGui::PushID((int)i);
                if (ImGui::Selectable(n.c_str(), connected == (int)i)) set_player_key(a, p, keys[i]);
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button(btn)) {
            if (g_waitFor == p) g_waitFor = -1;
            else { g_waitFor = p; g_waitSince = ImGui::GetTime(); g_waitReleased = false; }
        }
        row_end();
        ImGui::PopID();
    }
    ImGui::PushTextWrapPos(0);
    ImGui::TextColored(C_DIM, "Press to choose, then press any button on that player's controller. A controller is "
                              "known again the next time, also on another USB port when it has a serial number "
                              "(DualSense, DualShock 4, Switch Pro); others are known by their port.");
    ImGui::PopTextWrapPos();
}

// ---- the HD packs

struct Pack { std::string name; bool on; unsigned long long bytes; int files; };
static std::vector<Pack> g_packs;
static std::string g_packsFor;

static void folder_size(const std::string &dir, unsigned long long &bytes, int &files)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..")) continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) folder_size(dir + "\\" + fd.cFileName, bytes, files);
        else { bytes += ((unsigned long long)fd.nFileSizeHigh << 32) | fd.nFileSizeLow; files++; }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

static void list_packs(const App &a)
{
    g_packs.clear();
    g_packsFor = a.dir;
    for (int on = 1; on >= 0; on--) {
        std::string base = a.dir + (on ? "\\ROTKFix\\textures\\replace" : "\\ROTKFix\\textures\\replace_off");
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((base + "\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) continue;
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == '.') continue;
            Pack p{ fd.cFileName, on != 0, 0, 0 };
            folder_size(base + "\\" + fd.cFileName, p.bytes, p.files);
            g_packs.push_back(p);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
}

void rotkplus_packs_section(App &a)
{
    section("HD packs");
    if (g_packsFor != a.dir) list_packs(a);
    ImGui::PushTextWrapPos(0);
    ImGui::TextColored(C_DIM, "The folders of ROTKFix\\textures\\replace in the game folder (textures, portraits, fonts). "
                              "Turned off, a folder is moved to replace_off and the game's own pictures are used.");
    ImGui::PopTextWrapPos();
    if (g_packs.empty()) ImGui::TextColored(C_DIM, "%s", tr("No HD pack in the game folder."));
    for (size_t i = 0; i < g_packs.size(); i++) {
        Pack &p = g_packs[i];
        ImGui::PushID((int)i);
        char desc[96];
        snprintf(desc, sizeof desc, "%d %s, %.0f MB", p.files, tr("files"), p.bytes / 1048576.0);
        // the folders of our packages by their contents, any other by its own name
        static const struct { const char *dir, *name; } known[] = {
            { "esrgan", "HD textures" }, { "gc_colori", "Colours of the GameCube version" }, { "ritratti_gc", "HD portraits" },
            { "font_hd", "HD fonts" }, { "font_prova72", "HD fonts" } };
        const char *shownName = p.name.c_str();
        for (auto &k : known) if (!_stricmp(k.dir, p.name.c_str())) shownName = tr(k.name);
        row_begin(shownName, nullptr);
        bool on = p.on;
        if (ImGui::Checkbox("##pack", &on)) {
            std::string tex = a.dir + "\\ROTKFix\\textures\\";
            std::string from = tex + (p.on ? "replace\\" : "replace_off\\") + p.name, to = tex + (on ? "replace\\" : "replace_off\\") + p.name;
            CreateDirectoryA((tex + (on ? "replace" : "replace_off")).c_str(), NULL);
            if (MoveFileExA(from.c_str(), to.c_str(), 0)) p.on = on;
            else set_message(a, std::string(tr("The pack couldn't be moved (is the game running?): ")) + p.name, true);
        }
        ImGui::SameLine();
        ImGui::TextColored(C_DIM, "%s", desc);
        row_end();
        ImGui::PopID();
    }
    if (ImGui::Button(tr("Look again"))) list_packs(a);
}

// The front screen's "who plays": each player's controller in a line (front_who_plays, fellowship.inc)
void rotkplus_pad_lines(App &a, std::vector<std::string> &lines)
{
    // what each player will play with: the controller chosen for them; otherwise, as ROTK Fix and ROTKPlus hand them
    // out, the connected controllers nobody chose, in their order; with none left, player 1 has the keyboard and mouse
    std::vector<std::string> keys;
    for (SDL_Gamepad *g : a.pads) keys.push_back(key_of(g));
    std::vector<bool> taken(keys.size(), false);
    std::string chosen[4];
    for (int p = 0; p < 4; p++) chosen[p] = player_key(a, p);
    for (int p = 0; p < 4; p++)
        for (size_t i = 0; i < keys.size(); i++)
            if (!chosen[p].empty() && !_stricmp(keys[i].c_str(), chosen[p].c_str())) taken[i] = true;
    auto name_of = [&](size_t i) { return pad_shown_name(SDL_GetGamepadName(a.pads[i])); };
    for (int p = 0; p < 4; p++) {
        std::string what;
        if (!chosen[p].empty()) {
            for (size_t i = 0; i < keys.size() && what.empty(); i++)
                if (!_stricmp(keys[i].c_str(), chosen[p].c_str())) what = name_of(i);
            if (what.empty()) what = tr("its controller isn't connected");
        } else {
            for (size_t i = 0; i < keys.size() && what.empty(); i++)
                if (!taken[i]) { taken[i] = true; what = name_of(i); }
            if (what.empty()) what = p == 0 ? tr("keyboard and mouse") : "\xe2\x80\x94";
        }
        lines.push_back(what);
    }
}
