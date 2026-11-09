// ROTKPlus - the Fellowship page (included by pages.cpp): the players' names and statistics, read from
// ROTKPlus_players.ini in the game folder (the same file the game's Fellowship pages show and ROTKPlus writes at the
// end of every level), without starting the game. Three tabs: each player's card, the comparison, the last game.
// The file is read when the page opens; nothing is written to it.

struct FsPlayer { std::string name; std::map<std::string, long long> v; };
static std::vector<FsPlayer> g_fs;
static std::map<std::string, std::string> g_fsLast;   // [LastGame]
static std::string g_fsFor;
struct FsDeed { std::string id, title, desc, when; };
static std::vector<FsDeed> g_fsDeeds;
static void fs_forget() { g_fsFor.clear(); g_fsDeeds.clear(); }

static const char *const k_fsHero[9] = { "Aragorn", "Legolas", "Gimli", "Gandalf", "Sam", "Frodo", "Pippin", "Merry", "Faramir" };
static const struct { const char *code, *name; } k_fsLevel[] = {
    { "hel01", "Helm's Deep" }, { "ise01", "Road to Isengard" }, { "osg01", "Escape from Osgiliath" },
    { "pat01", "Paths of the Dead" }, { "pat03", "King of the Dead" }, { "min02", "The Southern Gate" },
    { "min00", "Top of the Wall" }, { "min04", "The Courtyard" }, { "pel02", "Pelennor Fields" },
    { "she01", "Shelob's Lair" }, { "cir00", "Cirith Ungol" }, { "bla01", "The Black Gate" }, { "cra01", "The Crack of Doom" },
};
// the peoples of the enemies, by the game's character types (ROTKPlus' Players.cpp)
static const struct { const char *name; int types[5]; } k_fsFoes[] = {
    { "Orcs", { 0x03, 0x1A, 0x23 } }, { "Uruks", { 0x15, 0x25, 0x26, 0x28, 0x24 } }, { "Uruk-hai", { 0x14, 0x29, 0x2A } },
    { "Berserkers", { 0x2B } }, { "The Dead", { 0x2C, 0x2D, 0x2E, 0x2F } }, { "Easterlings", { 0x34, 0x35, 0x37 } },
    { "Spiders", { 0x4A } }, { "Trolls", { 0x4C } }, { "Mumakil", { 0x32 } }, { "Nazgul", { 0x52 } },
};

static std::string fs_level_name(const std::string &code)
{
    for (auto &l : k_fsLevel) if (!_stricmp(l.code, code.c_str())) return tr(l.name);
    return code;
}

static std::vector<std::string> fs_section(const std::string &ini, const char *sec)
{
    std::vector<char> buf(1 << 16);
    DWORD n = GetPrivateProfileSectionA(sec, buf.data(), (DWORD)buf.size(), ini.c_str());
    std::vector<std::string> out;
    for (DWORD i = 0; i < n;) { std::string l(&buf[i]); i += (DWORD)l.size() + 1; if (!l.empty()) out.push_back(l); }
    return out;
}

static void fs_load(const App &a)
{
    g_fs.clear();
    g_fsLast.clear();
    g_fsFor = a.dir;
    const std::string ini = a.dir + "\\ROTKPlus_players.ini";
    std::vector<char> names(1 << 15);
    DWORD n = GetPrivateProfileSectionNamesA(names.data(), (DWORD)names.size(), ini.c_str());
    for (DWORD i = 0; i < n;) {
        std::string s(&names[i]);
        i += (DWORD)s.size() + 1;
        if (s.rfind("Stats.", 0) != 0) continue;
        FsPlayer p;
        p.name = s.substr(6);
        for (const std::string &l : fs_section(ini, s.c_str())) {
            size_t eq = l.find('=');
            if (eq != std::string::npos) p.v[l.substr(0, eq)] = atoll(l.c_str() + eq + 1);
        }
        g_fs.push_back(p);
    }
    for (const std::string &l : fs_section(ini, "LastGame")) {
        size_t eq = l.find('=');
        if (eq != std::string::npos) g_fsLast[l.substr(0, eq)] = l.substr(eq + 1);
    }
    std::sort(g_fs.begin(), g_fs.end(), [](const FsPlayer &x, const FsPlayer &y) {
        auto k = [](const FsPlayer &p) { auto it = p.v.find("Kills"); return it == p.v.end() ? 0LL : it->second; };
        return k(x) > k(y);
    });
}

static long long fs_get(const FsPlayer &p, const std::string &k) { auto it = p.v.find(k); return it == p.v.end() ? 0 : it->second; }

// 12.345 in Italian (and the other languages but English and French), 12,345 in English, 12 345 in French
static std::string fs_num(long long v)
{
    const std::string lang = i18n_current();
    const char sep = lang == "en" || lang.empty() ? ',' : lang == "fr" ? ' ' : '.';
    std::string d = std::to_string(v < 0 ? -v : v), out;
    for (size_t i = 0; i < d.size(); i++) {
        if (i && (d.size() - i) % 3 == 0) out += sep;
        out += d[i];
    }
    return v < 0 ? "-" + out : out;
}

static std::string fs_time(long long secs)
{
    char b[48];
    if (secs >= 3600) snprintf(b, sizeof b, "%lld h %lld min", secs / 3600, secs / 60 % 60);
    else snprintf(b, sizeof b, "%lld min", secs / 60);
    return b;
}

static int fs_favourite(const FsPlayer &p)
{
    int best = -1;
    long long n = 0;
    for (int h = 0; h < 9; h++) {
        char k1[16], k2[16];
        snprintf(k1, sizeof k1, "Hero%d", h);
        snprintf(k2, sizeof k2, "Hero%dPvp", h);
        long long x = fs_get(p, k1) + fs_get(p, k2);
        if (x > n) { n = x; best = h; }
    }
    return best;
}

static long long fs_foes(const FsPlayer &p, int people)
{
    long long n = 0;
    for (int t : k_fsFoes[people].types) {
        if (!t) continue;
        char k[16];
        snprintf(k, sizeof k, "Foe%02X", t);
        n += fs_get(p, k);
    }
    return n;
}

