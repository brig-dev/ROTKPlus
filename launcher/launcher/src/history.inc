// ROTKPlus: the History page. On the left a timeline: our story first, then every version, the newest at the top; on
// the right the chosen one: for the story, how ROTKPlus was born, the thanks and the promises; for a version, its name,
// a few words, its main changes as cards and, folded under them, every change from WHATS-NEW.txt (ROTKPlus'
// CHANGELOG). The words are here in English and translated through lang\<code>.txt like the rest. Included by pages.cpp
// after gallery.inc. Nothing personal goes in these texts: the package is public.

struct HiCard { const char *title, *text; };
struct HiVersion { const char *ver, *date, *name, *summary; HiCard cards[6]; };

static const HiVersion k_hiVersions[] = {
    { "1.5.1", "12/11/2026", "Four gems",
      "A new face for ROTKPlus, a smoother launcher and two fixes for four players.",
      { { "A new logo", "A star of four gems, one for each player, on the front page, in the header and as the icon." },
        { "No more waiting", "The settings tabs and About open at once: the slow checks run on their own." },
        { "The same spear", "Two heroes grabbing the same spear at the same moment no longer stop the game." },
        { "Every name right", "The Last game shows Pelennor Fields and the Palantir by their names." } } },
    { "1.5.0", "09/11/2026", "The great host",
      "Battles as big as the films: more enemies and more allies in every level, chosen on the Multiplayer page. And "
      "ROTKPlus gets a home of its own: this launcher.",
      { { "More enemies", "From x1.5 to x7 in the campaign, in co-op and for a single player, in every level." },
        { "More allies", "As many more as the enemies, or a factor of their own." },
        { "Never stuck", "With three or four players, whoever is stuck or left behind is brought back next to a friend; players 3 and 4 can also press Back." },
        { "The ROTKPlus launcher", "Made from Athelas: the Fellowship, each player's card to share, the screenshots, a controller for each player." },
        { "HD packs", "Textures, portraits and fonts: each pack on or off with one click." },
        { "Sharper letters", "The game's fonts can be replaced, for text twice as sharp." } } },
    { "1.4.2", "08/11/2026", "A fair parry",
      "In Versus a parry no longer stops everything: the strongest blows go through.",
      { { "Fierce blows land", "A fierce attack, the last blow of a combo and a fully drawn shot go through a parry." },
        { "The rest as before", "Speed attacks, plain arrows and spears are parried as in the game." } } },
    { "1.4.1", "07/11/2026", "Aragorn at the Black Gate",
      "Aragorn as third or fourth player stood still after the Black Gate's opening scene: now he fights like everyone else.",
      { { "The right costume", "The extra players get the same character as players 1 and 2, as the game does in its last levels." } } },
    { "1.4.0", "06/11/2026", "Versus grows up",
      "One Multiplayer page for everything, the teams of the Sun and the Moon, heroes only, timed games and tournaments.",
      { { "The Multiplayer page", "Mode, players, enemies, lives and teams on one page, then straight to the map." },
        { "2 vs 2", "The Team of the Sun against the Team of the Moon: teammates can't hit each other." },
        { "Tournaments", "The best of 3 or 5 rounds, and a Cup for the winner." },
        { "Timed games", "2, 3 or 5 minutes, a point for every knock-down, sudden death on a tie." },
        { "Heroes only", "Three arenas and no enemies: only you and your friends." },
        { "On the screen", "Who downs whom, lives and points in the HUD, the Last game over the map." } } },
    { "1.3.0", "04/11/2026", "A mod of Athelas",
      "ROTKPlus learns to live inside Athelas too, as a mod started by ROTK Fix, and comes with Athelas 1.8.1.",
      { { "One game, two ways", "The same sources as the classic DivxMediaLib.dll, the same settings and statistics." } } },
    { "1.2.1", "03/11/2026", "Enter, always",
      "On the Fellowship pages Enter sometimes did nothing: a mouse button Windows still thought was held. Fixed.",
      { { "A sure confirm", "A click is a press, not a button that only seems down; the game's own menus are guarded too." } } },
    { "1.2.0", "03/11/2026", "The Fellowship",
      "Every player gets a name, and the game remembers: kills, levels, heroes, records and the last game, in a new "
      "Fellowship entry of the main menu.",
      { { "Names", "Typed or picked with the controller, shown above the heads and in the banners." },
        { "Who is playing?", "Before every game, one row per player, as the last time." },
        { "Statistics", "Kills by kind of enemy, bosses, combos, parries, ladders and much more." },
        { "Records", "Four players side by side, the records and who holds them." },
        { "Music with four", "Fixed: with four players the music and the voices could go silent." },
        { "Faster texts", "The mod's pages back at 60 fps under Vulkan." } } },
    { "1.1.0", "28/10/2026", "A log for every game",
      "Every game writes its own log and keeps the earlier ones, so nothing that goes wrong is lost.",
      { { "Session logs", "A new file at every start, with the system and the files of ROTK Fix." },
        { "Perfect, fixed", "A third or fourth player no longer starts with the Perfect kills of an earlier level." } } },
    { "1.0.0", "26/10/2026", "Four on the sofa",
      "The first release: co-op for three and four players and a Versus mode for two to four, in the game's own language.",
      { { "Co-op for 3 and 4", "The whole campaign with up to four heroes on one screen." },
        { "Versus", "Two to four players against each other in the game's levels." },
        { "In your language", "Every new text in the language of your copy of the game." } } },
};
static const int k_hiCount = (int)(sizeof k_hiVersions / sizeof k_hiVersions[0]);

