// ROTKPlus: a player's card as a picture to share (Reddit, Discord...), from the Fellowship page. 1280 x 720: the
// launcher's background picture of the moment (from the player's own game), the favourite hero's portrait, the name,
// a title earned with the deeds, the main numbers, the peoples most defeated and the deeds as gems.
// How: the card is drawn with Dear ImGui into a draw list of its own during the frame (so the fonts make the letters
// it needs, sent to the graphics card with the frame), rendered into a texture after the frame (share_card_flush,
// called by main.cpp after the window is drawn), saved as a PNG in the game's Screenshots folder (the Screenshots page
// shows it too) and shown in a window with Copy, Open and Show in its folder. Included by pages.cpp after gallery.inc.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#include "../third_party/miniz/miniz.h"
#pragma GCC diagnostic pop

static const int k_shW = 1280, k_shH = 720;
static ImDrawList *g_shList = nullptr;          // the card, drawn in the frame it was asked for
static int g_shAsk = -1;                        // the player (index in g_fs) whose card the next frame draws
static bool g_shDrawn = false;                  // drawn: share_card_flush makes the picture
static bool g_shOpen = false, g_shFailed = false, g_shCopied = false;
static SDL_Texture *g_shTex = nullptr;          // the last card, shown in the window
static std::vector<unsigned char> g_shRgba;     // its pixels, for the clipboard
static std::vector<unsigned char> g_shPng;
static std::string g_shPath, g_shName;

// The picture on the clipboard, as Windows' bitmap (CF_DIB: Paint, Word, most programs) and as PNG ("PNG": browsers,
// Discord), so it can be pasted straight into a post or a chat.
static bool clipboard_image(const unsigned char *rgba, int w, int h, const std::vector<unsigned char> &png)
{
    if (!rgba || w <= 0 || h <= 0 || !OpenClipboard(NULL)) return false;
    EmptyClipboard();
    bool ok = false;
    const size_t bytes = sizeof(BITMAPINFOHEADER) + (size_t)w * h * 4;
    if (HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, bytes)) {
        BITMAPINFOHEADER *bi = (BITMAPINFOHEADER *)GlobalLock(g);
        memset(bi, 0, sizeof *bi);
        bi->biSize = sizeof *bi; bi->biWidth = w; bi->biHeight = h;   // (bottom-up: the last row first)
        bi->biPlanes = 1; bi->biBitCount = 32; bi->biCompression = BI_RGB; bi->biSizeImage = (DWORD)(w * h * 4);
        unsigned char *dst = (unsigned char *)(bi + 1);
        for (int y = 0; y < h; y++) {
            const unsigned char *s = rgba + (size_t)(h - 1 - y) * w * 4;
            unsigned char *d = dst + (size_t)y * w * 4;
            for (int x = 0; x < w; x++, s += 4, d += 4) { d[0] = s[2]; d[1] = s[1]; d[2] = s[0]; d[3] = 255; }
        }
        GlobalUnlock(g);
        if (SetClipboardData(CF_DIB, g)) ok = true; else GlobalFree(g);
    }
    if (!png.empty())
        if (HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, png.size())) {
            memcpy(GlobalLock(g), png.data(), png.size());
            GlobalUnlock(g);
            if (SetClipboardData(RegisterClipboardFormatA("PNG"), g)) ok = true; else GlobalFree(g);
        }
    CloseClipboard();
    return ok;
}

static ImU32 sh_col(ImVec4 c, float alpha = 1.0f) { c.w *= alpha; return ImGui::ColorConvertFloat4ToU32(c); }
static float sh_w(ImFont *f, float sz, const char *t) { return f->CalcTextSizeA(sz, FLT_MAX, 0, t).x; }
static float sh_fit(ImFont *f, float sz, const char *t, float room) { float w = sh_w(f, sz, t); return w > room ? sz * room / w : sz; }

// text with a soft shadow; align 0 from x, 0.5 centred on x, 1 ending at x
static void sh_text(ImDrawList *dl, ImFont *f, float sz, float x, float y, ImU32 col, const char *t, float align = 0, float shadow = 2)
{
    x -= sh_w(f, sz, t) * align;
    if (shadow > 0) dl->AddText(f, sz, ImVec2(x + shadow, y + shadow), IM_COL32(0, 0, 0, 160), t);
    dl->AddText(f, sz, ImVec2(x, y), col, t);
}

