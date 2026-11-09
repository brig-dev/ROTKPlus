// ROTKPlus - the ROTKPlus tab, in place of Athelas' Mods tab: ROTKPlus is the one mod this launcher carries.
// Its settings live in ROTKPlus.ini in the game folder (next to rotk.exe), the same file the game reads and the
// Multiplayer page writes; each change here is written there at once. The mod itself is ROTKFix\mods\ROTKPlus.dll,
// turned on and off by Enabled in ROTKFix\mods\ROTKPlus.ini ([Mod]).
#include "app.h"

static std::string plus_ini(const App &a) { return a.dir + "\\ROTKPlus.ini"; }
static std::string mod_ini(const App &a) { return a.dir + "\\ROTKFix\\mods\\ROTKPlus.ini"; }
static bool file_there(const std::string &p) { return GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
static std::string fix_ini(const App &a) { return a.dir + "\\ROTKFix.ini"; }

// On or off as ROTK Fix sees it: ROTKFix.ini's [Mods] ROTKPlus.dll first (Athelas' Mods tab wrote it there, 0 when the
// mod was turned off), then the mod's own Enabled (rotkfix.c, the mods' loader)
static bool plus_on(const App &a)
{
    char v[8] = "";
    if (GetPrivateProfileStringA("Mods", "ROTKPlus.dll", "", v, sizeof v, fix_ini(a).c_str()) > 0) return atoi(v) != 0;
    return GetPrivateProfileIntA("Mod", "Enabled", 1, mod_ini(a).c_str()) != 0;
}
static void plus_set(const App &a, bool on)
{
    WritePrivateProfileStringA("Mods", "ROTKPlus.dll", on ? "1" : "0", fix_ini(a).c_str());
    WritePrivateProfileStringA("Mod", "Enabled", on ? "1" : "0", mod_ini(a).c_str());
}

// The first Play from this launcher turns ROTKPlus on (a game that had Athelas may have it off in ROTKFix.ini); after
// that it is the ROTKPlus tab's choice. Remembered in the launcher's own settings ([ROTKPlus] TurnedOn).
void rotkplus_first_play(App &a)
{
    const std::string own = ports_ini();
    if (GetPrivateProfileIntA("ROTKPlus", "TurnedOn", 0, own.c_str())) return;
    if (!file_there(a.dir + "\\ROTKFix\\mods\\ROTKPlus.dll")) return;
    plus_set(a, true);
    WritePrivateProfileStringA("ROTKPlus", "TurnedOn", "1", own.c_str());
}

static int get_int(const App &a, const char *sec, const char *key, int def)
{
    return (int)GetPrivateProfileIntA(sec, key, def, plus_ini(a).c_str());
}
static void set_int(const App &a, const char *sec, const char *key, int v)
{
    WritePrivateProfileStringA(sec, key, std::to_string(v).c_str(), plus_ini(a).c_str());
}

// a factor in tenths (10 as in the game ... 50), with what -1 and 0 mean for that key
static void factor_row(const App &a, const char *label, const char *desc, const char *key, int def, const char *special,
                       int specialValue)
{
    // as in the game, x1.5, x2, x3, x5, x7: the same steps as the game's pages (09/11/2026, the halves in between made no
    // difference to see); a value saved before (x2.5, x4...) is shown as it is until another one is chosen
    static const int k[] = { 10, 15, 20, 30, 50, 70 };
    int v = get_int(a, "Coop", key, def);
    auto name = [&](int t) {
        if (t == specialValue && special) return std::string(special);
        if (t == 10) return std::string("As in the game");
        char b[16];
        snprintf(b, sizeof b, t % 10 ? "x%d.%d" : "x%d", t / 10, t % 10);
        return std::string(b);
    };
    row_begin(label, desc);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo((std::string("##") + key).c_str(), name(v).c_str())) {
        if (special && ImGui::Selectable(special, v == specialValue)) set_int(a, "Coop", key, specialValue);
        for (int t : k)
            if (ImGui::Selectable(name(t).c_str(), v == t)) set_int(a, "Coop", key, t);
        ImGui::EndCombo();
    }
    // the enemies at the highest factor: Gimli's line in the film, before the march on the Black Gate (in English in every
    // language but Italian, as in the game's pages: the other dubs' words could not be checked)
    if (strstr(key, "EnemyFactor") && v >= 70) {
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(C_GOLD, "%s", tr("\"Certainty of death. Small chance of success. What are we waiting for?\""));
        ImGui::PopTextWrapPos();
    }
    row_end();
}

// a key that is on (onValue) or off (0); FastText is on at 2 (1 = only ROTKPlus' own texts)
static void check_row(const App &a, const char *label, const char *desc, const char *sec, const char *key, int def,
                      int onValue = 1, int offValue = 0)
{
    bool on = get_int(a, sec, key, def) == onValue;
    row_begin(label, desc);
    if (ImGui::Checkbox((std::string("##") + sec + key).c_str(), &on)) set_int(a, sec, key, on ? onValue : offValue);
    row_end();
}

