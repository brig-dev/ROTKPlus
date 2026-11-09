// ROTKPlus: the Screenshots page. The pictures ROTK Fix saves in the game folder's Screenshots subfolder (the key in
// ROTK Fix's settings), newest first: small copies in a grid, made on a thread of their own so the page never waits;
// one picture at full size on a click, with the one before and after, open it, show it in its folder, or put it in the
// Recycle Bin. Included by pages.cpp after fellowship.inc.

struct GalShot {
    std::string path, name;
    unsigned long long when = 0;               // last written (FILETIME), for the order and the date shown
    SDL_Texture *tex = nullptr;                // the small copy, once made
    unsigned char *px = nullptr;               // decoded and shrunk by the thread, made into a texture here
    int w = 0, h = 0, state = 0;               // 0 waiting, 1 pixels ready, 2 shown, -1 couldn't be read
};
static std::vector<GalShot> g_gal;
static std::string g_galFor;                   // the folder read
static SDL_Thread *g_galThread = nullptr;
static SDL_AtomicInt g_galStop;
static SDL_Mutex *g_galMx = nullptr;
static int g_galView = -1;                     // the picture at full size, or -1 for the grid
static SDL_Texture *g_galBig = nullptr;
static int g_galBigFor = -1, g_galBigW = 0, g_galBigH = 0;

static std::string gal_dir(const App &a) { return a.dir + "\\Screenshots"; }

// a picture shrunk to at most 400 pixels across, by averaging (a box filter: no sharper copy is needed for a grid)
static unsigned char *gal_shrink(const unsigned char *src, int w, int h, int &ow, int &oh)
{
    int k = std::max(1, (w + 399) / 400);
    ow = std::max(1, w / k); oh = std::max(1, h / k);
    unsigned char *out = (unsigned char *)malloc((size_t)ow * oh * 4);
    if (!out) return nullptr;
    for (int y = 0; y < oh; y++)
        for (int x = 0; x < ow; x++) {
            unsigned s[4] = { 0, 0, 0, 0 };
            for (int dy = 0; dy < k; dy++) {
                const unsigned char *p = src + (((size_t)(y * k + dy) * w) + (size_t)x * k) * 4;
                for (int dx = 0; dx < k; dx++, p += 4) { s[0] += p[0]; s[1] += p[1]; s[2] += p[2]; s[3] += p[3]; }
            }
            unsigned char *o = out + ((size_t)y * ow + x) * 4;
            for (int c = 0; c < 4; c++) o[c] = (unsigned char)(s[c] / (unsigned)(k * k));
        }
    return out;
}

static int SDLCALL gal_worker(void *)
{
    for (size_t i = 0; i < g_gal.size() && !SDL_GetAtomicInt(&g_galStop); i++) {
        int w, h, c;
        unsigned char *full = stbi_load(g_gal[i].path.c_str(), &w, &h, &c, 4);
        int ow = 0, oh = 0;
        unsigned char *small = full ? gal_shrink(full, w, h, ow, oh) : nullptr;
        if (full) stbi_image_free(full);
        SDL_LockMutex(g_galMx);
        if (small) { g_gal[i].px = small; g_gal[i].w = ow; g_gal[i].h = oh; g_gal[i].state = 1; }
        else g_gal[i].state = -1;
        SDL_UnlockMutex(g_galMx);
    }
    return 0;
}

// the page forgets everything (read again on its next showing)
static void gal_forget()
{
    if (g_galThread) { SDL_SetAtomicInt(&g_galStop, 1); SDL_WaitThread(g_galThread, nullptr); g_galThread = nullptr; }
    for (auto &s : g_gal) { if (s.tex) SDL_DestroyTexture(s.tex); free(s.px); }
    g_gal.clear();
    g_galFor.clear();
    if (g_galBig) SDL_DestroyTexture(g_galBig);
    g_galBig = nullptr; g_galBigFor = -1; g_galView = -1;
}

