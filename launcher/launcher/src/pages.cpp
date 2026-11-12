// Athelas - the frame: the front screen (front.inc), the Journey (journey.inc) and the game pages around them.
#include "app.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshadow"
#include "../third_party/stb_image.h"
#pragma GCC diagnostic pop

// Shown for a game other than this folder's PC game
static void draw_other_game(App &a)
{
    LibGame &g = a.lib[a.cur];
    section(game_kind_name(g.kind));
    row_begin("Location", nullptr);
    ImGui::PushTextWrapPos(0);
    ImGui::TextUnformatted(g.path.c_str());
    ImGui::PopTextWrapPos();
    row_end();
    if (g.kind != GK_ROTK_PC) {
        row_begin("Version", nullptr);
        ImGui::Text("Xbox, %s copy, %s", game_region_name(g), g.iso ? "disc image" : "extracted files");
        if (g.titleId) { ImGui::SameLine(); ImGui::TextColored(C_DIM, "(title ID %08X)", (unsigned)g.titleId); }
        row_end();
        ImGui::Spacing();
        ImGui::PushTextWrapPos(0);
        if (tp_kind(g.kind) && release_untested(k_tomlPorts[tp_of(g.kind)].dir, g)) {
            const char *dir = k_tomlPorts[tp_of(g.kind)].dir;
            if (release_confirmed(dir, g))
                ImGui::TextColored(C_DIM, "An untested version (%s copy) you said works.", game_region_name(g));
            else {
                ImGui::TextColored(C_WARN, "This is an untested and unsupported version (%s copy). Proceed with caution: it "
                                   "may not start, or may have problems.", game_region_name(g));
                ImGui::TextUnformatted("Did it work? Let us know:");
                if (ImGui::Button("It worked")) report_version(a, dir, game_kind_name(g.kind), g, true);
                ImGui::SameLine();
                if (ImGui::Button("It didn't work")) report_version(a, dir, game_kind_name(g.kind), g, false);
            }
            ImGui::Spacing();
        }
        if (tp_kind(g.kind)) { ImGui::PopTextWrapPos(); draw_tt(a, g); return; }
        ImGui::TextColored(C_WARN, "Recognised, but not playable from here yet.");
        ImGui::TextUnformatted("The Xbox Return of the King needs its own PC port, which is still to be built. When it's ready, "
                               "its settings will appear here and Play will start it with your game files.");
        ImGui::PopTextWrapPos();
        return;
    }
    // the PC game in another folder: its own copy of ROTK Fix keeps its own settings there
    if (a.otherFor != g.path) { a.other = inspect_game(g.path); a.otherFor = g.path; }
    const GameInfo &o = a.other;
    row_begin("Version", nullptr);
    ImGui::TextUnformatted(o.versionText.c_str());
    row_end();
    ImGui::Spacing();
    if (fix_installed(o)) {        // (normally switched to straight away in draw_ui)
        if (ImGui::Button("Show its settings")) use_pc_game(a, g.path);
    } else {
        ImGui::PushTextWrapPos(0);
        ImGui::TextUnformatted("ROTK Fix isn't set up for this copy yet. Setting it up puts ROTK Fix's files in the game's folder, "
                               "next to rotk.exe (the game's own files aren't changed). Remove ROTK Fix on the About tab takes them out again.");
        ImGui::PopTextWrapPos();
        if (gold_button("Set up ROTK Fix there", ImVec2(S(240), S(36)), F_BODY)) { setup_here(a, g.path); a.otherFor.clear(); }
        if (!a.setupErr.empty()) { ImGui::PushTextWrapPos(0); ImGui::TextColored(C_BAD, "%s", a.setupErr.c_str()); ImGui::PopTextWrapPos(); }
        admin_setup_prompt(a);
    }
}

// ---------------------------------------------------------------------------
// Main screen

static void front_last_game(App &a, ImVec2 pos, ImVec2 sz);   // fellowship.inc (the front screen's cards)
static void front_who_plays(App &a, ImVec2 pos, ImVec2 sz);
static void gal_forget();   // gallery.inc
static void fs_forget();
static float hi_fs(ImFont *f, float k = 1.0f);   // history.inc: a font's size on screen, and a gem
static void hi_gem(ImDrawList *dl, ImVec2 c, float r, ImU32 col, bool ring = false);
struct App;
static void share_frame(App &a);              // share.inc: the player's card to share
static void share_ask(const App &a, int idx);
static bool clipboard_image(const unsigned char *rgba, int w, int h, const std::vector<unsigned char> &png);
#include "front.inc"

#include "fellowship.inc"

#include "gallery.inc"

#include "history.inc"

#include "share.inc"