// a bar: its share of the widest, a number at its end
static void fs_bar(const char *label, long long v, long long most, ImU32 col)
{
    ImGui::TextUnformatted(label);
    ImGui::SameLine(S(150));
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x - S(70), h = ImGui::GetTextLineHeight() * 0.7f;
    float f = most > 0 ? (float)v / (float)most : 0.0f;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p + ImVec2(0, S(4)), p + ImVec2(w, S(4) + h), IM_COL32(30, 40, 60, 200), S(3));
    if (f > 0) dl->AddRectFilled(p + ImVec2(0, S(4)), p + ImVec2(std::max(S(3), w * f), S(4) + h), col, S(3));
    ImGui::Dummy(ImVec2(w, h + S(6)));
    ImGui::SameLine();
    ImGui::TextUnformatted(fs_num(v).c_str());
}

static const ImU32 k_fsGem[4] = { IM_COL32(232, 140, 40, 255), IM_COL32(80, 120, 235, 255), IM_COL32(40, 170, 90, 255),
                                  IM_COL32(240, 200, 60, 255) };

// ROTKPlus: the colour of the i-th player of the Fellowship (by name, the most kills first): the four players' gems, then
// three more, so that a fifth or sixth friend is not mistaken for the first
static ImU32 fs_gem(size_t i)
{
    static const ImU32 more[3] = { IM_COL32(170, 100, 230, 255), IM_COL32(225, 85, 85, 255), IM_COL32(60, 190, 200, 255) };
    return i < 4 ? k_fsGem[i] : more[(i - 4) % 3];
}

static SDL_Texture *fs_hero_tex(const App &a, int hero);   // below: the heroes' portraits
static int fs_deeds_got(const FsPlayer &p, int *all);       // below: the player's deeds earned (and how many there are)

// The title the deeds earn (ROTKPlus' own deeds of the player): on the cards and the picture to share
static const char *fs_rank(int deeds, int all)
{
    return deeds >= all ? "Hero of Middle-earth" : deeds >= 13 ? "Lord of the West" : deeds >= 9 ? "Captain of Gondor"
         : deeds >= 5 ? "Guard of the Citadel" : "Soldier of Gondor";
}

// a diamond, filled or only drawn
static void fs_diamond(ImDrawList *dl, ImVec2 c, float r, ImU32 col, bool filled = true)
{
    ImVec2 d[4] = { ImVec2(c.x - r, c.y), ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r) };
    if (filled) dl->AddConvexPolyFilled(d, 4, col);
    else dl->AddPolyline(d, 4, col, S(1.2f), ImDrawFlags_Closed);
}

// the hero's portrait in a frame of the player's colour (or a large gem, before ROTKPlus has made the portraits)
static void fs_big_portrait(ImDrawList *dl, const App &a, int hero, ImVec2 p0, float side, ImU32 gem)
{
    ImVec2 p1 = p0 + ImVec2(side, side);
    dl->AddRectFilled(p0 - ImVec2(S(6), S(6)), p1 + ImVec2(S(6), S(6)), IM_COL32(6, 9, 16, 220), S(5));
    if (SDL_Texture *t = fs_hero_tex(a, hero)) dl->AddImage((ImTextureID)(intptr_t)t, p0, p1);
    else fs_diamond(dl, p0 + ImVec2(side, side) * 0.5f, side * 0.2f, gem);
    dl->AddRect(p0, p1, gem, S(3), S(2.5f));
    dl->AddRect(p0 - ImVec2(S(6), S(6)), p1 + ImVec2(S(6), S(6)), IM_COL32(170, 190, 220, 110), S(5), S(1));
}

// text in a font at a size, shrunk to fit a width
static float fs_fit(ImFont *f, float sz, const char *t, float room)
{
    float w = f->CalcTextSizeA(sz, FLT_MAX, 0, t).x;
    return w > room && w > 0 ? sz * room / w : sz;
}