// spaced capitals, one letter at a time (UTF-8), centred on x
static void sh_spaced(ImDrawList *dl, ImFont *f, float sz, float x, float y, ImU32 col, const std::string &t, float gap)
{
    auto len = [&](size_t i) { unsigned char c = (unsigned char)t[i]; return c >= 0xF0 ? 4u : c >= 0xE0 ? 3u : c >= 0xC0 ? 2u : 1u; };
    float w = 0;
    for (size_t i = 0; i < t.size(); i += len(i)) w += f->CalcTextSizeA(sz, FLT_MAX, 0, t.c_str() + i, t.c_str() + i + len(i)).x + gap;
    x -= (w - gap) * 0.5f;
    for (size_t i = 0; i < t.size(); i += len(i)) {
        dl->AddText(f, sz, ImVec2(x + 2, y + 2), IM_COL32(0, 0, 0, 160), t.c_str() + i, t.c_str() + i + len(i));
        dl->AddText(f, sz, ImVec2(x, y), col, t.c_str() + i, t.c_str() + i + len(i));
        x += f->CalcTextSizeA(sz, FLT_MAX, 0, t.c_str() + i, t.c_str() + i + len(i)).x + gap;
    }
}

static void sh_gem(ImDrawList *dl, ImVec2 c, float r, ImU32 col, bool filled = true)
{
    ImVec2 d[4] = { ImVec2(c.x - r, c.y), ImVec2(c.x, c.y - r), ImVec2(c.x + r, c.y), ImVec2(c.x, c.y + r) };
    if (filled) dl->AddConvexPolyFilled(d, 4, col);
    else dl->AddPolyline(d, 4, col, 1.5f, ImDrawFlags_Closed);
}