// a newer Athelas: a button at the top right (at y), on every page and the front screen, that asks to install it and
// shows the download
// The maker's mark, top right on every screen (as COGBox has one): "Made by Shagg" and, under it, the version of
// Athelas this is - the release's number, so nobody reads a port's own version for it. Returns the y below it.
static float maker_mark(ImVec2 size)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos();
    float right = size.x - S(48), y = S(34);   // (inside the window's frame)
    if (g_borderless && g_mainWin && !(SDL_GetWindowFlags(g_mainWin) & SDL_WINDOW_FULLSCREEN))
        y += S(26);   // (under Athelas' own window buttons)
    // ROTKPlus: brig-dev's project, built on Shagg's Athelas and ROTK Fix (MIT): both named, the version beside them
    const std::string markS = std::string(tr("A project by brig-dev")) + "  \xc2\xb7  v" LAUNCHER_VERSION;
    const char *mark = markS.c_str();
    const std::string ver = tr("built on Shagg's Athelas and ROTK Fix");
    float ms = F_SMALL->LegacySize * ImGui::GetStyle().FontScaleMain, vs = ms * 0.85f;
    ImVec2 m = F_SMALL->CalcTextSizeA(ms, FLT_MAX, 0, mark), v = F_SMALL->CalcTextSizeA(vs, FLT_MAX, 0, ver.c_str());
    ImVec2 mp = o + ImVec2(right - m.x, y), vp = o + ImVec2(right - v.x, y + m.y);
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (dx || dy) {
                dl->AddText(F_SMALL, ms, mp + ImVec2(dx * S(1.2f), dy * S(1.2f)), IM_COL32(0, 0, 0, 200), mark);
                dl->AddText(F_SMALL, vs, vp + ImVec2(dx * S(1.2f), dy * S(1.2f)), IM_COL32(0, 0, 0, 200), ver.c_str());
            }
    {   // the line is a link: ROTKPlus on GitHub
        // (the cursor is left after the button: putting it back without an item after it is an error for ImGui,
        // "SetCursorPos() to extend window boundaries"; whatever comes next sets its own place)
        ImGui::SetCursorScreenPos(mp);
        ImGui::PushID("makerlink");
        bool hit = ImGui::InvisibleButton("##brig", m, ImGuiButtonFlags_MouseButtonLeft);
        bool hov = ImGui::IsItemHovered();
        ImGui::PopID();
        if (hov) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("%s", tr("ROTKPlus on GitHub: if you like it, leave a star."));
            dl->AddLine(mp + ImVec2(0, m.y), mp + ImVec2(m.x, m.y), ImGui::GetColorU32(C_GOLD_H), S(1));
        }
        if (hit) ShellExecuteA(NULL, "open", "https://github.com/brig-dev/ROTKPlus", NULL, NULL, SW_SHOWNORMAL);
        dl->AddText(F_SMALL, ms, mp, ImGui::GetColorU32(hov ? C_TEXT : C_GOLD_H), mark);
    }
    dl->AddText(F_SMALL, vs, vp, ImGui::GetColorU32(C_DIM), ver.c_str());
    return y + m.y + v.y + S(8);
}

static void update_button(App &a, ImVec2 size, float sy)
{
    const float pad = S(30);
    {
        std::string nv = update_newer();
        float uprog = 0;
        int ust = update_install_state(&uprog, nullptr);
        if (!nv.empty() || ust == 1 || ust == 2) {
            char lbl[64];
            if (ust == 1 || ust == 2)   // two steps, each 0 to 100%: the download, then the files put in place
                snprintf(lbl, sizeof lbl, (std::string(tr(ust == 1 ? "Downloading  %d%%" : "Installing  %d%%")) + "##hdrupd").c_str(), (int)(uprog * 100 + 0.5f));
            else snprintf(lbl, sizeof lbl, (std::string(tr("Update to %s")) + "##hdrupd").c_str(), nv.c_str());   // (translated before: "Update to 1.5.2" matches no line, it showed "Aggiorna to")
            ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
            float bw = std::max(S(150), ImGui::CalcTextSize(lbl, nullptr, true).x + S(28));
            ImGui::SetCursorPos(ImVec2(size.x - pad - bw, sy + S(4)));
            ImGui::PushStyleColor(ImGuiCol_Text, C_GOLD_H);
            if (orn_plate_button(lbl, ImVec2(bw, S(30)), ust != 1 && ust != 2)) g_askUpdate = true;
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ust != 1 && ust != 2)
                ImGui::SetTooltip("ROTKPlus %s is out. Your settings, saves and achievements are kept.", nv.c_str());
            ImGui::PopFont();
        }
    }
    update_popup(a);
}