static const char *const k_hiStory[] = {
    "The Return of the King came out in 2003 and on consoles it was a legendary game: the music and the voices of "
    "the films, the heroes side by side, the Southern Gate with endless orcs to cut down.",
    "In October 2026 it came back to life thanks to Athelas, Shagg's launcher and ROTK Fix, which made the old game "
    "run beautifully on modern PCs, with controllers, widescreen and Vulkan. That was the spark that made me think: "
    "could more be done?",
    "Then my own wish to bring back games played together on one screen, face to face, as they used to be. Having "
    "friends over is something else, and online play, as far as I'm concerned, will never replace it. So, on 23 "
    "October 2026, ROTKPlus began: a small library the game loads by itself, studied function by function, address "
    "by address, so that four could play.",
    "After more than a hundred hours of work, between tests, prompts, Claude subscriptions and coffee, it grew: "
    "co-op for three and four, a Versus mode with teams, tournaments and timed games, a name and statistics for "
    "every player, more enemies for bigger battles and, at last, this launcher, made from Athelas under its MIT "
    "licence and with every credit to Shagg.",
    "Everything is done with respect: for Tolkien's world and the films the game was born from, for the people who "
    "made it, and for the players who still love it. No file of the game is shipped: ROTKPlus works on your own "
    "copy, and you can take it away at any moment.",
    "It is written by me, brig-dev, with the unique support of millions of Claude's tokens (Anthropic), and I test "
    "it every day to hunt down new bugs and think up new, interesting features.",
};
static const char *const k_hiThanks[] = {
    "Shagg, for Athelas and ROTK Fix: the launcher, the renderer and the fixes all of this stands on.",
    "Electronic Arts and the studios who made The Return of the King in 2003.",
    "The128ReBits, for the HD textures of the GameCube version: I used a few of them for personal use only, and "
    "they are not in this package. Go and see their work: they make much more.",
    "J.R.R. Tolkien, and the films that gave the game its faces and its music.",
    "DXVK, SDL and Dear ImGui, the open source projects inside this launcher.",
    "Everyone who plays it, reports a bug or simply says thank you.",
};
static const HiCard k_hiPromises[] = {
    { "Free, always", "ROTKPlus costs nothing and never will. A coffee on Ko-fi is only a thank-you." },
    { "Open source", "MIT licence: the code is on GitHub, for anyone to read, learn from and improve." },
    { "Your own copy", "No file of the game is shipped: you need your own copy of The Return of the King." },
    { "Your data stays home", "Names and statistics are saved on your PC and never sent anywhere." },
    { "Unofficial", "A fan's project, not affiliated with Electronic Arts, Warner Bros., Middle-earth Enterprises or the Tolkien Estate." },
    { "Made with obsessive care", "Every version is tested over and over before it is released." },
};

