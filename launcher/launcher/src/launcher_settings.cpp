// Athelas - the launcher's own settings: its window and the games it knows.
#include "app.h"
#include "update.h"

// Athelas' own window (as COGBox has it): fullscreen with F11 or LT + RT together, from any screen, a borderless window
// with its own buttons; its size and choices are remembered. Not the games' display settings.
bool launcher_ini_get(const char *k, int def) { return GetPrivateProfileIntA("Launcher", k, def, ports_ini().c_str()) != 0; }

void launcher_ini_put(const char *k, int v) { WritePrivateProfileStringA("Launcher", k, std::to_string(v).c_str(), ports_ini().c_str()); }

bool g_borderless;
bool g_animations = true;

SDL_Window *g_mainWin;   // (for the maker's mark: under the borderless window's own buttons)

void set_fullscreen(App &a, bool on)
{
    SDL_SetWindowFullscreen(a.win, on);
    launcher_ini_put("Fullscreen", on);
}

static void set_borderless(App &a, bool on)
{
    g_borderless = on;
    SDL_SetWindowBordered(a.win, !on);
    launcher_ini_put("Borderless", on);
}

static void window_section(App &a)
{
    row_begin("Language", "ROTKPlus' own language. The game's language is on its Gameplay tab.");
    language_combo();
    row_end();
    row_begin("Fullscreen", "The launcher fills the screen. F11, or LT + RT together, switches it too.");
    bool fs = (SDL_GetWindowFlags(a.win) & SDL_WINDOW_FULLSCREEN) != 0;
    if (ImGui::Checkbox("##fullscreen", &fs)) set_fullscreen(a, fs);
    row_end();
    row_begin("Borderless window", "ROTKPlus draws its own minimise, maximise and close buttons in the top-right corner; "
                                   "drag the strip beside them to move the window.");
    bool bl = g_borderless;
    if (ImGui::Checkbox("##borderless", &bl)) set_borderless(a, bl);
    row_end();
    row_begin("Updates", "At start, GitHub is asked for the newest ROTKPlus. Only the release page's number is asked for; "
                         "nothing about you or this PC is sent.");
    bool on = updates_on();
    if (ImGui::Checkbox("Check at start##upd", &on)) {
        WritePrivateProfileStringA("Updates", "Check", on ? "1" : "0", ports_ini().c_str());
        if (on) update_check_start(LAUNCHER_VERSION);
    }
    row_end();
}

// Show / Remove from the list wait for the next frame (games_apply_pending, at the top of draw_ui): the page drawing
// now holds a reference to the shown game (review: removing the last one left it pointing past the list)
static int g_showNext = -1, g_removeNext = -1;
void games_apply_pending(App &a)
{
    if (g_showNext >= 0 && g_showNext < (int)a.lib.size()) a.cur = g_showNext;
    if (g_removeNext >= 0 && g_removeNext < (int)a.lib.size()) {
        int remove = g_removeNext;
        a.lib.erase(a.lib.begin() + remove);
        if (a.cur == remove) a.cur = a.lib.empty() ? -1 : 0;
        else if (a.cur > remove) a.cur--;
        library_save(exe_dir(), a.lib);
    }
    g_showNext = g_removeNext = -1;
}

void games_section(App &a)
{
    section("Your games");
    for (size_t i = 0; i < a.lib.size(); i++) {
        const LibGame &g = a.lib[i];
        ImGui::PushID((int)i);
        ImGui::Spacing();
        ImGui::TextColored((int)i == a.cur ? C_GOLD_H : C_TEXT, "%s", game_kind_name(g.kind));
        ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
        ImGui::TextColored(C_DIM, "%s%s", g.path.c_str(), g.iso ? "  (disc image)" : "");
        ImGui::PopFont();
        bool isOwn = g.kind == GK_ROTK_PC && !_stricmp(g.path.c_str(), a.dir.c_str());
        if ((int)i != a.cur && ImGui::SmallButton("Show")) { g_showNext = (int)i; if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) ImGui::CloseCurrentPopup(); }
        if (!isOwn) {
            if ((int)i != a.cur) ImGui::SameLine();
            if (ImGui::SmallButton("Remove from list")) { g_removeNext = (int)i; if (ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) ImGui::CloseCurrentPopup(); }
        }
        ImGui::PopID();
    }
    ImGui::Spacing();
    if (ImGui::Button("Add a game folder...")) {
        std::string d;
        if (pick_folder(d, L"Choose the game folder (with rotk.exe)")) add_game_from(a, d);
    }

}