// ROTKPlus: one card per player, in the style of the picture to share: the favourite hero's portrait in the player's
// colour, the name, the title the deeds earn and the hero, four numbers large, the others small under them, the
// picture to share at the top right; the enemies by people folded under the card.
static void fs_cards(App &a)
{
    if (g_fs.empty()) {
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(C_DIM, "No statistics yet: they are written at the end of every level played with ROTKPlus "
                                  "(ROTKPlus_players.ini in the game folder).");
        ImGui::PopTextWrapPos();
        return;
    }
    const float fsS = ImGui::GetStyle().FontScaleMain;
    for (size_t i = 0; i < g_fs.size(); i++) {
        const FsPlayer &p = g_fs[i];
        const ImU32 gem = fs_gem(i);
        ImGui::PushID((int)i);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        const float W = ImGui::GetContentRegionAvail().x, H = S(184);
        const ImVec2 p0 = ImGui::GetCursorScreenPos(), p1 = p0 + ImVec2(W, H);
        ImGui::Dummy(ImVec2(W, H));
        const ImVec2 after = ImGui::GetCursorScreenPos();
        // the card: a dark plate, a stripe of the player's colour on the left, a thin silver line
        dl->AddRectFilled(p0, p1, IM_COL32(12, 17, 30, 205), S(6));
        dl->AddRectFilled(p0, ImVec2(p0.x + S(5), p1.y), gem, S(6), ImDrawFlags_RoundTopLeft | ImDrawFlags_RoundBottomLeft);
        dl->AddRect(p0, p1, IM_COL32(90, 110, 140, 150), S(6), S(1));
        // the portrait: small, no larger than the game's own HUD icons (128 pixels) on a 1080p screen, as they blow up
        // badly; centred
        const int fav = fs_favourite(p);
        const float side = S(72);
        const ImVec2 q0 = p0 + ImVec2(S(32), (H - side) * 0.5f);
        fs_big_portrait(dl, a, fav, q0, side, gem);
        // the name, then the title and the hero
        const float x = q0.x + side + S(30), right = p1.x - S(18);
        int all = 0, got = fs_deeds_got(p, &all);
        const float bw = ImGui::CalcTextSize(tr("Picture to share")).x + ImGui::GetStyle().FramePadding.x * 2;
        const float ns = fs_fit(F_TITLE, F_TITLE->LegacySize * fsS * 0.62f, p.name.c_str(), right - x - bw - S(16));
        dl->AddText(F_TITLE, ns, ImVec2(x + S(1.5f), p0.y + S(12) + S(1.5f)), IM_COL32(0, 0, 0, 140), p.name.c_str());
        dl->AddText(F_TITLE, ns, ImVec2(x, p0.y + S(12)), ImGui::GetColorU32(C_GOLD_H), p.name.c_str());
        const float ty = p0.y + S(16) + ns;
        fs_diamond(dl, ImVec2(x + S(5), ty + S(11)), S(5), gem);
        std::string rank = std::string(tr(fs_rank(got, all))) + "   \xc2\xb7   " + (fav >= 0 ? tr(k_fsHero[fav]) : "-");
        const float rs = F_HEAD->LegacySize * fsS;
        dl->AddText(F_HEAD, rs, ImVec2(x + S(18), ty), ImGui::GetColorU32(C_TEXT), rank.c_str());
        // four numbers large, each with its label under it
        const std::string val[4] = { fs_num(fs_get(p, "Kills")), fs_num(fs_get(p, "Wins")), fs_time(fs_get(p, "Seconds")),
                                     fs_num(fs_get(p, "BestKills")) };
        const char *lab[4] = { "Enemies", "Levels completed", "Time played", "Best level" };
        const float cw = (right - x) / 4, ny = ty + rs + S(16);
        for (int k = 0; k < 4; k++) {
            float cx = x + cw * (float)k;
            dl->AddLine(ImVec2(cx, ny), ImVec2(cx + S(26), ny), gem, S(2));
            float vs = fs_fit(F_HEAD, F_HEAD->LegacySize * fsS * 1.45f, val[k].c_str(), cw - S(12));
            dl->AddText(F_HEAD, vs, ImVec2(cx, ny + S(6)), ImGui::GetColorU32(C_TEXT), val[k].c_str());
            float ls = fs_fit(F_SMALL, F_SMALL->LegacySize * fsS, tr(lab[k]), cw - S(12));
            dl->AddText(F_SMALL, ls, ImVec2(cx, ny + S(10) + vs), ImGui::GetColorU32(C_DIM), tr(lab[k]));
        }
        // the rest, small, at the bottom of the card
        char rest[256];
        if (fs_get(p, "PvpGames"))
            snprintf(rest, sizeof rest, "%s %s   \xc2\xb7   %s %s   \xc2\xb7   %s %s   \xc2\xb7   %s %s / %s   \xc2\xb7   %s %s",
                     tr("Assists:"), fs_num(fs_get(p, "Assists")).c_str(), tr("Falls:"), fs_num(fs_get(p, "Deaths")).c_str(),
                     tr("Experience:"), fs_num(fs_get(p, "Experience")).c_str(), tr("Versus won:"),
                     fs_num(fs_get(p, "PvpWins")).c_str(), fs_num(fs_get(p, "PvpGames")).c_str(), tr("Cups:"),
                     fs_num(fs_get(p, "PvpCups")).c_str());
        else
            snprintf(rest, sizeof rest, "%s %s   \xc2\xb7   %s %s   \xc2\xb7   %s %s", tr("Assists:"),
                     fs_num(fs_get(p, "Assists")).c_str(), tr("Falls:"), fs_num(fs_get(p, "Deaths")).c_str(),
                     tr("Experience:"), fs_num(fs_get(p, "Experience")).c_str());
        const float ss = fs_fit(F_SMALL, F_SMALL->LegacySize * fsS, rest, right - x);
        dl->AddText(F_SMALL, ss, ImVec2(x, p1.y - S(16) - ss), ImGui::GetColorU32(C_DIM), rest);
        // the picture to share, at the top right
        ImGui::SetCursorScreenPos(ImVec2(right - bw, p0.y + S(14)));
        if (ImGui::Button("Picture to share")) share_ask(a, (int)i);
        ImGui::SetCursorScreenPos(after);
        if (getenv("ROTKPLUS_FOES") && *getenv("ROTKPLUS_FOES")) ImGui::SetNextItemOpen(true, ImGuiCond_Once);   // (tests)
        if (ImGui::TreeNode(tr("Enemies by people"))) {
            // the most defeated first, as in the picture to share (equal numbers keep the table's order)
            std::vector<std::pair<long long, int>> foes;
            for (int f = 0; f < (int)(sizeof k_fsFoes / sizeof k_fsFoes[0]); f++)
                if (long long n = fs_foes(p, f)) foes.push_back({ n, f });
            std::stable_sort(foes.begin(), foes.end(), [](const std::pair<long long, int> &l, const std::pair<long long, int> &r) {
                return l.first > r.first;
            });
            for (auto &f : foes) fs_bar(tr(k_fsFoes[f.second].name), f.first, foes[0].first, gem);
            ImGui::TreePop();
        }
        ImGui::Dummy(ImVec2(0, S(8)));
        ImGui::PopID();
    }
}