static void gal_load(const App &a)
{
    gal_forget();
    g_galFor = a.dir;
    WIN32_FIND_DATAA fd;
    HANDLE fh = FindFirstFileA((gal_dir(a) + "\\*.png").c_str(), &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            GalShot s;
            s.name = fd.cFileName;
            s.path = gal_dir(a) + "\\" + s.name;
            s.when = ((unsigned long long)fd.ftLastWriteTime.dwHighDateTime << 32) | fd.ftLastWriteTime.dwLowDateTime;
            g_gal.push_back(s);
        } while (FindNextFileA(fh, &fd));
        FindClose(fh);
    }
    std::sort(g_gal.begin(), g_gal.end(), [](const GalShot &x, const GalShot &y) { return x.when > y.when; });
    if (!g_galMx) g_galMx = SDL_CreateMutex();
    SDL_SetAtomicInt(&g_galStop, 0);
    if (!g_gal.empty()) g_galThread = SDL_CreateThread(gal_worker, "screenshots", nullptr);
}

static std::string gal_date(unsigned long long when)
{
    FILETIME ft{ (DWORD)when, (DWORD)(when >> 32) }, lt;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &lt);
    FileTimeToSystemTime(&lt, &st);
    char b[64];
    snprintf(b, sizeof b, "%02d/%02d/%04d  %02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);
    return b;
}

// ROTKPlus: the day a picture was taken, written out in the launcher's language ("Sunday 8 November 2026"), as the
// heading of that day's pictures in the grid
static std::string gal_day(unsigned long long when)
{
    FILETIME ft{ (DWORD)when, (DWORD)(when >> 32) }, lt;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&ft, &lt);
    FileTimeToSystemTime(&lt, &st);
    static const struct { const char *code; const wchar_t *loc; } k_loc[] = {
        { "it", L"it-IT" }, { "de", L"de-DE" }, { "fr", L"fr-FR" }, { "es", L"es-ES" }, { "pt-BR", L"pt-BR" },
        { "pl", L"pl-PL" }, { "ru", L"ru-RU" }, { "ja", L"ja-JP" }, { "ko", L"ko-KR" }, { "zh-Hans", L"zh-CN" } };
    const wchar_t *loc = L"en-GB";
    const std::string lang = i18n_current();
    for (const auto &e : k_loc) if (lang == e.code) loc = e.loc;
    wchar_t w[96];
    char u[256];
    if (!GetDateFormatEx(loc, DATE_LONGDATE, &st, NULL, w, 96, NULL) ||
        !WideCharToMultiByte(CP_UTF8, 0, w, -1, u, sizeof u, NULL, NULL)) {
        snprintf(u, sizeof u, "%02d/%02d/%04d", st.wDay, st.wMonth, st.wYear);
    }
    std::string r = u;
    if (!r.empty() && r[0] >= 'a' && r[0] <= 'z') r[0] = (char)(r[0] - 32);   // (Italian, French... write it small)
    return r;
}

// ROTKPlus: a screenshot among the launcher's own background pictures (ROTKFix\backgrounds beside the launcher, the
// "my pictures" of Launcher settings), shown next
static std::string g_galNote;
static double g_galNoteAt = -100;
static bool g_galNoteBad = false;
static void gal_use_as_background(App &a, const GalShot &s)
{
    const std::string dir = exe_dir() + "\\ROTKFix\\backgrounds", dest = dir + "\\" + s.name;
    SHCreateDirectoryExA(NULL, dir.c_str(), NULL);
    g_galNoteAt = ImGui::GetTime();
    g_galNoteBad = !CopyFileA(s.path.c_str(), dest.c_str(), FALSE);
    if (g_galNoteBad) { g_galNote = tr("It couldn't be copied among the backgrounds."); return; }
    if (g_bgShow == BG_ART || g_bgShow == BG_NONE) { g_bgShow = BG_ART_AND_MINE; bg_show_save(); }
    auto it = std::find(a.bg.files.begin(), a.bg.files.end(), dest);
    if (it != a.bg.files.end()) a.bg.index = (int)(it - a.bg.files.begin()) - 1;
    else a.bg.files.insert(a.bg.files.begin() + std::min((int)a.bg.files.size(), a.bg.index + 1), dest);
    if (!a.bg.job) backgrounds_next(a.bg);
    g_galNote = tr("Now among the launcher's backgrounds (About tab: Background pictures).");
}