// the card of g_fs[idx], into g_shList (in the frame: the fonts make their letters now)
static void share_build(App &a, int idx)
{
    const FsPlayer &p = g_fs[(size_t)idx];
    if (!g_shList) g_shList = IM_NEW(ImDrawList)(ImGui::GetDrawListSharedData());
    ImDrawList *dl = g_shList;
    const float W = (float)k_shW, H = (float)k_shH;
    dl->_ResetForNewFrame();
    dl->_SetPixelDensity(1.0f);
    dl->PushTexture(ImGui::GetIO().Fonts->TexRef);
    dl->PushClipRect(ImVec2(0, 0), ImVec2(W, H), false);
    const ImU32 silver = sh_col(C_GOLD_H), soft = sh_col(C_GOLD), text = sh_col(C_TEXT), dim = sh_col(C_DIM), gem = fs_gem((size_t)idx);

    // the picture, clearer on the left (around the portrait), darker under the numbers
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), IM_COL32(8, 11, 20, 255));
    if (a.bg.cur.tex) draw_picture_cover(dl, a.bg.cur, ImVec2(W, H));
    const ImU32 clear = IM_COL32(6, 9, 16, 110), dark = IM_COL32(6, 9, 16, 222);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(W * 0.42f, H), clear, dark, dark, clear);
    dl->AddRectFilled(ImVec2(W * 0.42f, 0), ImVec2(W, H), dark);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(W, 150), IM_COL32(4, 6, 12, 170), IM_COL32(4, 6, 12, 170), IM_COL32(4, 6, 12, 0), IM_COL32(4, 6, 12, 0));
    dl->AddRectFilledMultiColor(ImVec2(0, H - 110), ImVec2(W, H), IM_COL32(4, 6, 12, 0), IM_COL32(4, 6, 12, 0), IM_COL32(4, 6, 12, 200), IM_COL32(4, 6, 12, 200));
    // the frame: two lines and a gem at each corner
    dl->AddRect(ImVec2(14, 14), ImVec2(W - 14, H - 14), sh_col(C_GOLD, 0.85f), 0, 2.0f);
    dl->AddRect(ImVec2(22, 22), ImVec2(W - 22, H - 22), sh_col(C_GOLD, 0.35f), 0, 1.0f);
    for (ImVec2 c : { ImVec2(22, 22), ImVec2(W - 22, 22), ImVec2(22, H - 22), ImVec2(W - 22, H - 22) }) sh_gem(dl, c, 6, silver);

    // the game's name at the top
    sh_spaced(dl, F_HEAD, 19, W * 0.5f, 42, soft, tr("THE LORD OF THE RINGS"), 5);
    sh_spaced(dl, F_TITLE, 38, W * 0.5f, 66, silver, tr("The Return of the King"), 1);
    dl->AddLine(ImVec2(W * 0.5f - 190, 124), ImVec2(W * 0.5f - 14, 124), sh_col(C_GOLD, 0.5f), 1.0f);
    dl->AddLine(ImVec2(W * 0.5f + 14, 124), ImVec2(W * 0.5f + 190, 124), sh_col(C_GOLD, 0.5f), 1.0f);
    sh_gem(dl, ImVec2(W * 0.5f, 124), 5, gem);

    // left: the favourite hero
    const int fav = fs_favourite(p);
    const float side = 250, px = 104, py = 176, cx = px + side * 0.5f;
    dl->AddRectFilled(ImVec2(px - 12, py - 12), ImVec2(px + side + 12, py + side + 12), IM_COL32(6, 9, 16, 210), 8);
    if (SDL_Texture *t = fs_hero_tex(a, fav)) dl->AddImage((ImTextureID)(intptr_t)t, ImVec2(px, py), ImVec2(px + side, py + side));
    else sh_gem(dl, ImVec2(cx, py + side * 0.5f), 46, gem);
    dl->AddRect(ImVec2(px, py), ImVec2(px + side, py + side), gem, 3, 4);
    dl->AddRect(ImVec2(px - 12, py - 12), ImVec2(px + side + 12, py + side + 12), sh_col(C_GOLD, 0.7f), 8, 1.5f);
    const char *hero = fav >= 0 ? tr(k_fsHero[fav]) : "-";
    sh_text(dl, F_HEAD, sh_fit(F_HEAD, 34, hero, side + 40), cx, py + side + 28, silver, hero, 0.5f);
    sh_text(dl, F_BODY, 22, cx, py + side + 72, dim, tr("Favourite hero"), 0.5f);

    // right: the name, the title, the numbers
    const float rx = 430, rw = W - 92 - rx;
    sh_text(dl, F_TITLE, sh_fit(F_TITLE, 64, p.name.c_str(), rw), rx, 150, silver, p.name.c_str(), 0, 3);
    const int nd = (int)(sizeof k_fsPlayerDeeds / sizeof k_fsPlayerDeeds[0]);
    int got = 0;
    for (const auto &d : k_fsPlayerDeeds) if (d.value(p) >= d.need) got++;
    sh_gem(dl, ImVec2(rx + 7, 240), 6, gem);
    sh_text(dl, F_HEAD, 24, rx + 24, 226, text, tr(fs_rank(got, nd)));
    dl->AddLine(ImVec2(rx, 272), ImVec2(rx + rw, 272), sh_col(C_GOLD, 0.35f), 1.0f);

    const bool pvp = fs_get(p, "PvpGames") > 0;
    char versus[48];
    snprintf(versus, sizeof versus, "%s / %s", fs_num(fs_get(p, "PvpWins")).c_str(), fs_num(fs_get(p, "PvpGames")).c_str());
    const std::string val[6] = { fs_num(fs_get(p, "Kills")), fs_num(fs_get(p, "Wins")), fs_time(fs_get(p, "Seconds")),
                                 fs_num(fs_get(p, "BestKills")), fs_num(fs_get(p, "Assists")),
                                 pvp ? std::string(versus) : fs_num(fs_get(p, "Experience")) };
    const char *lab[6] = { "Enemies", "Levels completed", "Time played", "Best level", "Assists", pvp ? "Versus won" : "Experience" };
    const float cw = rw / 3;
    for (int i = 0; i < 6; i++) {
        float x = rx + (float)(i % 3) * cw, y = 294.0f + (float)(i / 3) * 102.0f;
        dl->AddLine(ImVec2(x, y), ImVec2(x + 36, y), gem, 2.0f);
        sh_text(dl, F_HEAD, sh_fit(F_HEAD, 44, val[i].c_str(), cw - 18), x, y + 8, text, val[i].c_str());
        sh_text(dl, F_BODY, sh_fit(F_BODY, 21, tr(lab[i]), cw - 18), x, y + 60, dim, tr(lab[i]));
    }

    // the three peoples most defeated, as bars
    const int nf = (int)(sizeof k_fsFoes / sizeof k_fsFoes[0]);
    std::vector<std::pair<long long, int>> foes;
    for (int f = 0; f < nf; f++) if (long long n = fs_foes(p, f)) foes.push_back({ n, f });
    std::stable_sort(foes.begin(), foes.end(), [](const std::pair<long long, int> &x, const std::pair<long long, int> &y) { return x.first > y.first; });
    if (foes.size() > 3) foes.resize(3);
    float y = 506;
    for (auto &f : foes) {
        const char *name = tr(k_fsFoes[f.second].name);
        sh_text(dl, F_BODY, 21, rx, y, text, name);
        const float b0 = rx + 150, b1 = rx + rw - 110, bw = (b1 - b0) * (float)f.first / (float)foes[0].first;
        dl->AddRectFilled(ImVec2(b0, y + 8), ImVec2(b1, y + 18), IM_COL32(30, 40, 60, 200), 4);
        dl->AddRectFilled(ImVec2(b0, y + 8), ImVec2(b0 + std::max(6.0f, bw), y + 18), gem, 4);
        std::string n = fs_num(f.first);
        sh_text(dl, F_BODY, 21, rx + rw, y, text, n.c_str(), 1.0f);
        y += 32;
    }

    // the deeds: one gem each, full when earned
    char dn[32];
    snprintf(dn, sizeof dn, "%d / %d", got, nd);
    sh_text(dl, F_HEAD, 21, rx, 612, soft, tr("Deeds"));
    float gx = rx + sh_w(F_HEAD, 21, tr("Deeds")) + 16;
    sh_text(dl, F_HEAD, 21, gx, 612, text, dn);
    gx += sh_w(F_HEAD, 21, dn) + 26;
    for (int i = 0; i < nd && gx < rx + rw; i++, gx += 21) sh_gem(dl, ImVec2(gx, 625), 7, i < got ? gem : sh_col(C_DIM, 0.7f), i < got);

    // the signature
    sh_text(dl, F_HEAD, 22, 48, H - 62, silver, "ROTKPlus");
    sh_text(dl, F_BODY, 20, 48 + sh_w(F_HEAD, 22, "ROTKPlus") + 14, H - 60, dim, tr("co-op and versus for up to four players"));
    sh_text(dl, F_BODY, 20, W - 48, H - 60, dim, "github.com/brig-dev/ROTKPlus", 1.0f);
    g_shDrawn = true;
}