void tab_rotkplus(App &a)
{
    section("ROTKPlus");
    const bool dll = file_there(a.dir + "\\ROTKFix\\mods\\ROTKPlus.dll");
    char ver[64] = "";
    GetPrivateProfileStringA("Mod", "Version", "", ver, sizeof ver, mod_ini(a).c_str());
    bool on = plus_on(a);
    std::string st = dll ? std::string("ROTKPlus ") + ver : "ROTKPlus isn't in the game folder yet: press Play once";
    status_dot(dll && on, dll, st.c_str(), nullptr);
    if (dll) {
        row_begin("ROTKPlus on", "Co-op with 3 and 4 players, the Versus mode, more enemies and allies, the players' "
                                 "names and statistics (the Multiplayer and Fellowship entries of the main menu). "
                                 "Off, the game is as ROTK Fix leaves it.");
        if (ImGui::Checkbox("##plus_on", &on)) plus_set(a, on);
        row_end();
    }
    if (file_there(a.dir + "\\DivxMediaLib_orig.dll")) {
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(C_WARN, "ROTKPlus is also installed the old way, as DivxMediaLib.dll in the game folder: only one "
                                   "of the two works. To keep this one, delete DivxMediaLib.dll and rename "
                                   "DivxMediaLib_orig.dll to DivxMediaLib.dll (with the game closed).");
        ImGui::PopTextWrapPos();
    }

    section("Campaign");
    factor_row(a, "More enemies (co-op)", "The levels' common enemies (soldiers, shields and archers) come out of their "
               "spawners this many times, and the levels count them as their own. Captains, trolls, mumakil, spiders and "
               "bosses stay as they are. Also chosen on the Multiplayer page.", "EnemyFactor", -1,
               "Recommended (x1.5 with 3 players, x2 with 4)", -1);
    factor_row(a, "More allies (co-op)", "The soldiers of Gondor and Rohan on your side.", "AllyFactor", 0,
               "As many more as the enemies", 0);
    factor_row(a, "More enemies (single player)", "The same for a Single game (also chosen on Who is playing?).",
               "SingleEnemyFactor", 10, nullptr, 0);
    factor_row(a, "More allies (single player)", "The soldiers on your side in a Single game.", "SingleAllyFactor", 0,
               "As many more as the enemies", 0);
    ImGui::PushTextWrapPos(0);
    ImGui::TextColored(C_DIM, "At the Top of the Wall the level is lost when too many orcs are on the wall: with many more "
                              "enemies that comes soon.");
    ImGui::PopTextWrapPos();
    check_row(a, "Bring back stuck players", "With 3 or 4 players, whoever keeps running in place for a second and a "
              "half, or stays far from players 1 and 2 for a few seconds, is brought next to a teammate. Players 3 and 4 "
              "can also press Back to come back next to player 1 or 2 at any time.", "Coop", "Unstick", 1);

    section("Versus");
    check_row(a, "Heavy blows go through a parry", "Between players a parry stops speed attacks, plain arrows and "
              "spears, but not fierce attacks, the last move of a combo or a fully drawn shot.", "PvP",
              "HeavyUnblockable", 1);
    check_row(a, "Only the arenas on the map", "The map offers only the three arenas (the Southern Gate, the Courtyard "
              "of Minas Tirith, the Black Gate).", "PvP", "ArenasOnly", 1);

    section("Other");
    check_row(a, "All texts drawn in one go", "Every text of the game drawn in one go instead of a letter at a time "
              "(smoother menus).", "General", "FastText", 2, 2, 1);
    check_row(a, "Detailed log", "A complete trace of every level in ROTKPlus_logs, for reporting problems.", "Log",
              "Detail", 0);
    ImGui::Spacing();
    if (ImGui::Button("Open ROTKPlus.ini"))
        ShellExecuteA(NULL, "open", plus_ini(a).c_str(), NULL, NULL, SW_SHOWNORMAL);
    ImGui::SameLine();
    if (ImGui::Button("Open the logs"))
        ShellExecuteA(NULL, "open", (a.dir + "\\ROTKPlus_logs").c_str(), NULL, NULL, SW_SHOWNORMAL);
    ImGui::SameLine();
    if (ImGui::Button("ROTKPlus on GitHub"))
        ShellExecuteA(NULL, "open", "https://github.com/brig-dev/ROTKPlus", NULL, NULL, SW_SHOWNORMAL);
}

// The players' controllers open the Controls tab, the HD packs close the Graphics tab
void tab_controls_plus(App &a) { rotkplus_pads_section(a); tab_controls(a); }
void tab_graphics_plus(App &a) { tab_graphics(a); rotkplus_packs_section(a); }
// one tab for what changes the play: ROTKPlus' (more enemies, versus...) first, then ROTK Fix's
void tab_gameplay_plus(App &a) { tab_rotkplus(a); ImGui::Dummy(ImVec2(0, S(8))); tab_gameplay(a); }