// ROTKPlus: a screenshot on the clipboard, to paste into a post or a chat
static void gal_copy(const GalShot &s)
{
    g_galNoteAt = ImGui::GetTime();
    std::vector<unsigned char> png;
    if (FILE *f = fopen(s.path.c_str(), "rb")) {
        fseek(f, 0, SEEK_END);
        long n = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (n > 0 && n < (64 << 20)) { png.resize((size_t)n); if (fread(png.data(), 1, png.size(), f) != png.size()) png.clear(); }
        fclose(f);
    }
    int w = 0, h = 0, c = 0;
    unsigned char *px = png.empty() ? nullptr : stbi_load_from_memory(png.data(), (int)png.size(), &w, &h, &c, 4);
    bool ok = px && clipboard_image(px, w, h, png);
    if (px) stbi_image_free(px);
    g_galNoteBad = !ok;
    g_galNote = tr(ok ? "Copied: paste it into a post or a chat." : "It couldn't be copied.");
}

// the picture to the Recycle Bin (it can be taken back from there)
static bool gal_recycle(const std::string &path)
{
    wchar_t w[MAX_PATH + 2] = {};
    if (!MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, w, MAX_PATH)) return false;
    SHFILEOPSTRUCTW op = {};
    op.wFunc = FO_DELETE;
    op.pFrom = w;   // (ends with two zeros: the array is cleared)
    op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;
    return SHFileOperationW(&op) == 0 && !op.fAnyOperationsAborted;
}