// Athelas' own settings (COGBox's About > SETTINGS): the window, updates, language, shortcuts, background pictures
static void launcher_settings(App &a)
{
    section("Launcher");
    window_section(a);
    row_begin("Background pictures", "Shown behind the launcher, changing every 10 seconds. Game art is read from your own "
                                     "copies; nothing is copied. My pictures: any PNG or JPG in the backgrounds folder.");
    ImGui::SetNextItemWidth(-FLT_MIN);   // the whole row; Folder and Shuffle on the line under it, in the same column
    if (ImGui::BeginCombo("##bgshow", k_bgShowName[g_bgShow])) {
        for (int i = 0; i < BG_SHOW_COUNT; i++)
            if (ImGui::Selectable(k_bgShowName[i], i == g_bgShow) && i != g_bgShow) {
                g_bgShow = i;
                bg_show_save();
                a.bg.group = -1;           // list again on the next frame
            }
        ImGui::EndCombo();
    }
    
    if (ImGui::Button("Folder##bg")) {
        std::string d = exe_dir() + "\\ROTKFix\\backgrounds";
        SHCreateDirectoryExA(NULL, d.c_str(), NULL);
        ShellExecuteA(NULL, "open", d.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Shuffle")) a.bg.group = -1;   // list again on the next frame
    row_end();
    row_begin("Animations", "The background pictures come slowly closer, embers rise on the front page and each page "
                            "fades in. Off: everything stands still.");
    bool an = g_animations;
    if (ImGui::Checkbox("##anim", &an)) { g_animations = an; launcher_ini_put("Animations", an); }
    row_end();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(tr("Shortcuts"));
    shortcuts_section(a);
}

// About, the last part (COGBox's layout): Athelas' own settings, then one credits line; the game list and the
// licences behind buttons (Shagg, 6 Nov: About was too cluttered)
void about_credits(App &a)
{
    launcher_settings(a);
    section("Credits");
    ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
    ImGui::PushTextWrapPos(0);
    ImGui::PushTextWrapPos(0);
    ImGui::TextColored(C_DIM, "ROTKPlus %s  -  %s", LAUNCHER_VERSION, tr("launcher and ROTK Fix by Shagg (Athelas), ROTKPlus by brig-dev, with Claude (Anthropic)  -  a free project by a fan, not affiliated with EA, Warner Bros. or the Tolkien Estate"));
    ImGui::PopTextWrapPos();
    ImGui::PopTextWrapPos();
    ImGui::PopFont();
    ImGui::Spacing();
    if (ImGui::Button("Licences...")) ImGui::OpenPopup("Licences");
    ImGui::SameLine();
    if (ImGui::Button("ROTKPlus on GitHub")) ShellExecuteA(NULL, "open", "https://github.com/brig-dev/ROTKPlus", NULL, NULL, SW_SHOWNORMAL);
    tip("ROTKPlus on GitHub: if you like it, leave a star.");
    ImGui::SameLine();
    if (ImGui::Button("Support ROTKPlus on Ko-fi")) ShellExecuteA(NULL, "open", ROTKPLUS_KOFI, NULL, NULL, SW_SHOWNORMAL);
    tip("brig-dev's Ko-fi page, in your browser. ROTKPlus is and stays free: a coffee is only a thank-you.");

    ImVec2 vp = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize(ImVec2(std::min(S(720), vp.x - S(40)), std::min(S(620), vp.y - S(40))));
    if (ImGui::BeginPopupModal("Your games", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::BeginChild("lo", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - S(6)), ImGuiChildFlags_NavFlattened);
        games_section(a);
        ImGui::EndChild();
        if (ImGui::Button("Close", ImVec2(S(150), 0)) || dialog_back()) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::SetNextWindowSize(ImVec2(std::min(S(640), vp.x - S(40)), 0));
    if (ImGui::BeginPopupModal("Licences", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        static const char *lic[][2] = {
            { "DXVK", "zlib" }, { "SDL3", "zlib" }, { "Dear ImGui", "MIT" }, { "stb_image, stb_image_write", "public domain / MIT" },
            { "DSOAL, OpenAL Soft", "LGPL" }, { "Uncial Antiqua, Cinzel, EB Garamond", "SIL Open Font License" },
            { "Liberation Sans", "SIL Open Font License" }, { "XboxXBE-Recomp", "MIT, (c) 2026 TGP482" },
            { "toml++", "MIT" }, { "miniz", "MIT" },
            { "Athelas and ROTK Fix", "MIT, (c) 2026 Shagg" }, { "ROTKPlus", "MIT, (c) 2026 brig-dev" } };
        for (auto &l : lic) { ImGui::TextColored(C_GOLD, "%s", l[0]); ImGui::SameLine(S(330)); ImGui::TextUnformatted(l[1]); }
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(S(150), 0)) || dialog_back()) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