// After the frame: the card into a texture, read back, saved as a PNG; the window opens on the next frame
void share_card_flush(SDL_Renderer *ren)
{
    if (!g_shDrawn) return;
    g_shDrawn = false;
    g_shOpen = true;
    g_shFailed = true;
    g_shCopied = false;
    if (g_shTex) SDL_DestroyTexture(g_shTex);
    g_shTex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, k_shW, k_shH);
    if (!g_shTex) return;
    SDL_SetTextureScaleMode(g_shTex, SDL_SCALEMODE_LINEAR);
    SDL_Texture *was = SDL_GetRenderTarget(ren);
    SDL_SetRenderTarget(ren, g_shTex);
    SDL_SetRenderDrawColor(ren, 8, 11, 20, 255);
    SDL_RenderClear(ren);
    ImDrawData dd;
    dd.Valid = true;
    dd.DisplayPos = ImVec2(0, 0);
    dd.DisplaySize = ImVec2((float)k_shW, (float)k_shH);
    dd.FramebufferScale = ImVec2(1, 1);
    dd.AddDrawList(g_shList);
    ImGui_ImplSDLRenderer3_RenderDrawData(&dd, ren);
    SDL_Surface *s = SDL_RenderReadPixels(ren, NULL);
    SDL_SetRenderTarget(ren, was);
    SDL_Surface *c = s ? SDL_ConvertSurface(s, SDL_PIXELFORMAT_RGBA32) : nullptr;
    if (s) SDL_DestroySurface(s);
    if (!c) return;
    const int w = c->w, h = c->h;
    g_shRgba.assign((size_t)w * h * 4, 0);
    std::vector<unsigned char> rgb((size_t)w * h * 3);
    for (int yy = 0; yy < h; yy++) {
        const unsigned char *src = (const unsigned char *)c->pixels + (size_t)yy * c->pitch;
        memcpy(&g_shRgba[(size_t)yy * w * 4], src, (size_t)w * 4);
        for (int x = 0; x < w; x++) {
            g_shRgba[((size_t)yy * w + x) * 4 + 3] = 255;
            for (int k = 0; k < 3; k++) rgb[((size_t)yy * w + x) * 3 + k] = src[x * 4 + k];
        }
    }
    SDL_DestroySurface(c);
    size_t len = 0;
    void *png = tdefl_write_image_to_png_file_in_memory(rgb.data(), w, h, 3, &len);
    if (!png) return;
    g_shPng.assign((unsigned char *)png, (unsigned char *)png + len);
    mz_free(png);
    CreateDirectoryA(g_shPath.substr(0, g_shPath.rfind('\\')).c_str(), NULL);
    if (FILE *f = fopen(g_shPath.c_str(), "wb")) {
        g_shFailed = fwrite(g_shPng.data(), 1, g_shPng.size(), f) != g_shPng.size();
        fclose(f);
    }
}