static void fs_compare(App &a)
{
    if (g_fs.empty()) { ImGui::TextColored(C_DIM, "%s", tr("No statistics yet.")); return; }
    static const struct { const char *label, *key; } rows[] = {
        { "Enemies", "Kills" }, { "Assists", "Assists" }, { "Levels completed", "Wins" }, { "Experience", "Experience" },
        { "Speed attacks that hit", "SpeedHits" }, { "Fierce attacks that hit", "FierceHits" }, { "Shields broken", "ShieldsBroken" },
        { "Kicks", "Kicks" }, { "At range", "RangedKills" }, { "Ladders pushed down", "Ladders" }, { "Falls", "Deaths" },
        { "Versus won", "PvpWins" }, { "Cups", "PvpCups" }, { "Best level", "BestKills" },
    };
    const int cols = 1 + (int)std::min<size_t>(g_fs.size(), 6);
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.06f, 0.09f, 0.16f, 0.85f));   // (night blue, not grey)
    if (ImGui::BeginTable("cmp", cols, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        for (int c = 1; c < cols; c++) ImGui::TableSetupColumn(g_fs[(size_t)c - 1].name.c_str());
        // ROTKPlus: the heading is each player's portrait over the name, in the player's colour
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int c = 1; c < cols; c++) {
            ImGui::TableSetColumnIndex(c);
            const FsPlayer &p = g_fs[(size_t)c - 1];
            const float side = S(46);
            ImVec2 q = ImGui::GetCursorScreenPos() + ImVec2(S(8), S(8));
            fs_big_portrait(ImGui::GetWindowDrawList(), a, fs_favourite(p), q, side, fs_gem((size_t)c - 1));
            ImGui::Dummy(ImVec2(side + S(16), side + S(16)));
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(fs_gem((size_t)c - 1)), "%s", p.name.c_str());
        }
        for (auto &r : rows) {
            long long best = 0;
            for (int c = 1; c < cols; c++) best = std::max(best, fs_get(g_fs[(size_t)c - 1], r.key));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(tr(r.label));
            for (int c = 1; c < cols; c++) {
                ImGui::TableSetColumnIndex(c);
                long long v = fs_get(g_fs[(size_t)c - 1], r.key);
                if (v == best && v > 0) {
                    ImVec2 q = ImGui::GetCursorScreenPos() + ImVec2(S(5), ImGui::GetTextLineHeight() * 0.5f);
                    fs_diamond(ImGui::GetWindowDrawList(), q, S(4), fs_gem((size_t)c - 1));
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + S(14));
                    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(fs_gem((size_t)c - 1)), "%s", fs_num(v).c_str());
                } else {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + S(14));
                    ImGui::TextUnformatted(fs_num(v).c_str());
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor();
    ImGui::TextColored(C_DIM, "%s", tr("The best of each row has a gem in that player's colour."));
}

// ---- Deeds
// The game's: ROTK Fix keeps them in ROTKFix\achievements.txt in the game folder ([list] id = title | description,
// [unlocked] id = when). Fate's are left out (Fate was a choice of Athelas' Journey page, not in this launcher).
// The players': ROTKPlus' own, each player's from their statistics, with the progress towards it.

static void fs_load_deeds(const App &a)
{
    g_fsDeeds.clear();
    const std::string ini = a.dir + "\\ROTKFix\\achievements.txt";
    std::map<std::string, std::string> when;
    for (const std::string &l : fs_section(ini, "unlocked")) {
        size_t eq = l.find('=');
        if (eq == std::string::npos) continue;
        std::string k = l.substr(0, eq), v = l.substr(eq + 1);
        while (!k.empty() && k.back() == ' ') k.pop_back();
        while (!v.empty() && v[0] == ' ') v.erase(0, 1);
        when[k] = v;
    }
    for (const std::string &l : fs_section(ini, "list")) {
        size_t eq = l.find('='), bar = l.find('|');
        if (eq == std::string::npos || bar == std::string::npos || bar < eq) continue;
        FsDeed d;
        d.id = l.substr(0, eq);
        while (!d.id.empty() && d.id.back() == ' ') d.id.pop_back();
        if (d.id.rfind("fate", 0) == 0) continue;
        d.title = l.substr(eq + 1, bar - eq - 1);
        d.desc = l.substr(bar + 1);
        for (std::string *t : { &d.title, &d.desc }) {
            while (!t->empty() && t->back() == ' ') t->pop_back();
            while (!t->empty() && (*t)[0] == ' ') t->erase(0, 1);
        }
        auto w = when.find(d.id);
        if (w != when.end()) d.when = w->second;
        g_fsDeeds.push_back(d);
    }
}

struct FsPlayerDeed { const char *title, *desc; long long (*value)(const FsPlayer &); long long need; };
static long long fsv_kills(const FsPlayer &p) { return fs_get(p, "Kills"); }
static long long fsv_best(const FsPlayer &p) { return fs_get(p, "BestKills"); }
static long long fsv_assists(const FsPlayer &p) { return fs_get(p, "Assists"); }
static long long fsv_shields(const FsPlayer &p) { return fs_get(p, "ShieldsBroken"); }
static long long fsv_ladders(const FsPlayer &p) { return fs_get(p, "Ladders"); }
static long long fsv_ranged(const FsPlayer &p) { return fs_get(p, "RangedKills"); }
static long long fsv_kicks(const FsPlayer &p) { return fs_get(p, "KicksHit"); }
static long long fsv_wins(const FsPlayer &p) { return fs_get(p, "Wins"); }
static long long fsv_hours(const FsPlayer &p) { return fs_get(p, "Seconds") / 3600; }
static long long fsv_pvp(const FsPlayer &p) { return fs_get(p, "PvpWins"); }
static long long fsv_cups(const FsPlayer &p) { return fs_get(p, "PvpCups"); }
static long long fsv_downs(const FsPlayer &p) { return fs_get(p, "PvpKills"); }
static long long fsv_heroes(const FsPlayer &p)   // heroes played at least once (a level or a versus game)
{
    long long n = 0;
    for (int h = 0; h < 9; h++) {
        char k1[16], k2[16];
        snprintf(k1, sizeof k1, "Hero%d", h);
        snprintf(k2, sizeof k2, "Hero%dPvp", h);
        if (fs_get(p, k1) + fs_get(p, k2) > 0) n++;
    }
    return n;
}
static long long fsv_dead(const FsPlayer &p) { return fs_foes(p, 4); }   // the Dead (k_fsFoes' order)