static int g_hiSel = 0;                  // 0 the story, 1.. the versions (k_hiVersions[sel - 1])
static bool g_hiScrollTop = true;
static std::map<std::string, std::vector<std::string>> g_hiNotes;   // WHATS-NEW.txt: every change, by version
static bool g_hiNotesRead = false;

// WHATS-NEW.txt cut by its "## <version>" headings; the lines wrapped by hand joined again (as the notes are written)
static void hi_read_notes()
{
    g_hiNotesRead = true;
    FILE *f = fopen((exe_dir() + "\\WHATS-NEW.txt").c_str(), "rb");
    if (!f) return;
    std::string cur;
    char buf[2048];
    while (fgets(buf, sizeof buf, f)) {
        std::string l = buf;
        while (!l.empty() && (l.back() == '\n' || l.back() == '\r')) l.pop_back();
        for (size_t p; (p = l.find("**")) != std::string::npos;) l.erase(p, 2);
        if (l.rfind("## ", 0) == 0) {
            cur = l.substr(3);
            size_t sp = cur.find(' ');
            if (sp != std::string::npos) cur = cur.substr(0, sp);
            continue;
        }
        if (cur.empty() || l.rfind("# ", 0) == 0) continue;
        auto &v = g_hiNotes[cur];
        size_t lead = l.find_first_not_of(' ');
        if (lead == std::string::npos) { if (!v.empty() && !v.back().empty()) v.push_back(""); continue; }
        bool cont = !v.empty() && !v.back().empty() && v.back()[0] != '#' && l[lead] != '-' && l[lead] != '#';
        if (cont) v.back() += " " + l.substr(lead);
        else v.push_back(l.substr(lead));
    }
    fclose(f);
}

static float hi_fs(ImFont *f, float k) { return f->LegacySize * ImGui::GetStyle().FontScaleMain * k; }

static void hi_gem(ImDrawList *dl, ImVec2 c, float r, ImU32 col, bool ring)
{
    ImVec2 d[4] = { ImVec2(c.x - r, c.y), ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r) };
    dl->AddConvexPolyFilled(d, 4, col);
    if (ring) {
        float R = r + S(4);
        ImVec2 e[4] = { ImVec2(c.x - R, c.y), ImVec2(c.x, c.y - R), ImVec2(c.x + R, c.y), ImVec2(c.x, c.y + R) };
        dl->AddPolyline(e, 4, ImGui::GetColorU32(C_GOLD_H), S(1.5f), ImDrawFlags_Closed);
    }
}

// a paragraph in the page's own font, wrapped to the column
static void hi_para(const char *text, ImVec4 col, ImFont *font = nullptr)
{
    if (font) ImGui::PushFont(font, font->LegacySize);
    ImGui::PushTextWrapPos(0);
    ImGui::TextColored(col, "%s", tr(text));
    ImGui::PopTextWrapPos();
    if (font) ImGui::PopFont();
}