// ROTKPlus: spaced letters centred on cx, with a dark outline (the settings' banner)
static void hdr_spaced(ImDrawList *dl, ImFont *f, float sz, float cx, float y, ImU32 col, const std::string &t, float gap)
{
    auto len = [&](size_t i) { unsigned char c = (unsigned char)t[i]; return c >= 0xF0 ? 4u : c >= 0xE0 ? 3u : c >= 0xC0 ? 2u : 1u; };
    float w = 0;
    for (size_t i = 0; i < t.size(); i += len(i)) w += f->CalcTextSizeA(sz, FLT_MAX, 0, t.c_str() + i, t.c_str() + i + len(i)).x + gap;
    float x = cx - (w - gap) * 0.5f;
    for (size_t i = 0; i < t.size(); i += len(i)) {
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                if (dx || dy) dl->AddText(f, sz, ImVec2(x + (float)dx * S(1.4f), y + (float)dy * S(1.4f)), IM_COL32(0, 0, 0, 170),
                                          t.c_str() + i, t.c_str() + i + len(i));
        dl->AddText(f, sz, ImVec2(x, y), col, t.c_str() + i, t.c_str() + i + len(i));
        x += f->CalcTextSizeA(sz, FLT_MAX, 0, t.c_str() + i, t.c_str() + i + len(i)).x + gap;
    }
}