static const FsPlayerDeed k_fsPlayerDeeds[] = {
    { "First blood", "Defeat 10 enemies.", fsv_kills, 10 },
    { "A hundred orcs", "Defeat 100 enemies.", fsv_kills, 100 },
    { "A thousand foes", "Defeat 1,000 enemies.", fsv_kills, 1000 },
    { "Five thousand", "Defeat 5,000 enemies.", fsv_kills, 5000 },
    { "A day of wrath", "Defeat 100 enemies in one level.", fsv_best, 100 },
    { "Shoulder to shoulder", "Help a companion to 50 kills.", fsv_assists, 50 },
    { "Shield breaker", "Break 50 shields.", fsv_shields, 50 },
    { "Keeper of the wall", "Push down 25 ladders.", fsv_ladders, 25 },
    { "Never misses", "Defeat 100 enemies at range.", fsv_ranged, 100 },
    { "Boots of Gondor", "Land 100 kicks.", fsv_kicks, 100 },
    { "Oathbreakers' bane", "Defeat 100 of the Dead.", fsv_dead, 100 },
    { "The whole road", "Complete 13 levels.", fsv_wins, 13 },
    { "The Fellowship", "Play every hero.", fsv_heroes, 9 },
    { "Veteran", "Play for 10 hours.", fsv_hours, 10 },
    { "Duellist", "Win 10 versus games.", fsv_pvp, 10 },
    { "Champion", "Win a tournament's cup.", fsv_cups, 1 },
    { "Warlord", "Knock down 100 opponents in Versus.", fsv_downs, 100 },
};

static int fs_deeds_got(const FsPlayer &p, int *all)
{
    int got = 0;
    for (const auto &d : k_fsPlayerDeeds) if (d.value(p) >= d.need) got++;
    if (all) *all = (int)(sizeof k_fsPlayerDeeds / sizeof k_fsPlayerDeeds[0]);
    return got;
}

static void fs_deeds(App &a)
{
    if (g_fsDeeds.empty()) fs_load_deeds(a);
    section(tr("The players' deeds"));
    if (g_fs.empty()) ImGui::TextColored(C_DIM, "%s", tr("No statistics yet."));
    // ROTKPlus: a row per player as on the cards (portrait, name, title, the deeds as gems); a click opens the list
    const float fsS = ImGui::GetStyle().FontScaleMain;
    for (size_t i = 0; i < g_fs.size(); i++) {
        const FsPlayer &p = g_fs[i];
        const ImU32 gem = fs_gem(i);
        int all = 0, got = fs_deeds_got(p, &all);
        ImGui::PushID((int)i);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        const float W = ImGui::GetContentRegionAvail().x, H = S(66);
        const ImVec2 p0 = ImGui::GetCursorScreenPos(), p1 = p0 + ImVec2(W, H);
        ImGuiStorage *st = ImGui::GetStateStorage();
        const ImGuiID sid = ImGui::GetID("open");
        bool open = st->GetBool(sid, false);
        if (ImGui::InvisibleButton("##row", ImVec2(W, H))) { open = !open; st->SetBool(sid, open); }
        const bool hov = ImGui::IsItemHovered() || ImGui::IsItemFocused();
        dl->AddRectFilled(p0, p1, hov ? IM_COL32(22, 30, 50, 225) : IM_COL32(12, 17, 30, 205), S(6));
        dl->AddRectFilled(p0, ImVec2(p0.x + S(5), p1.y), gem, S(6), ImDrawFlags_RoundTopLeft | ImDrawFlags_RoundBottomLeft);
        dl->AddRect(p0, p1, hov ? ImGui::GetColorU32(C_GOLD_H) : IM_COL32(90, 110, 140, 150), S(6), S(1));
        const float side = H - S(22);
        fs_big_portrait(dl, a, fs_favourite(p), p0 + ImVec2(S(20), S(11)), side, gem);
        const float x = p0.x + S(20) + side + S(20);
        const float ns = F_HEAD->LegacySize * fsS * 1.15f, rs = F_SMALL->LegacySize * fsS;
        dl->AddText(F_HEAD, ns, ImVec2(x, p0.y + S(9)), ImGui::GetColorU32(C_GOLD_H), p.name.c_str());
        dl->AddText(F_SMALL, rs, ImVec2(x, p0.y + S(13) + ns), ImGui::GetColorU32(C_DIM), tr(fs_rank(got, all)));
        // on the right: the arrow, the count, the gems
        const float cy = p0.y + H * 0.5f;
        float rx = p1.x - S(20);
        ImVec2 tri[3];
        if (open) { tri[0] = ImVec2(rx - S(6), cy - S(3)); tri[1] = ImVec2(rx + S(6), cy - S(3)); tri[2] = ImVec2(rx, cy + S(4)); }
        else      { tri[0] = ImVec2(rx - S(3), cy - S(6)); tri[1] = ImVec2(rx + S(4), cy); tri[2] = ImVec2(rx - S(3), cy + S(6)); }
        dl->AddTriangleFilled(tri[0], tri[1], tri[2], ImGui::GetColorU32(C_DIM));
        char cnt[24];
        snprintf(cnt, sizeof cnt, "%d / %d", got, all);
        const float cs = F_HEAD->LegacySize * fsS;
        const float cwid = F_HEAD->CalcTextSizeA(cs, FLT_MAX, 0, cnt).x;
        rx -= S(22) + cwid;
        dl->AddText(F_HEAD, cs, ImVec2(rx, cy - cs * 0.55f), ImGui::GetColorU32(C_TEXT), cnt);
        const float step = S(15), gx0 = rx - S(18) - step * (float)(all - 1);
        if (gx0 > x + S(160))
            for (int k = 0; k < all; k++)
                fs_diamond(dl, ImVec2(gx0 + step * (float)k, cy), S(5.5f), k < got ? gem : IM_COL32(150, 162, 182, 170), k < got);
        if (open) {
            ImGui::Indent(S(24));
            for (const auto &d : k_fsPlayerDeeds) {
                long long v = d.value(p);
                bool done = v >= d.need;
                ImVec2 c = ImGui::GetCursorScreenPos() + ImVec2(S(6), ImGui::GetTextLineHeight() * 0.55f);
                fs_diamond(dl, c, S(5), done ? gem : IM_COL32(150, 162, 182, 170), done);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + S(20));
                ImGui::TextColored(done ? C_GOLD_H : C_DIM, "%s", tr(d.title));
                ImGui::SameLine(S(280));
                if (done) ImGui::TextColored(C_DIM, "%s", tr(d.desc));
                else ImGui::TextColored(C_DIM, "%s  (%s / %s)", tr(d.desc), fs_num(v).c_str(), fs_num(d.need).c_str());
            }
            ImGui::Unindent(S(24));
        }
        ImGui::Dummy(ImVec2(0, S(6)));
        ImGui::PopID();
    }
    section(tr("The game's deeds"));
    if (g_fsDeeds.empty()) {
        ImGui::PushTextWrapPos(0);
        ImGui::TextColored(C_DIM, "%s", tr("None yet: ROTK Fix writes them in ROTKFix\\achievements.txt as you play."));
        ImGui::PopTextWrapPos();
        return;
    }
    int got = 0;
    for (const auto &d : g_fsDeeds) if (!d.when.empty()) got++;
    ImGui::TextColored(C_DIM, "%s %d / %d", tr("Earned:"), got, (int)g_fsDeeds.size());
    for (const auto &d : g_fsDeeds) {
        bool done = !d.when.empty();
        ImGui::TextColored(done ? C_GOLD_H : C_DIM, "%s %s", done ? "\xe2\x97\x86" : "\xe2\x97\x87", tr(d.title.c_str()));
        ImGui::SameLine(S(260));
        ImGui::TextColored(C_DIM, "%s%s%s", tr(d.desc.c_str()), done ? "  -  " : "", done ? d.when.c_str() : "");
    }
}