// cards two by two (one by one when narrow): a gem, a title in gold and a few words under it
static void hi_cards(const HiCard *cards, int n)
{
    const float avail = ImGui::GetContentRegionAvail().x, gap = S(14);
    const int cols = avail > S(560) ? 2 : 1;
    const float cw = (avail - gap * (cols - 1)) / cols, pad = S(14), textX = S(40);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float ts = hi_fs(F_BODY), ss = hi_fs(F_SMALL);
    for (int i = 0; i < n; i += cols) {
        float rowH = 0;
        for (int k = 0; k < cols && i + k < n; k++) {
            const HiCard &c = cards[i + k];
            float h = pad * 2 + F_BODY->CalcTextSizeA(ts, FLT_MAX, cw - textX - pad, tr(c.title)).y + S(4) +
                      F_SMALL->CalcTextSizeA(ss, FLT_MAX, cw - textX - pad, tr(c.text)).y;
            rowH = std::max(rowH, h);
        }
        ImVec2 o = ImGui::GetCursorScreenPos();
        for (int k = 0; k < cols && i + k < n; k++) {
            const HiCard &c = cards[i + k];
            ImVec2 p0 = o + ImVec2((cw + gap) * k, 0), p1 = p0 + ImVec2(cw, rowH);
            orn_plate(dl, p0, p1, IM_COL32(12, 17, 28, 215), false);
            hi_gem(dl, p0 + ImVec2(S(21), pad + ts * 0.55f), S(6), k_fsGem[(i + k) % 4]);
            dl->AddText(F_BODY, ts, p0 + ImVec2(textX, pad), ImGui::GetColorU32(C_GOLD_H), tr(c.title), nullptr, cw - textX - pad);
            float th = F_BODY->CalcTextSizeA(ts, FLT_MAX, cw - textX - pad, tr(c.title)).y;
            dl->AddText(F_SMALL, ss, p0 + ImVec2(textX, pad + th + S(4)), ImGui::GetColorU32(C_TEXT), tr(c.text), nullptr, cw - textX - pad);
        }
        ImGui::Dummy(ImVec2(avail, rowH));
        ImGui::Dummy(ImVec2(0, gap - ImGui::GetStyle().ItemSpacing.y * 2));
    }
}

// a heading: small capitals in gold, a thin line to the right
static void hi_heading(const char *text)
{
    ImGui::Dummy(ImVec2(0, S(10)));
    float x1 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    ImGui::PushFont(F_HEAD, F_HEAD->LegacySize);
    ImGui::TextColored(C_GOLD_H, "%s", tr(text));
    ImVec2 r0 = ImGui::GetItemRectMin(), r1 = ImGui::GetItemRectMax();
    ImGui::PopFont();
    if (x1 > r1.x + S(20))
        ImGui::GetWindowDrawList()->AddLine(ImVec2(r1.x + S(14), (r0.y + r1.y) * 0.5f), ImVec2(x1, (r0.y + r1.y) * 0.5f),
                                            IM_COL32(120, 140, 170, 90), S(1));
    ImGui::Dummy(ImVec2(0, S(4)));
}

static void hi_story(App &a, SDL_Renderer *ren)
{
    if (!g_front.coversLooked) front_load_covers(ren);
    const FrontTex &logo = g_front.cover[FR_ROTK];
    const float avail = ImGui::GetContentRegionAvail().x;
    if (logo.tex && logo.h > 0) {   // our logo at the top, framed like the front screen's
        // (small enough that the story starts on the same screen)
        float h = std::min(S(190), (avail - S(20)) * logo.h / logo.w), w = h * logo.w / logo.h;
        ImVec2 p0 = ImGui::GetCursorScreenPos() + ImVec2((avail - w) * 0.5f, S(8));
        ImDrawList *dl = ImGui::GetWindowDrawList();
        if (!logo.cut) dl->AddRectFilled(p0 - ImVec2(S(5), S(5)), p0 + ImVec2(w, h) + ImVec2(S(5), S(5)), IM_COL32(6, 9, 16, 200), S(4));
        dl->AddImage((ImTextureID)(intptr_t)logo.tex, p0, p0 + ImVec2(w, h));
        if (!logo.cut) dl->AddRect(p0 - ImVec2(S(5), S(5)), p0 + ImVec2(w, h) + ImVec2(S(5), S(5)), ImGui::GetColorU32(C_GOLD), S(4), S(1.5f));
        ImGui::Dummy(ImVec2(avail, h + S(22)));
    }
    hi_heading("How it was born");
    for (const char *p : k_hiStory) { hi_para(p, C_TEXT, F_BODY); ImGui::Dummy(ImVec2(0, S(6))); }
    hi_heading("Thanks to");
    for (int i = 0; i < (int)(sizeof k_hiThanks / sizeof k_hiThanks[0]); i++) {
        ImVec2 c = ImGui::GetCursorScreenPos() + ImVec2(S(8), hi_fs(F_BODY) * 0.6f);
        hi_gem(ImGui::GetWindowDrawList(), c, S(4.5f), k_fsGem[i % 4]);
        ImGui::Indent(S(24));
        hi_para(k_hiThanks[i], C_TEXT, F_BODY);
        ImGui::Unindent(S(24));
        ImGui::Dummy(ImVec2(0, S(2)));
    }
    hi_heading("Our promises");
    hi_cards(k_hiPromises, (int)(sizeof k_hiPromises / sizeof k_hiPromises[0]));
    ImGui::Dummy(ImVec2(0, S(6)));
    if (ImGui::Button("Support ROTKPlus on Ko-fi##hk")) ShellExecuteA(NULL, "open", ROTKPLUS_KOFI, NULL, NULL, SW_SHOWNORMAL);
    ImGui::SameLine();
    if (ImGui::Button("ROTKPlus on GitHub##hg")) ShellExecuteA(NULL, "open", "https://github.com/brig-dev/ROTKPlus", NULL, NULL, SW_SHOWNORMAL);
    (void)a;
}