static void draw_gallery(App &a, ImVec2 size, SDL_Renderer *ren)
{
    if (g_galFor != a.dir) {
        gal_load(a);
        static bool envOnce = true;   // testing: ROTKPLUS_SHOT = n opens the n-th picture
        if (envOnce) { envOnce = false; if (const char *e = getenv("ROTKPLUS_SHOT")) if (*e && !g_gal.empty()) g_galView = std::min((int)g_gal.size() - 1, atoi(e)); }
    }
    // the small copies the thread has made become textures (a few each frame)
    if (g_galMx) {
        SDL_LockMutex(g_galMx);
        int made = 0;
        for (auto &s : g_gal)
            if (s.state == 1 && made < 6) {
                s.tex = make_texture(ren, s.px, s.w, s.h);
                free(s.px); s.px = nullptr;
                s.state = s.tex ? 2 : -1;
                made++;
            }
        SDL_UnlockMutex(g_galMx);
    }
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const float ts = F_TITLE->LegacySize * 0.5f * ImGui::GetStyle().FontScaleMain;
    ImVec2 tp = ImGui::GetWindowPos() + ImVec2(S(28), S(16));
    const char *title = tr("Screenshots");
    dl->AddText(F_TITLE, ts, tp + ImVec2(S(1.5f), S(1.5f)), IM_COL32(0, 0, 0, 110), title);
    dl->AddText(F_TITLE, ts, tp, ImGui::GetColorU32(C_GOLD_H), title);

    const float panelW = std::min(size.x - S(60), S(1180)), panelX = (size.x - panelW) * 0.5f, top = S(84);
    const float panelH = size.y - top - S(80);
    ImVec2 panelA = ImGui::GetWindowPos() + ImVec2(panelX, top), panelB = panelA + ImVec2(panelW, panelH);
    orn_panel_fill(dl, panelA, panelB, ImGui::GetColorU32(rgb(9, 12, 20, 0.80f)));
    const float inset = S(9) * 0.5f + S(5);
    ImGui::SetCursorPos(ImVec2(panelX + inset, top + inset));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("gallery", ImVec2(panelW - inset * 2, panelH - inset * 2), ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_NavFlattened);
    bool modal = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    if (g_galView >= (int)g_gal.size()) g_galView = -1;

    if (g_galView < 0) {
        // ---- the grid
        ImGui::PushTextWrapPos(0);
        if (g_gal.empty()) {
            ImGui::TextColored(C_DIM, "%s", tr("No screenshots yet. In the game press the screenshot key: "
                                                "ROTK Fix saves them in the game folder's Screenshots subfolder."));
        } else {
            char n[96];
            snprintf(n, sizeof n, tr("%d screenshots, the newest first. A click shows one at full size."), (int)g_gal.size());
            ImGui::TextColored(C_DIM, "%s", n);
        }
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        const float avail = ImGui::GetContentRegionAvail().x, gap = S(12);
        int cols = std::max(1, (int)((avail + gap) / (S(250) + gap)));
        float tw = (avail - gap * (cols - 1)) / cols, th = tw * 10.0f / 16.0f;
        std::string lastDay;
        int col = 0;
        for (int i = 0; i < (int)g_gal.size(); i++) {
            GalShot &s = g_gal[(size_t)i];
            std::string day = gal_day(s.when);   // ROTKPlus: the pictures by day, each day under its heading
            if (day != lastDay) {
                if (i) ImGui::Dummy(ImVec2(0, S(4)));
                ImGui::PushFont(F_HEAD, F_HEAD->LegacySize);
                ImGui::TextColored(C_GOLD_H, "%s", day.c_str());
                ImGui::PopFont();
                lastDay = day;
                col = 0;
            }
            if (col) ImGui::SameLine(0, gap);
            col = (col + 1) % cols;
            ImGui::PushID(i);
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("##shot", ImVec2(tw, th))) { g_galView = i; }
            bool hov = ImGui::IsItemHovered() || ImGui::IsItemFocused();
            ImDrawList *wl = ImGui::GetWindowDrawList();
            ImVec2 p1 = p0 + ImVec2(tw, th);
            wl->AddRectFilled(p0, p1, IM_COL32(6, 9, 16, 200), S(4));
            if (s.tex) {   // the whole picture, fitted inside the frame
                float k = std::min(tw / s.w, th / s.h), iw = s.w * k, ih = s.h * k;
                ImVec2 q0 = p0 + ImVec2((tw - iw) * 0.5f, (th - ih) * 0.5f);
                wl->AddImage((ImTextureID)(intptr_t)s.tex, q0, q0 + ImVec2(iw, ih));
            } else {
                const char *t = s.state < 0 ? "?" : "...";
                ImVec2 sz = ImGui::CalcTextSize(t);
                wl->AddText(p0 + ImVec2((tw - sz.x) * 0.5f, (th - sz.y) * 0.5f), ImGui::GetColorU32(C_DIM), t);
            }
            wl->AddRect(p0, p1, hov ? ImGui::GetColorU32(C_GOLD_H) : IM_COL32(58, 74, 102, 200), S(4), S(hov ? 2.0f : 1.0f));
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", gal_date(s.when).c_str());
            ImGui::PopID();
        }
    } else {
        // ---- one picture at full size
        GalShot &s = g_gal[(size_t)g_galView];
        if (g_galBigFor != g_galView) {
            if (g_galBig) SDL_DestroyTexture(g_galBig);
            g_galBig = nullptr; g_galBigFor = g_galView;
            int w, h, c;
            if (unsigned char *px = stbi_load(s.path.c_str(), &w, &h, &c, 4)) {
                g_galBig = make_texture(ren, px, w, h);
                g_galBigW = w; g_galBigH = h;
                stbi_image_free(px);
            }
        }
        ImGui::TextColored(C_GOLD_H, "%s", gal_date(s.when).c_str());
        ImGui::SameLine();
        ImGui::TextColored(C_DIM, "  %d / %d  \xc2\xb7  %s", g_galView + 1, (int)g_gal.size(), s.name.c_str());
        float bh = S(40);
        ImVec2 room = ImGui::GetContentRegionAvail() - ImVec2(0, bh + S(14) + ImGui::GetTextLineHeightWithSpacing());   // (and a line for Copy's note)
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        if (g_galBig) {
            float k = std::min(room.x / g_galBigW, room.y / g_galBigH), iw = g_galBigW * k, ih = g_galBigH * k;
            ImVec2 q0 = p0 + ImVec2((room.x - iw) * 0.5f, (room.y - ih) * 0.5f);
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)g_galBig, q0, q0 + ImVec2(iw, ih));
        } else ImGui::TextColored(C_BAD, "%s", tr("This picture can't be read."));
        ImGui::SetCursorScreenPos(p0 + ImVec2(0, room.y + S(10)));
        bool prev = ImGui::Button("\xe2\x80\xb9##gprev", ImVec2(S(60), bh));
        ImGui::SameLine();
        bool next = ImGui::Button("\xe2\x80\xba##gnext", ImVec2(S(60), bh));
        ImGui::SameLine(0, S(24));
        if (ImGui::Button("Open", ImVec2(0, bh))) ShellExecuteA(NULL, "open", s.path.c_str(), NULL, NULL, SW_SHOWNORMAL);
        ImGui::SameLine();
        if (ImGui::Button("Show in its folder", ImVec2(0, bh)))
            ShellExecuteA(NULL, "open", "explorer.exe", ("/select,\"" + s.path + "\"").c_str(), NULL, SW_SHOWNORMAL);
        ImGui::SameLine();
        if (ImGui::Button("Copy", ImVec2(0, bh))) gal_copy(s);
        ImGui::SameLine();
        if (ImGui::Button("Use as background", ImVec2(0, bh))) gal_use_as_background(a, s);
        ImGui::SameLine();
        if (ImGui::Button("Recycle Bin...", ImVec2(0, bh))) ImGui::OpenPopup("##galdel");
        if (ImGui::GetTime() - g_galNoteAt < 6.0) ImGui::TextColored(g_galNoteBad ? C_BAD : C_GOOD, "%s", g_galNote.c_str());
        if (ImGui::BeginPopupModal("##galdel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(C_GOLD_H, "%s", tr("Put this screenshot in the Recycle Bin?"));
            ImGui::TextColored(C_DIM, "%s", tr("You can take it back from there."));
            ImGui::Spacing();
            if (ImGui::Button("Recycle Bin", ImVec2(S(140), 0))) {
                std::string dir = a.dir;
                int at = g_galView;
                bool ok = gal_recycle(s.path);
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                gal_load(a);   // (read again: the thread lists the folder as it is now)
                g_galView = g_gal.empty() ? -1 : std::min(at, (int)g_gal.size() - 1);
                if (!ok) set_message(a, tr("The screenshot couldn't be moved to the Recycle Bin."), true);
                ImGui::EndChild();
                ImGui::PopStyleColor();
                orn_panel_frame(dl, panelA, panelB);
                return;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(S(110), 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        if (!modal) {
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadL1)) prev = true;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadR1)) next = true;
        }
        int n = (int)g_gal.size();
        if (prev) g_galView = (g_galView + n - 1) % n;
        if (next) g_galView = (g_galView + 1) % n;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    orn_panel_frame(dl, panelA, panelB);

    modal = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel);
    ImGui::SetCursorPos(ImVec2(S(40), size.y - S(60)));
    bool back = orn_plate_button("\xe2\x80\xb9  Back##gback", ImVec2(S(130), S(36)), true);
    ImGui::SameLine(0, S(16));
    if (orn_plate_button("Open the folder##gdir", ImVec2(S(200), S(36)), true)) {
        CreateDirectoryA(gal_dir(a).c_str(), NULL);
        ShellExecuteA(NULL, "open", gal_dir(a).c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
    if (!modal && (ImGui::IsKeyPressed(ImGuiKey_Escape, false) || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false))) back = true;
    if (back) {
        if (g_galView >= 0) g_galView = -1;   // from a picture back to the grid
        else { a.gallery = false; a.front = true; gal_forget(); }
    }
}