// the last game, also shown on the front screen
// The heroes' portraits: ROTKPlus makes them on the player's own PC from the game's HUD (ROTKPlus_icons\Hero<n>.raw in
// the game folder: "RPI1", width and height as 32-bit numbers, then the pixels as B, G, R, A). Nothing of the game is
// shipped: without the files (the mod not played yet) the rows just have no picture.
static SDL_Texture *g_fsHeroTex[9];
static bool g_fsHeroTried[9];
static std::string g_fsHeroFor;
static SDL_Texture *fs_hero_tex(const App &a, int hero)
{
    if (hero < 0 || hero > 8 || a.dir.empty()) return nullptr;
    if (g_fsHeroFor != a.dir) {
        for (int i = 0; i < 9; i++) { if (g_fsHeroTex[i]) SDL_DestroyTexture(g_fsHeroTex[i]); g_fsHeroTex[i] = nullptr; g_fsHeroTried[i] = false; }
        g_fsHeroFor = a.dir;
    }
    if (g_fsHeroTried[hero]) return g_fsHeroTex[hero];
    g_fsHeroTried[hero] = true;
    FILE *f = fopen((a.dir + "\\ROTKPlus_icons\\Hero" + std::to_string(hero) + ".raw").c_str(), "rb");
    if (!f) return nullptr;
    char magic[4];
    uint32_t w = 0, h = 0;
    if (fread(magic, 1, 4, f) == 4 && !memcmp(magic, "RPI1", 4) && fread(&w, 4, 1, f) == 1 && fread(&h, 4, 1, f) == 1 &&
        w > 0 && h > 0 && w <= 1024 && h <= 1024) {
        std::vector<unsigned char> px((size_t)w * h * 4);
        if (fread(px.data(), 1, px.size(), f) == px.size()) {
            for (size_t i = 0; i < px.size(); i += 4) std::swap(px[i], px[i + 2]);   // B, G, R, A -> R, G, B, A
            if (SDL_Renderer *ren = SDL_GetRenderer(g_mainWin)) g_fsHeroTex[hero] = make_texture(ren, px.data(), (int)w, (int)h);
        }
    }
    fclose(f);
    return g_fsHeroTex[hero];
}

// a portrait in a small frame (or, without one, a gem in the player's colour)
static void fs_portrait(ImDrawList *dl, const App &a, int hero, ImVec2 p0, float side, ImU32 gem)
{
    ImVec2 p1 = p0 + ImVec2(side, side);
    if (SDL_Texture *t = fs_hero_tex(a, hero)) {
        dl->AddRectFilled(p0, p1, IM_COL32(6, 9, 16, 220), S(3));
        dl->AddImage((ImTextureID)(intptr_t)t, p0, p1);
        dl->AddRect(p0, p1, gem, S(3), S(1.5f));
    } else {
        ImVec2 c = p0 + ImVec2(side, side) * 0.5f;
        float r = side * 0.18f;
        ImVec2 d[4] = { ImVec2(c.x - r, c.y), ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r) };
        dl->AddConvexPolyFilled(d, 4, gem);
    }
}

// the last game's players: name, hero, kills (knock-downs in Versus), falls
struct FsRow { std::string name; int hero; std::string kills, falls; };
static bool fs_last_rows(std::string &level, std::string &how, std::vector<FsRow> &rows)
{
    auto kind = g_fsLast.find("Kind");
    if (kind == g_fsLast.end() || kind->second == "0") return false;
    int k = atoi(kind->second.c_str());
    level = fs_level_name(g_fsLast["Level"]);
    how = tr(k == 1 ? "level completed" : k == 2 ? "level left" : "Versus game");
    if (!g_fsLast["Result"].empty()) how += "  -  " + g_fsLast["Result"];
    for (int i = 0; i < 4; i++) {
        auto it = g_fsLast.find("R" + std::to_string(i));
        if (it == g_fsLast.end()) continue;
        char buf[160];
        strncpy(buf, it->second.c_str(), sizeof buf - 1);
        buf[sizeof buf - 1] = 0;
        const char *f[9] = {};
        char *save = nullptr;
        for (int j = 0; j < 9; j++) f[j] = strtok_s(j ? nullptr : buf, "|", &save);
        if (!f[4]) continue;
        rows.push_back({ f[0], atoi(f[1]), std::string(f[2]) + " " + tr(k == 3 ? "down" : "enemies"),
                         std::string(f[4]) + " " + tr(atoi(f[4]) == 1 ? "fall" : "falls") });
    }
    return true;
}