static void hi_version(const HiVersion &v, int idx)
{
    // the title: the version large, its name in gold beside it, the date under them
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float big = hi_fs(F_TITLE, 0.62f);
    std::string vt = "ROTKPlus " + std::string(v.ver);
    ImVec2 p = ImGui::GetCursorScreenPos() + ImVec2(0, S(6));
    ImVec2 vs = F_TITLE->CalcTextSizeA(big, FLT_MAX, 0, vt.c_str());
    dl->AddText(F_TITLE, big, p + ImVec2(S(1.5f), S(1.5f)), IM_COL32(0, 0, 0, 120), vt.c_str());
    dl->AddText(F_TITLE, big, p, ImGui::GetColorU32(C_GOLD_H), vt.c_str());
    ImGui::Dummy(ImVec2(vs.x, vs.y + S(8)));
    ImGui::PushFont(F_HEAD, F_HEAD->LegacySize);
    ImGui::TextColored(C_GOLD, "%s", tr(v.name));
    ImGui::PopFont();
    ImGui::SameLine(0, S(14));
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(C_DIM, "%s", tr(v.date));
    if (idx == 0) {   // the newest: a little badge
        ImGui::SameLine(0, S(12));
        ImGui::TextColored(C_GOOD, "\xe2\x97\x86 %s", tr("this version"));
    }
    ImGui::Dummy(ImVec2(0, S(6)));
    hi_para(v.summary, C_TEXT, F_BODY);
    hi_heading("What it brings");
    int n = 0;
    while (n < 6 && v.cards[n].title) n++;
    hi_cards(v.cards, n);
    // every change, folded: the notes as they are written (in English)
    if (!g_hiNotesRead) hi_read_notes();
    auto it = g_hiNotes.find(v.ver);
    if (it != g_hiNotes.end() && !it->second.empty()) {
        ImGui::Dummy(ImVec2(0, S(6)));
        ImGui::PushStyleColor(ImGuiCol_Text, C_GOLD_H);
        if (getenv("ROTKPLUS_NOTES_OPEN")) ImGui::SetNextItemOpen(true, ImGuiCond_Once);   // (testing)
        bool open = ImGui::CollapsingHeader((std::string(tr("Every change (in English)")) + "##hall").c_str());
        ImGui::PopStyleColor();
        if (open) {
            i18n_verbatim(true);   // (the notes are English: no word of them translated)
            ImGui::PushFont(F_SMALL, F_SMALL->LegacySize);
            ImGui::PushTextWrapPos(0);
            for (auto &l : it->second) {
                if (l.empty()) { ImGui::Dummy(ImVec2(0, S(4))); continue; }
                if (l.rfind("### ", 0) == 0) { ImGui::TextColored(C_GOLD, "%s", l.c_str() + 4); continue; }
                if (l[0] == '-') {
                    ImVec2 c = ImGui::GetCursorScreenPos() + ImVec2(S(6), hi_fs(F_SMALL) * 0.6f);
                    hi_gem(ImGui::GetWindowDrawList(), c, S(3), IM_COL32(150, 170, 200, 200));
                    ImGui::Indent(S(18));
                    ImGui::TextColored(C_DIM, "%s", l.c_str() + (l.size() > 1 && l[1] == ' ' ? 2 : 1));
                    ImGui::Unindent(S(18));
                } else ImGui::TextColored(C_DIM, "%s", l.c_str());
                ImGui::Dummy(ImVec2(0, S(2)));
            }
            ImGui::PopTextWrapPos();
            ImGui::PopFont();
            i18n_verbatim(false);
        }
    }
}