void share_shutdown()
{
    if (g_shList) { IM_DELETE(g_shList); g_shList = nullptr; }
    if (g_shTex) { SDL_DestroyTexture(g_shTex); g_shTex = nullptr; }
}

// From the Players tab: the card of player idx (drawn on the next frame, in share_frame)
static void share_ask(const App &a, int idx)
{
    g_shAsk = idx;
    std::string n;
    for (char ch : g_fs[(size_t)idx].name)
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_') n += ch;
    if (n.empty()) n = "Player";
    SYSTEMTIME st;
    GetLocalTime(&st);
    char b[96];
    snprintf(b, sizeof b, "ROTKPlus_%s_%04d-%02d-%02d_%02d-%02d-%02d.png", n.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour,
             st.wMinute, st.wSecond);
    g_shName = b;
    g_shPath = a.dir + "\\Screenshots\\" + g_shName;
}

// Every frame of the Fellowship page: draws an asked card, and the window with the finished one
static void share_frame(App &a)
{
    static bool envOnce = true;   // testing: ROTKPLUS_SHARE = n makes the n-th player's card (with the background picture)
    if (envOnce && (a.bg.cur.tex || ImGui::GetTime() > 6.0)) {
        envOnce = false;
        if (const char *e = getenv("ROTKPLUS_SHARE"))
            if (*e && !g_fs.empty()) share_ask(a, std::max(0, std::min((int)g_fs.size() - 1, atoi(e))));
    }
    if (g_shAsk >= 0) {
        if (g_shAsk < (int)g_fs.size()) share_build(a, g_shAsk);
        g_shAsk = -1;
    }
    if (g_shOpen) { ImGui::OpenPopup("##share"); g_shOpen = false; }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("##share", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) return;
    bool close = false;
    if (g_shFailed || !g_shTex) {
        ImGui::TextColored(C_BAD, "%s", tr("The picture couldn't be made."));
    } else {
        ImGui::TextColored(C_GOLD_H, "%s", tr("A picture to share, saved with the screenshots"));
        ImGui::TextColored(C_DIM, "%s", g_shName.c_str());
        ImGui::Spacing();
        const float iw = std::min(S(720), ImGui::GetMainViewport()->Size.x - S(120));
        ImGui::Image((ImTextureID)(intptr_t)g_shTex, ImVec2(iw, iw * k_shH / k_shW));
        ImGui::Spacing();
        if (ImGui::Button("Copy", ImVec2(0, S(36))))
            g_shCopied = clipboard_image(g_shRgba.data(), k_shW, k_shH, g_shPng);
        ImGui::SameLine();
        if (ImGui::Button("Open", ImVec2(0, S(36)))) ShellExecuteA(NULL, "open", g_shPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        ImGui::SameLine();
        if (ImGui::Button("Show in its folder", ImVec2(0, S(36))))
            ShellExecuteA(NULL, "open", "explorer.exe", ("/select,\"" + g_shPath + "\"").c_str(), NULL, SW_SHOWNORMAL);
        ImGui::SameLine();
    }
    if (ImGui::Button("Close", ImVec2(S(110), S(36))) || ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
        ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false))
        close = true;
    if (g_shCopied) ImGui::TextColored(C_GOOD, "%s", tr("Copied: paste it into a post or a chat."));
    if (close) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}