static bool fs_last_lines(std::string &title, std::vector<std::string> &lines)
{
    auto kind = g_fsLast.find("Kind");
    if (kind == g_fsLast.end() || kind->second == "0") return false;
    int k = atoi(kind->second.c_str());
    title = fs_level_name(g_fsLast["Level"]) + "  -  " +
            tr(k == 1 ? "level completed" : k == 2 ? "level left" : "Versus game");
    if (!g_fsLast["Result"].empty()) title += "  -  " + g_fsLast["Result"];
    for (int i = 0; i < 4; i++) {
        auto it = g_fsLast.find("R" + std::to_string(i));
        if (it == g_fsLast.end()) continue;
        char buf[160];
        strncpy(buf, it->second.c_str(), sizeof buf - 1);
        buf[sizeof buf - 1] = 0;
        const char *f[9] = {};
        char *save = nullptr;
        for (int j = 0; j < 9; j++) f[j] = strtok_s(j ? nullptr : buf, "|", &save);
        if (!f[4]) continue;
        int hero = atoi(f[1]);
        char l[160];
        snprintf(l, sizeof l, "%s (%s): %s %s, %s %s", f[0], hero >= 0 && hero < 9 ? k_fsHero[hero] : "?", f[2],
                 tr(k == 3 ? "down" : "enemies"), f[4], tr(atoi(f[4]) == 1 ? "fall" : "falls"));
        lines.push_back(l);
    }
    return true;
}