void draw_ui(App &a, ImVec2 size, SDL_Renderer *ren)
{
    games_apply_pending(a);   // Show / Remove from the games list, between frames
    // change background every 15 s
    {   // testing: ROTKPLUS_PAGE = history, screenshots, fellowship or settings opens that page; ROTKPLUS_HISTORY = n
        // chooses the History's entry (0 the story)
        static bool pageEnv = true;
        if (pageEnv && a.front) {
            pageEnv = false;
            std::string p = getenv("ROTKPLUS_PAGE") ? getenv("ROTKPLUS_PAGE") : "";
            if (p == "history") { a.history = true; a.front = false; }
            else if (p == "screenshots") { a.gallery = true; a.front = false; }
            else if (p == "fellowship") { a.fellowship = true; a.front = false; }
            else if (p == "settings" && fr_first(a, FR_ROTK, GK_ROTK_PC) >= 0) open_settings(a, fr_first(a, FR_ROTK, GK_ROTK_PC));
            else if (p == "settings") { a.cur = -1; open_settings(a, -1); }   // (no game yet: the page that finds it)
            if (const char *h = getenv("ROTKPLUS_HISTORY")) g_hiSel = std::max(0, std::min(k_hiCount, atoi(h)));
        }
    }
    // Background pictures: the game art comes from the player's Return of the King (PC)
    // (or, without the PC version, from an extracted Xbox copy: fewer and smaller pictures)
    if (SDL_GetAtomicInt(&g_artState) == 0) {
        std::vector<std::string> dirs;
        for (GameKind k : { GK_ROTK_PC, GK_ROTK_XBOX })
            for (const LibGame &g : a.lib)
                if (g.kind == k) dirs.push_back(g.path);     // (a folder, or an Xbox disc image)
        art_start(dirs);
    }
    {
        GameKind k = a.cur >= 0 && a.cur < (int)a.lib.size() ? a.lib[a.cur].kind : GK_NONE;
        int group = k == GK_TT_XBOX ? 1 : k == GK_FOTR_XBOX ? 2 : k == GK_TA_XBOX ? 4 : 0;   // (4: The Third Age: its film stills and the other games' art)
        std::string dir = group == 4 || group == 0 ? ta_mix_dirs(a) : a.lib[a.cur].path;
        if (a.front) front_bg_group(a, group, dir);   // the game in front of the carousel
        {   // for now every game's backgrounds are The Two Towers' gallery (39 film stills and 3 menu pictures) and
            // Fellowship's sketches, the best art on the Xbox discs, from whichever of the two the player has (group 5);
            // with neither, each game's own as before
            // ROTKPlus: not here - only The Return of the King's own pictures (group 0)
        }
        if (group != a.bg.group || (group == 0 && ((art_ready() && !a.bg.artIn) || dir != a.bg.dir)) || (group >= 1 && group <= 2 && dir != a.bg.dir) ||
            (group == 4 && (dir != a.bg.dir || (art_ready() && !a.bg.artIn))) || (group == 5 && dir != a.bg.dir)) {
            backgrounds_rebuild(a.bg, group, exe_dir(), dir);
            a.bg.dir = dir;
        }
    }
    backgrounds_poll(a.bg, ren);
    if (!a.bg.job && !a.bg.files.empty() &&
        (!a.bg.cur.tex || a.bg.fresh || (a.bg.files.size() > 1 && ImGui::GetTime() - a.bg.changedAt > 10.0))) {
        backgrounds_next(a.bg);
        if (a.bg.job) a.bg.fresh = false;
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(size);
    ImGui::Begin("##main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
    draw_background(ImGui::GetBackgroundDrawList(), size, a.bg);
    if (a.front && g_animations) draw_embers(ImGui::GetBackgroundDrawList(), size);
    {   // ROTKPlus: a page that opens comes out of the night blue, in a quarter of a second
        static int was = -1;
        static double at = -10;
        const int page = a.front ? 0 : a.fellowship ? 1 : a.history ? 2 : a.gallery ? 3 : 4;
        if (page != was) { if (was >= 0) at = ImGui::GetTime(); was = page; }
        const float t = (float)(ImGui::GetTime() - at) / 0.3f;
        if (g_animations && t < 1.0f)
            ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0, 0), size, IM_COL32(6, 9, 16, (int)(190.0f * (1 - t) * (1 - t))));
    }
    // an installed update: the new launcher takes over (whichever page is showing: checked before the front page
    // returns, which 1.7.0 missed)
    static bool restarted = false;
    if (!restarted && update_install_state(nullptr, nullptr) == 3) {
        restarted = true;
        if (update_restart()) a.quit = true;
    }
    {   // a game just closed: the statistics and the deeds are read again
        static bool wasRunning;
        if (wasRunning && !a.running) fs_forget();
        wasRunning = a.running != NULL;
    }
    if (a.history) {
        draw_history(a, size, ren);
        update_button(a, size, maker_mark(size));
        ImGui::End();
        return;
    }
    if (a.gallery) {
        draw_gallery(a, size, ren);
        update_button(a, size, maker_mark(size));
        ImGui::End();
        return;
    }
    if (a.fellowship) {
        draw_fellowship(a, size, ren);
        update_button(a, size, maker_mark(size));
        ImGui::End();
        return;
    }
    if (a.front) {
        draw_front(a, size, ren);
        update_button(a, size, maker_mark(size));
        ImGui::End();
        return;
    }

    float pad = S(30), bannerH = S(142), barH = S(66);
    const GameInfo &g = a.game;

    // ---- banner (ROTKPlus): our emblem on the left, the game's name in the middle as on the front page (THE LORD OF THE
    // RINGS spaced, The Return of the King large, a rule with a gem), the project's mark on the right. Athelas had a
    // picker of its games here (box, name and version on a plate) and its name as a title: with one game, not needed
    ImDrawList *dl = ImGui::GetWindowDrawList();
    {
        if (!g_front.coversLooked) front_load_covers(ren);   // (a page opened first: our emblem for the header)
        front_art_poll(a, ren);
        const ImVec2 wp = ImGui::GetWindowPos();
        if (a.cur >= 0) {
            int e = fr_entry_of(a.lib[a.cur].kind);
            // the player's own scan first, then our mark (the star alone: the whole logo is too wide here), then the cover
            const FrontTex &ct = e == FR_ROTK && g_boxScan.tex ? g_boxScan : e == FR_ROTK && g_mark.tex ? g_mark : fr_face(a, e);
            float bh = S(104), bw = bh * (ct.tex && ct.h > 0 ? std::max(0.55f, std::min(1.45f, (float)ct.w / ct.h)) : 0.70f);
            ImVec2 p0 = wp + ImVec2(pad + S(4), S(26)), p1 = p0 + ImVec2(bw, bh);
            if (ct.tex) {
                ImVec2 uv0, uv1;
                crop_uv(ct.w, ct.h, bw, bh, uv0, uv1);
                if (!ct.cut) dl->AddRectFilled(p0 + ImVec2(S(3), S(4)), p1 + ImVec2(S(3), S(4)), IM_COL32(0, 0, 0, 120));
                dl->AddImage((ImTextureID)(intptr_t)ct.tex, p0, p1, uv0, uv1);
                if (!ct.cut) dl->AddRect(p0, p1, IM_COL32(81, 119, 140, 200), 0.0f, S(1));   // (our emblem: no frame)
            }
        }
        // the game's name, centred
        const float cx = wp.x + size.x * 0.5f, s1 = hi_fs(F_HEAD, 0.85f), s2 = hi_fs(F_TITLE, 0.6f);
        float ty = wp.y + S(34);
        hdr_spaced(dl, F_HEAD, s1, cx, ty, ImGui::GetColorU32(C_GOLD), tr("THE LORD OF THE RINGS"), S(4));
        ty += s1 + S(2);
        hdr_spaced(dl, F_TITLE, s2, cx, ty, ImGui::GetColorU32(C_GOLD_H), tr("The Return of the King"), S(1));
        ty += s2 + S(8);
        dl->AddLine(ImVec2(cx - S(130), ty), ImVec2(cx - S(12), ty), IM_COL32(170, 190, 220, 120), S(1));
        dl->AddLine(ImVec2(cx + S(12), ty), ImVec2(cx + S(130), ty), IM_COL32(170, 190, 220, 120), S(1));
        hi_gem(dl, ImVec2(cx, ty), S(4), ImGui::GetColorU32(C_GOLD_H));
    }
    // another PC copy chosen that already has ROTK Fix: show its settings
    if (a.cur >= 0 && a.lib[a.cur].kind == GK_ROTK_PC && !a.lib[a.cur].iso && _stricmp(a.lib[a.cur].path.c_str(), a.dir.c_str())) {
        if (a.otherFor != a.lib[a.cur].path) { a.other = inspect_game(a.lib[a.cur].path); a.otherFor = a.lib[a.cur].path; }
        if (fix_installed(a.other)) use_pc_game(a, a.lib[a.cur].path);
    }
    const bool own = own_game(a);
    // an Xbox game shown (The Two Towers, The Fellowship of the Ring, The Return of the King), and its port ready: Play
    // and Save work for it
    if (!own && tp_kind(a.lib[a.cur].kind)) {
        tp_use(a, tp_of(a.lib[a.cur].kind));
        if (!a.ttLooked) tt_find(a);
    }
    const bool ttShown = !own && tp_kind(a.lib[a.cur].kind) && a.ttLooked && tt_ready(a);
    g_xboxPage = false;   // The Return of the King on Xbox had the PC version's page up to 1.5; it has its own now

    // status (top right, one per line)
    struct St { bool ok, warn; std::string label, tip; };
    std::vector<St> st;
    bool filesOk = g.hasDll && g.hasProxyD3D8 && g.hasProxyDinput && g.hasDxvk;
    st.push_back({ g.exeFound && g.versionOk, g.exeFound, g.exeFound ? (g.versionOk ? "Game v1.0" : g.discProtected ? "Disc version (protected)" : "Unknown game version") : "Game not found",
                   g.exeFound ? g.versionText : "Add your game on the About tab." });
    st.push_back({ filesOk, false, filesOk ? "ROTK Fix ready" : "Files missing",
                   filesOk ? "ROTK Fix and the Vulkan renderer are installed." : "Some ROTK Fix files are missing - see the About tab." });
    if (g.otherD3D8 || g.otherDinput) {
        std::string f = g.otherD3D8 && g.otherDinput ? "d3d8.dll and dinput8.dll" : g.otherD3D8 ? "d3d8.dll" : "dinput8.dll";
        st.push_back({ false, true, "Another mod found",
                       "The " + f + " in the game folder belongs to another mod (for example dgVoodoo, DXVK on its own, "
                       "or a fix that came with a repack). ROTK Fix can't load while it's there.\n\n"
                       "Move it out of the game folder (keep it in case you want it back), then copy ROTK Fix's " + f + " in again." });
    }
    if (own && !g.exeFound) st.clear();   // the welcome screen: nothing is wrong, nothing to report yet
    if (!own) {   // another game is shown: just say what it is
        st.clear();
        const LibGame &lg = a.lib[a.cur];
        bool tt = tp_kind(lg.kind);
        if (tt && !a.ttLooked) tt_find(a);
        bool ready = tt ? tt_ready(a) : game_kind_playable(lg.kind);
        st.push_back({ ready, true, ready ? "Ready" : tt ? "Port not found" : "Not playable yet", lg.path });
    }
    // ROTKPlus: only the lines that need attention
    st.erase(std::remove_if(st.begin(), st.end(), [](const St &x) { return x.ok; }), st.end());
    ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
    float colW = 0;
    for (auto &x : st) colW = std::max(colW, ImGui::CalcTextSize(x.label.c_str()).x + S(15));
    float sy = maker_mark(size), lh = ImGui::GetTextLineHeight() + S(4);
    for (auto &x : st) {
        ImGui::SetCursorPos(ImVec2(size.x - pad - colW, sy));
        status_dot(x.ok, x.warn, x.label.c_str(), x.tip.c_str());
        sy += lh;
    }
    ImGui::PopFont();
    // a newer Athelas: a button under the status, on every page, that asks to install it (and shows the download)
    update_button(a, size, sy);

    // ---- settings panel
    // an elven frame around it; the panel itself sits just inside the frame
    const float frameT = S(9);
    // on the right, the picture left of it in the clear
    const float panelW = std::min(size.x - pad * 2, S(600)), panelX = (size.x - panelW) * 0.5f;
    ImVec2 panelA = ImGui::GetWindowPos() + ImVec2(panelX, bannerH);
    ImVec2 panelB = panelA + ImVec2(panelW, size.y - bannerH - barH - S(30));
    orn_panel_fill(ImGui::GetWindowDrawList(), panelA, panelB, ImGui::GetColorU32(rgb(15, 14, 13, 0.70f)));
    const float inset = frameT * 0.5f + S(5);   // clear of the frame's lines
    ImGui::SetCursorPos(ImVec2(panelX + inset, bannerH + inset));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("panel", ImVec2(panelW - inset * 2, size.y - bannerH - barH - S(30) - inset * 2), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened);
    g_labelW = std::max(S(230), ImGui::GetContentRegionAvail().x * 0.36f);
    if (ttShown) {
        if (!a.ttLoaded) tt_load_settings(a);
        tt_tabs(a, a.lib[a.cur]);
    } else if (!own) {
        ImGui::BeginChild("other", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoBackground);
        draw_other_game(a);
        ImGui::Dummy(ImVec2(0, S(10)));
        games_section(a);
        ImGui::EndChild();
    } else if (!g.exeFound) {
        ImGui::BeginChild("setup", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoBackground);
        draw_setup(a);
        ImGui::EndChild();
    } else {
    ImGui::PushFont(F_TAB, F_TAB->LegacySize);
    static const char *tabNames[] = { "Display", "Graphics", "Controls", "Audio", "Gameplay", "About" };
    static int rotkSel = 0;
    float tabW = tabs_begin(tabNames, 6, rotkSel);
    if (ImGui::BeginTabBar("tabs")) {
        struct T { const char *name; void (*fn)(App &); } tabs[] = {
            { "###display", tab_display }, { "###graphics", tab_graphics_plus }, { "###controls", tab_controls_plus },
            { "###audio", tab_audio }, { "###gameplay", tab_gameplay_plus }, { "###about", tab_about } };
        // ROTKFIX_TAB=n (testing): open on tab n
        static int startTab = getenv("ROTKFIX_TAB") ? atoi(getenv("ROTKFIX_TAB")) : -1;
        if (int step = pad_tab_step(a)) startTab = (rotkSel + step + 6) % 6;
        int ti = 0;
        for (auto &t : tabs) {
            int idx = ti;
            ImGuiTabItemFlags tf = ti++ == startTab ? ImGuiTabItemFlags_SetSelected : 0;
            ImGui::SetNextItemWidth(tabW);
            if (ImGui::BeginTabItem(t.name, nullptr, tf)) {
                rotkSel = idx;
                g_slowWhere = tabNames[idx];   // for the slow-frame note (main.cpp)
                pad_tab_focus();
                ImGui::PopFont();
                ImGui::BeginChild("tab", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoBackground);
                // ROTKPlus: the pages scroll up and down only (a row a little too wide let the mouse wheel, the
                // controller or a click move the whole page sideways)
                if (ImGui::GetScrollX() != 0.0f) ImGui::SetScrollX(0.0f);
                t.fn(a);
                ImGui::Dummy(ImVec2(0, S(6)));
                if (getenv("ROTKFIX_SCROLL_END")) ImGui::SetScrollY(ImGui::GetScrollMaxY());   // testing
                else if (const char *sye = getenv("ROTKFIX_SCROLL_Y")) ImGui::SetScrollY(std::min((float)atof(sye) * g_scale, ImGui::GetScrollMaxY()));   // testing
                ImGui::EndChild();
                ImGui::PushFont(F_TAB, F_TAB->LegacySize);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
        startTab = -1;
    }
    tabs_end();
    ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    orn_panel_frame(ImGui::GetWindowDrawList(), panelA, panelB);
    // Down on a page's last control: ImGui finds nothing below it inside the panel and stays there, so with a
    // controller or the keyboard the Library, Save and Play buttons couldn't be reached (owner's report, 5 Nov). When a
    // Down press leaves the cursor where it was, it goes to Save; Up from there goes back into the page.
    static ImGuiID downFrom = 0, saveId = 0, libId = 0;   // Save, or Library when Save can't be pressed
    static int downAge = 0;
    static float downScroll = 0;   // the page's scroll at the press: a press that only scrolled the page isn't the end
    {
        ImGuiContext &gc = *GImGui;
        bool free = !ImGui::IsAnyItemActive() && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
        if (downFrom && ++downAge >= 2) {
            ImGuiID to = saveId ? saveId : libId;
            bool scrolled = gc.NavWindow && gc.NavWindow->Scroll.y != downScroll;
            if (free && !scrolled && gc.NavId == downFrom && to && downFrom != saveId && downFrom != libId) {
                ImGui::SetFocusID(to, ImGui::GetCurrentWindow());
                ImGui::SetNavCursorVisible(true);
            }
            downFrom = 0;
        }
        // (any owner: ImGui's navigation owns these keys, so a plain IsKeyPressed never sees them)
        auto pressed = [](ImGuiKey k) { return ImGui::IsKeyPressed(k, ImGuiInputFlags_Repeat, ImGuiKeyOwner_Any); };
        bool down = pressed(ImGuiKey_DownArrow) || pressed(ImGuiKey_GamepadDpadDown) || pressed(ImGuiKey_GamepadLStickDown);
        if (down && free && gc.NavId && !downFrom) { downFrom = gc.NavId; downAge = 0; downScroll = gc.NavWindow ? gc.NavWindow->Scroll.y : 0; }
    }

    // ---- bottom bar: summary on the left, buttons on the right
    float by = size.y - barH - S(16);
    int w, h; effective_res(a, w, h);
    static const char *dm[] = { "Borderless", "Fullscreen", "Windowed" };
    static const char *pn[] = { "Original", "Balanced", "High", "Ultra", "Custom" };
    int fps = a.s.vsync ? std::min(a.s.targetFps, effective_refresh(a)) : a.s.targetFps;
    float playW = S(170), playH = S(54), btnH = S(40), saveW = S(100);
    float bx = size.x - pad - playW;
    // summary and messages on a small parchment card, readable over any background
    {
        const float libW = S(130), cardX = pad + S(6) + libW + S(12);   // between the Library button and Save
        float cardW = bx - saveW - S(24) - cardX;
        bool msg = !a.message.empty() && ImGui::GetTime() - a.messageAt < (a.messageBad ? 30.0 : 4.0);
        std::string newer = update_install_state(nullptr, nullptr) > 0 ? std::string() : update_newer();   // not while it installs
        // playing with a controller: its shortcuts, in its own names, while there's no message to show
        const int forcedHints = pad_hints_test();
        const bool padHints = !msg && (forcedHints >= 0 || (a.controllers > 0 && GImGui->NavCursorVisible &&
                                                           GImGui->NavInputSource == ImGuiInputSource_Gamepad));
        // the card's height from the font's own line height (ROTKPlus 1.5.2: it counted the "is out" line as 18 pixels,
        // less than a line and its spacing, so with the controller's buttons too their line went below the card)
        ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
        const float lh = ImGui::GetTextLineHeight(), sp = ImGui::GetStyle().ItemSpacing.y, textW = cardW - S(18);
        char sum[256], nw[160] = "", nwShort[96] = "";
        if (own && !g.exeFound) snprintf(sum, sizeof sum, "%s", "Add a game to begin");
        else if (own) snprintf(sum, sizeof sum, "%d x %d  |  %s  |  %d fps  |  %s", w, h, dm[a.s.displayMode], fps, pn[detect_preset(a.s)]);
        else if (ttShown) snprintf(sum, sizeof sum, "%s", tt_summary(a).c_str());
        else snprintf(sum, sizeof sum, "%s  |  %s", game_kind_name(a.lib[a.cur].kind),
                      game_kind_playable(a.lib[a.cur].kind) ? "settings in its own launcher" : "not playable yet");
        if (!newer.empty()) snprintf(nw, sizeof nw, tr("ROTKPlus %s is out  -  press Update at the top right"), newer.c_str());   // (translated here: once written, it no longer matches the translation's line)
        if (!newer.empty()) snprintf(nwShort, sizeof nwShort, tr("ROTKPlus %s is out"), newer.c_str());
        static const char kGap[] = "     \xc2\xb7     ";
        // the news of a newer version on the summary's line when both fit there, else on a line of its own
        // (the summary is translated as drawn: some room for longer words); when the whole news doesn't fit, its short
        // form, "ROTKPlus 1.5.2 is out" (the header's button says the rest)
        const float sumW = ImGui::CalcTextSize(sum).x * 1.15f + ImGui::CalcTextSize(kGap).x;
        if (nw[0] && sumW + ImGui::CalcTextSize(nw).x > textW && sumW + ImGui::CalcTextSize(nwShort).x <= textW)
            snprintf(nw, sizeof nw, "%s", nwShort);
        const bool nwInline = nw[0] && sumW + ImGui::CalcTextSize(nw).x <= textW;
        float cardH = S(12) + lh;
        if (nw[0] && !nwInline) cardH += sp + lh;
        if (msg) cardH += sp + ImGui::CalcTextSize(a.message.c_str(), nullptr, false, textW).y;   // however many lines it wraps to
        if (padHints) cardH += sp + S(2) + ImGui::GetFontSize() * 1.2f;   // (ROTKPlus: the buttons drawn, on one line)
        // centred on Play, but never lower than a little under it (the window's frame is close below)
        const float cy = std::min(by + (playH - cardH) * 0.5f, by + playH + S(6) - cardH);
        ImVec2 c0 = ImGui::GetWindowPos() + ImVec2(cardX, cy), c1 = c0 + ImVec2(cardW, cardH);
        orn_plate(ImGui::GetWindowDrawList(), c0, c1, ImGui::GetColorU32(rgb(15, 14, 13, 0.86f)), false);
        ImGui::SetCursorPos(ImVec2(cardX + S(10), cy + S(5)));
        ImGui::PushTextWrapPos(cardX + cardW - S(8));
        ImGui::TextColored(C_DIM, "%s", sum);
        if (nwInline) {
            ImGui::SameLine(0, 0); ImGui::TextColored(C_DIM, "%s", kGap);
            ImGui::SameLine(0, 0); ImGui::TextColored(C_GOLD_H, "%s", nw);
        } else if (nw[0]) {
            ImGui::SetCursorPosX(cardX + S(10));
            ImGui::TextColored(C_GOLD_H, "%s", nw);
        }
        if (msg) {
            ImGui::SetCursorPosX(cardX + S(10));
            ImGui::TextColored(a.messageBad ? C_BAD : C_GOOD, "%s", a.message.c_str());
        }
        if (padHints) {   // ROTKPlus: the controller's buttons drawn as on it, each with what it does
            static const PadHint kSet[] = { { PG_SHOULDERS, "tabs" }, { PG_START, "Play" }, { PG_SELECT, "Save" }, { PG_EAST, "back" },
                                            { PG_TRIGGERS, "fullscreen" } };
            const int ls = forcedHints >= 0 ? forcedHints : label_style(a);
            ImFont *hf = ImGui::GetFont();
            float hfs = ImGui::GetFontSize();
            const float hw = pad_hints_width(kSet, 5, ls, hf, hfs);
            if (hw > cardW - S(20)) hfs *= (cardW - S(20)) / hw;
            ImGui::SetCursorPosX(cardX + S(10));
            const ImVec2 hp = ImGui::GetCursorScreenPos() + ImVec2(0, S(2));
            pad_hints_draw(ImGui::GetWindowDrawList(), hp, kSet, 5, ls, hf, hfs, ImGui::GetColorU32(C_GOLD));
            ImGui::Dummy(ImVec2(0, hfs * 1.2f));
        }
        ImGui::PopTextWrapPos();
        ImGui::PopFont();
    }

    // ROTKFIX_PLAY (testing): press Play once at start-up
    // ROTKFIX_TT_SET="graphics.fps=180,..." / ROTKFIX_TT_PRESET=name / ROTKFIX_TT_SAVE (testing):
    // change Two Towers settings the way the menus do, then save
    static bool ttTest = true;
    if (ttTest && ttShown) {
        ttTest = false;
        if (!a.ttLoaded) tt_load_settings(a);
        if (const char *e = getenv("ROTKFIX_TT_PRESET"))
            for (auto &pr : a.tt.presets) if (pr.name == e)
                for (auto &kv : pr.values) { size_t d = kv.first.find('.'); a.ttFile.set(kv.first.substr(0, d), kv.first.substr(d + 1), kv.second); }
        if (const char *e = getenv("ROTKFIX_TT_SET")) {
            std::string all = e;
            size_t at = 0;
            while (at < all.size()) {
                size_t en = all.find(',', at); if (en == std::string::npos) en = all.size();
                std::string item = all.substr(at, en - at);
                size_t d = item.find('.'), q = item.find('=');
                if (d != std::string::npos && q != std::string::npos)
                    tt_put(a, item.substr(0, d).c_str(), item.substr(d + 1, q - d - 1).c_str(), item.substr(q + 1));
                at = en + 1;
            }
        }
        if (getenv("ROTKFIX_TT_SAVE")) tt_save(a);
    }
    static bool autoUpdate = getenv("ROTKFIX_UPDATE_NOW") != nullptr;   // testing: install a newer release at once
    if (autoUpdate && !update_newer().empty()) { autoUpdate = false; update_install_start(update_newer()); }
    static bool autoPlay = getenv("ROTKFIX_PLAY") != nullptr;
    if (autoPlay && ttShown) { autoPlay = false; tt_play(a, a.lib[a.cur]); }
    ImGui::SetCursorPos(ImVec2(bx, by));
    bool playOk = !(ttShown ? a.running != NULL : (!own || !g.exeFound || g_fixRemoved));
    // a controller's Start is Play, and its View / Back button Save, wherever the cursor is (not with a menu open,
    // a control being changed or a button being set)
    bool padFree = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) && !ImGui::IsAnyItemActive() &&
                   a.capture < 0 && a.kbCapture < 0 && !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    bool padPlay = padFree && ImGui::IsKeyPressed(ImGuiKey_GamepadStart, false);
    bool padSave = padFree && ImGui::IsKeyPressed(ImGuiKey_GamepadBack, false);
    if (orn_arch_button("PLAY", ImVec2(playW, playH), F_PLAY, playOk) || (padPlay && playOk)) {
        if (ttShown) tt_play(a, a.lib[a.cur]);
        else pc_play(a);   // (Athelas updated since ROTK Fix was set up there: the game gets the new ROTK Fix first)
    }
    // back to the front screen (the carousel), bottom left. ROTKPlus: B as on the other pages (the hint says so; only Y
    // did it, 09/11/2026), and Esc. Not when that press only closed a list or let go of a control: ImGui does that at
    // the start of the frame, so what was open or held at the end of the last one is what counts.
    static bool busyBefore = false;
    const bool padBack = padFree && !busyBefore &&
                         (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, ImGuiInputFlags_None, ImGuiKeyOwner_Any) ||
                          ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false));
    const bool escBack = a.capture < 0 && a.kbCapture < 0 && !busyBefore && !ImGui::IsAnyItemActive() &&
                         !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) &&
                         ImGui::IsKeyPressed(ImGuiKey_Escape, ImGuiInputFlags_None, ImGuiKeyOwner_Any);
    ImGui::SetCursorPos(ImVec2(pad + S(6), by + (playH - btnH) * 0.5f));
    if (orn_plate_button("\xe2\x80\xb9  Back##lib", ImVec2(S(130), btnH), true) || padBack || escBack)
        back_to_front(a);
    libId = ImGui::GetItemID();
    bx -= saveW + S(12);
    ImGui::SetCursorPos(ImVec2(bx, by + (playH - btnH) * 0.5f));
    bool saveOk = ttShown || !(!own || !g.exeFound || g_fixRemoved);
    bool saveHit = orn_plate_button("Save", ImVec2(saveW, btnH), saveOk);
    saveId = saveOk ? ImGui::GetItemID() : 0;
    if (saveHit || (padSave && saveOk)) {
        if (ttShown) tt_save(a); else save(a);
    }

    busyBefore = ImGui::IsAnyItemActive() || a.capture >= 0 || a.kbCapture >= 0 ||
                 ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    draw_transition(a, size);
    ImGui::End();
}