static void draw_history(App &a, ImVec2 size, SDL_Renderer *ren)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float ts = F_TITLE->LegacySize * 0.5f * ImGui::GetStyle().FontScaleMain;
    ImVec2 tp = ImGui::GetWindowPos() + ImVec2(S(28), S(16));
    const char *title = tr("History");
    dl->AddText(F_TITLE, ts, tp + ImVec2(S(1.5f), S(1.5f)), IM_COL32(0, 0, 0, 110), title);
    dl->AddText(F_TITLE, ts, tp, ImGui::GetColorU32(C_GOLD_H), title);

    const float panelW = std::min(size.x - S(60), S(1240)), panelX = (size.x - panelW) * 0.5f, top = S(84);
    const float panelH = size.y - top - S(80);
    ImVec2 panelA = ImGui::GetWindowPos() + ImVec2(panelX, top), panelB = panelA + ImVec2(panelW, panelH);
    orn_panel_fill(dl, panelA, panelB, ImGui::GetColorU32(rgb(9, 12, 20, 0.82f)));
    const float inset = S(9) * 0.5f + S(5);
    bool modal = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    const int N = k_hiCount + 1;
    if (!modal) {   // up / down (arrows, D-pad, stick) walk the timeline
        auto pressed = [](ImGuiKey k) { return ImGui::IsKeyPressed(k, ImGuiInputFlags_Repeat, ImGuiKeyOwner_Any); };
        int was = g_hiSel;
        if (pressed(ImGuiKey_UpArrow) || pressed(ImGuiKey_GamepadDpadUp) || pressed(ImGuiKey_GamepadLStickUp)) g_hiSel = std::max(0, g_hiSel - 1);
        if (pressed(ImGuiKey_DownArrow) || pressed(ImGuiKey_GamepadDpadDown) || pressed(ImGuiKey_GamepadLStickDown)) g_hiSel = std::min(N - 1, g_hiSel + 1);
        if (was != g_hiSel) g_hiScrollTop = true;
    }

    // ---- the timeline
    const float tlW = std::min(S(330), panelW * 0.32f);
    ImGui::SetCursorPos(ImVec2(panelX + inset, top + inset));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
    ImGui::BeginChild("hitl", ImVec2(tlW, panelH - inset * 2), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar);
    {
        ImDrawList *tl = ImGui::GetWindowDrawList();
        const float rowH = S(64), lineX = S(26);
        ImVec2 o = ImGui::GetCursorScreenPos();
        // the line through the gems, from the first to the last
        tl->AddLine(o + ImVec2(lineX, rowH * 0.5f), o + ImVec2(lineX, rowH * (N - 0.5f)), IM_COL32(120, 140, 170, 110), S(2));
        const float hs = hi_fs(F_HEAD), ss = hi_fs(F_SMALL);
        for (int i = 0; i < N; i++) {
            ImGui::PushID(i);
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##hi", ImVec2(ImGui::GetContentRegionAvail().x, rowH))) { g_hiSel = i; g_hiScrollTop = true; }
            bool hov = ImGui::IsItemHovered(), sel = g_hiSel == i;
            if (sel || hov)
                orn_plate(tl, p0 + ImVec2(lineX + S(16), S(4)), p0 + ImVec2(ImGui::GetItemRectSize().x, rowH - S(4)),
                          sel ? IM_COL32(30, 44, 72, 230) : IM_COL32(20, 28, 44, 180), false);
            ImU32 gem = i == 0 ? ImGui::GetColorU32(C_GOLD_H) : k_fsGem[(i - 1) % 4];
            hi_gem(tl, p0 + ImVec2(lineX, rowH * 0.5f), sel ? S(8) : S(6), gem, sel);
            const char *t1 = i == 0 ? tr("Our story") : k_hiVersions[i - 1].ver;
            const char *t2 = i == 0 ? tr("How ROTKPlus was born") : tr(k_hiVersions[i - 1].name);
            float tx = lineX + S(28);
            tl->AddText(F_HEAD, hs, p0 + ImVec2(tx, S(10)), ImGui::GetColorU32(sel ? C_GOLD_H : C_TEXT), t1);
            std::string sub = i == 0 ? std::string(t2) : std::string(t2) + "  \xc2\xb7  " + tr(k_hiVersions[i - 1].date);
            ImVec4 clip(p0.x, p0.y, p0.x + ImGui::GetItemRectSize().x - S(6), p0.y + rowH);
            tl->AddText(F_SMALL, ss, p0 + ImVec2(tx, S(12) + hs), ImGui::GetColorU32(C_DIM), sub.c_str(), nullptr, 0, &clip);
            if (sel && g_hiScrollTop) ImGui::SetScrollHereY(0.3f);
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    ImGui::PopItemFlag();

    // ---- the page of the one chosen
    ImGui::SameLine(0, S(12));
    ImVec2 cp = ImGui::GetCursorScreenPos();
    dl->AddLine(cp - ImVec2(S(6), -S(10)), cp + ImVec2(-S(6), panelH - inset * 2 - S(10)), IM_COL32(120, 140, 170, 70), S(1));
    ImGui::BeginChild("hipage", ImVec2(0, panelH - inset * 2), ImGuiChildFlags_AlwaysUseWindowPadding);
    if (g_hiScrollTop) { ImGui::SetScrollY(0); g_hiScrollTop = false; }
    {   // testing: ROTKPLUS_SCROLL = pixels down the page, a few frames after it opens (when its height is known)
        static int frames = 0;
        if (++frames == 4 && getenv("ROTKPLUS_SCROLL")) ImGui::SetScrollY((float)atof(getenv("ROTKPLUS_SCROLL")) * g_scale);
    }
    // the right stick, Page Up / Page Down scroll the page
    {
        float step = 0;
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickDown) || ImGui::IsKeyDown(ImGuiKey_PageDown)) step += S(14);
        if (ImGui::IsKeyDown(ImGuiKey_GamepadRStickUp) || ImGui::IsKeyDown(ImGuiKey_PageUp)) step -= S(14);
        if (step != 0 && !modal) ImGui::SetScrollY(std::max(0.0f, std::min(ImGui::GetScrollMaxY(), ImGui::GetScrollY() + step)));
    }
    if (g_hiSel == 0) hi_story(a, ren);
    else hi_version(k_hiVersions[g_hiSel - 1], g_hiSel - 1);
    ImGui::Dummy(ImVec2(0, S(12)));
    ImGui::EndChild();
    ImGui::PopStyleColor();
    orn_panel_frame(dl, panelA, panelB);

    modal = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    ImGui::SetCursorPos(ImVec2(S(40), size.y - S(60)));
    bool back = orn_plate_button("\xe2\x80\xb9  Back##hback", ImVec2(S(130), S(36)), true);
    if (!modal && (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false))) back = true;
    if (back) { a.history = false; a.front = true; }
}