static void fs_last(App &a)
{
    std::string title;
    std::vector<std::string> lines;
    if (!fs_last_lines(title, lines)) { ImGui::TextColored(C_DIM, "%s", tr("No game played with ROTKPlus yet.")); return; }
    ImGui::PushFont(F_HEAD, F_HEAD->LegacySize);
    ImGui::TextColored(C_GOLD_H, "%s", title.c_str());
    ImGui::PopFont();
    std::string lv, how;
    std::vector<FsRow> rows;
    fs_last_rows(lv, how, rows);
    ImGui::Dummy(ImVec2(0, S(6)));
    for (size_t i = 0; i < lines.size(); i++) {
        const float side = S(48);
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        fs_portrait(ImGui::GetWindowDrawList(), a, i < rows.size() ? rows[i].hero : -1, p0, side, k_fsGem[i % 4]);
        ImGui::SetCursorScreenPos(p0 + ImVec2(side + S(14), (side - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::TextUnformatted(lines[i].c_str());
        ImGui::SetCursorScreenPos(p0 + ImVec2(0, side + S(8)));
        ImGui::Dummy(ImVec2(0, 0));
    }
}

static void draw_fellowship(App &a, ImVec2 size, SDL_Renderer *)
{
    if (g_fsFor != a.dir) fs_load(a);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float ts = F_TITLE->LegacySize * 0.5f * ImGui::GetStyle().FontScaleMain;
    ImVec2 tp = ImGui::GetWindowPos() + ImVec2(S(28), S(16));
    const char *title = tr("The Fellowship");
    dl->AddText(F_TITLE, ts, tp + ImVec2(S(1.5f), S(1.5f)), IM_COL32(0, 0, 0, 110), title);
    dl->AddText(F_TITLE, ts, tp, ImGui::GetColorU32(C_GOLD_H), title);

    const float panelW = std::min(size.x - S(60), S(760)), panelX = (size.x - panelW) * 0.5f, top = S(84);
    const float panelH = size.y - top - S(80);
    ImVec2 panelA = ImGui::GetWindowPos() + ImVec2(panelX, top), panelB = panelA + ImVec2(panelW, panelH);
    orn_panel_fill(dl, panelA, panelB, ImGui::GetColorU32(rgb(9, 12, 20, 0.80f)));
    const float inset = S(9) * 0.5f + S(5);
    ImGui::SetCursorPos(ImVec2(panelX + inset, top + inset));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("fellowship", ImVec2(panelW - inset * 2, panelH - inset * 2), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened);
    static const char *names[] = { "Players", "Comparison", "Deeds", "Last game" };
    const int N = 4;
    static int sel = 0;
    ImGui::PushFont(F_TAB, F_TAB->LegacySize);
    float tabW = tabs_begin(names, N, sel);
    if (ImGui::BeginTabBar("ftabs")) {
        int startTab = -1;
        if (int step = pad_tab_step(a)) startTab = (sel + step + N) % N;
        static bool firstEnv = true;   // testing: ROTKPLUS_FTAB = n opens the Fellowship on tab n
        if (firstEnv) { firstEnv = false; if (const char *ft = getenv("ROTKPLUS_FTAB")) startTab = std::max(0, std::min(N - 1, atoi(ft))); }
        static const char *ids[] = { "###fplayers", "###fcompare", "###fdeeds", "###flast" };
        for (int t = 0; t < N; t++) {
            ImGui::SetNextItemWidth(tabW);
            if (ImGui::BeginTabItem(ids[t], nullptr, t == startTab ? ImGuiTabItemFlags_SetSelected : 0)) {
                sel = t;
                pad_tab_focus();
                ImGui::PopFont();
                ImGui::BeginChild("ftab", ImVec2(0, 0), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_NoBackground);
                if (t == 0) fs_cards(a); else if (t == 1) fs_compare(a); else if (t == 2) fs_deeds(a); else fs_last(a);
                ImGui::Dummy(ImVec2(0, S(6)));
                ImGui::EndChild();
                ImGui::PushFont(F_TAB, F_TAB->LegacySize);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    tabs_end();
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    orn_panel_frame(dl, panelA, panelB);

    bool modal = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    share_frame(a);   // (after: the Escape that closes its window does not also leave the page)
    ImGui::SetCursorPos(ImVec2(S(40), size.y - S(60)));
    bool back = orn_plate_button("\xe2\x80\xb9  Back##fback", ImVec2(S(130), S(36)), true);
    if (!modal && (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false))) back = true;
    if (back) { a.fellowship = false; a.front = true; }
}

// The front screen's last game: a small panel on the left, when there is room (draw_front calls it)
static void front_last_game(App &a, ImVec2 pos, ImVec2 sz)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos();
    float x = pos.x, y = pos.y, w = sz.x, h = sz.y;
    orn_plate(dl, o + ImVec2(x, y), o + ImVec2(x + w, y + h), IM_COL32(9, 12, 20, 200), false);
    ImFont *f = F_SMALL;
    float fs = f->LegacySize * ImGui::GetStyle().FontScaleMain, lh = fs * 1.25f;
    std::string title;
    std::vector<std::string> lines;
    if (a.dir.empty() || (g_fsFor != a.dir && (fs_load(a), false)) || !fs_last_lines(title, lines)) {
        dl->AddText(F_HEAD, F_HEAD->LegacySize * ImGui::GetStyle().FontScaleMain, o + ImVec2(x + S(14), y + S(10)),
                    ImGui::GetColorU32(C_GOLD_H), tr("Last game"));
        ImVec4 clip0(o.x + x, o.y + y, o.x + x + w - S(10), o.y + y + h);
        dl->AddText(f, fs, o + ImVec2(x + S(14), y + S(42)), ImGui::GetColorU32(C_DIM), tr("None yet: play a level with ROTKPlus."),
                    nullptr, w - S(28), &clip0);
        return;
    }
    std::string level, how;
    std::vector<FsRow> rows;
    fs_last_rows(level, how, rows);
    ImVec4 clip(o.x + x, o.y + y, o.x + x + w - S(10), o.y + y + h);
    const float hs = F_HEAD->LegacySize * ImGui::GetStyle().FontScaleMain;
    dl->AddText(F_HEAD, hs, o + ImVec2(x + S(14), y + S(10)), ImGui::GetColorU32(C_GOLD_H), tr("Last game"), nullptr, 0, &clip);
    // the level and how it ended, on one line under the heading
    std::string sub = level + "  \xc2\xb7  " + how;
    dl->AddText(f, fs, o + ImVec2(x + S(14), y + S(16) + hs), ImGui::GetColorU32(C_TEXT), sub.c_str(), nullptr, 0, &clip);
    // the players: a portrait, the name and hero, the numbers; two columns from three players on
    const int n = std::min(4, (int)rows.size());
    const int cols = n > 2 ? 2 : 1;
    const float top = y + S(22) + hs + lh, rowsH = y + h - S(10) - top;
    const int perCol = (n + cols - 1) / cols;
    const float rh = perCol ? std::min(S(52), rowsH / perCol) : 0, side = std::max(S(18), rh - S(6));
    const float cw = (w - S(28)) / cols;
    for (int i = 0; i < n; i++) {
        const FsRow &r = rows[(size_t)i];
        int c = cols == 2 ? i % 2 : 0, row = cols == 2 ? i / 2 : i;
        ImVec2 p0 = o + ImVec2(x + S(14) + c * cw, top + row * rh + (rh - side) * 0.5f);
        fs_portrait(dl, a, r.hero, p0, side, k_fsGem[i % 4]);
        ImVec4 cc(p0.x, o.y + y, p0.x + cw - S(8), o.y + y + h);
        // two lines beside the portrait: the name, then the hero and the numbers (smaller); with two columns the
        // numbers are the enemies only, the falls don't fit
        const char *hero = r.hero >= 0 && r.hero < 9 ? tr(k_fsHero[r.hero]) : "?";
        std::string line2 = std::string(hero) + "  \xc2\xb7  " + r.kills + (cols == 1 ? ",  " + r.falls : std::string());
        const float s1 = fs, s2 = fs * 0.84f;
        float tx = p0.x + side + S(10), mid = p0.y + side * 0.5f;
        dl->AddText(f, s1, ImVec2(tx, mid - s1 + S(1)), ImGui::GetColorU32(C_TEXT), r.name.c_str(), nullptr, 0, &cc);
        dl->AddText(f, s2, ImVec2(tx, mid + S(1)), ImGui::GetColorU32(C_DIM), line2.c_str(), nullptr, 0, &cc);
    }
    (void)lines;
}

// The front screen's players: whose controller is which, on the right (as chosen on the Controls tab)
static void front_who_plays(App &a, ImVec2 pos, ImVec2 sz)
{
    std::vector<std::string> lines;
    if (!a.dir.empty()) rotkplus_pad_lines(a, lines);
    lines.resize(4);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 o = ImGui::GetWindowPos();
    float x = pos.x, y = pos.y, w = sz.x, h = sz.y;
    ImFont *f = F_SMALL;
    float fs = f->LegacySize * ImGui::GetStyle().FontScaleMain, lh = fs * 1.25f;
    orn_plate(dl, o + ImVec2(x, y), o + ImVec2(x + w, y + h), IM_COL32(9, 12, 20, 200), false);
    dl->AddText(F_HEAD, F_HEAD->LegacySize * ImGui::GetStyle().FontScaleMain, o + ImVec2(x + S(14), y + S(10)),
                ImGui::GetColorU32(C_GOLD_H), tr("Controllers"));
    // how many are connected, on the heading's line on the right
    char n[64];
    if (a.controllers == 1) snprintf(n, sizeof n, "%s", tr("1 connected"));
    else snprintf(n, sizeof n, tr("%d connected"), a.controllers);
    ImVec2 ns = f->CalcTextSizeA(fs, FLT_MAX, 0, n);
    dl->AddText(f, fs, o + ImVec2(x + w - S(14) - ns.x, y + S(14)), ImGui::GetColorU32(a.controllers ? C_GOOD : C_DIM), n);
    // the four players, one under the other: the gem, the number, its controller
    float ty = y + S(42);
    for (int p = 0; p < 4; p++, ty += lh) {
        ImVec4 clip(o.x + x, o.y + y, o.x + x + w - S(10), o.y + y + h);
        ImVec2 c = o + ImVec2(x + S(20), ty + lh * 0.45f);
        ImVec2 d[4] = { ImVec2(c.x - S(4), c.y), ImVec2(c.x, c.y - S(4)), ImVec2(c.x + S(4), c.y), ImVec2(c.x, c.y + S(4)) };
        dl->AddConvexPolyFilled(d, 4, k_fsGem[p]);
        char l[160];
        snprintf(l, sizeof l, "%d  %s", p + 1, lines[(size_t)p].c_str());
        dl->AddText(f, fs, o + ImVec2(x + S(32), ty), ImGui::GetColorU32(C_DIM), l, nullptr, 0, &clip);
    }
}
