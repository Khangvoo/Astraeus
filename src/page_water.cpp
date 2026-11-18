// The tools page's water tab: the day's glasses, and how often to be reminded. It shares its numbers with the Drink Water widget,
// so a glass counted here shows there and the other way round. The reminder itself runs from main.cpp's once-a-second tick.
#include <algorithm>
#include <cstdio>
#include <string>

#include "faces_state.h"
#include "glyphs.h"
#include "pages.h"
#include "water.h"

namespace {

constexpr float kRowA = 52, kChipH = 28, kGap = 8, kLabelW = 58;

std::wstring Volume(int ml) { // 750 ml, 1.25 L
    wchar_t buf[24]{};
    if (ml < 1000) swprintf_s(buf, L"%d ml", ml);
    else swprintf_s(buf, L"%.3g L", ml / 1000.0);
    return buf;
}

std::wstring EveryLabel(int minutes) {
    if (minutes <= 0) return L"Off";
    if (minutes % 60 == 0) return std::to_wstring(minutes / 60) + L"h";
    return std::to_wstring(minutes) + L"m";
}

std::wstring HoursLabel(int from, int to) {
    if (from == to) return L"Hours: all day";
    wchar_t buf[32]{};
    swprintf_s(buf, L"Hours: %d–%d", from, to);
    return buf;
}

} // namespace

void WaterTab(PageCtx& p, const D2D1_RECT_F& r) {
    Ui& ui = p.ui;
    Canvas& c = ui.C();
    const Theme& t = p.shell;
    namespace st = faces::state;

    int have = st::Water(), goal = st::WaterGoal(), glassMl = st::WaterGlassMl(), every = st::WaterRemindMin();
    int from = 0, to = 0;
    st::WaterWindow(from, to);
    float width = Width(r);

    // the day so far: a ring, the count, and a glass more or less
    D2D1_POINT_2F ring = {r.left + 26, r.top + kRowA / 2};
    w::Gauge(ui, t, Ui::Id("wt.ring"), ring, 24, 4, goal > 0 ? (std::min)(1.0f, (float)have / (float)goal) : 0.0f);
    c.Icon(glyph::Coffee, ring, 16, have >= goal ? t.accentHi : t.accent);
    float tx = r.left + 62;
    c.Text(std::to_wstring(have) + L" of " + std::to_wstring(goal), Rect(tx, r.top - 2, 130, 34), Weight::Thin, 28, t.text);
    std::wstring detail = have >= goal ? L"Goal reached  ·  " + Volume(have * glassMl) : L"glasses  ·  " + Volume(have * glassMl);
    c.Text(detail, Rect(tx, r.top + 32, (std::max)(60.0f, width - 62 - 150), 16), Weight::Light, 11, t.textDim);

    D2D1_RECT_F more = Rect(r.right - 86, r.top + 10, 86, 32), less = Rect(more.left - kGap - 44, r.top + 10, 44, 32);
    if (w::Chip(ui, t, Ui::Id("wt.less"), less, L"−1", false)) st::WaterAdd(-1);
    if (w::Chip(ui, t, Ui::Id("wt.more"), more, L"+1 glass", true)) st::WaterAdd(1);

    // how often to be reminded
    float y = r.top + kRowA + kGap;
    c.Text(L"Remind", Rect(r.left, y, kLabelW, kChipH), Weight::Light, 11, t.textDim);
    constexpr int kN = (int)(sizeof(water::kEvery) / sizeof(water::kEvery[0]));
    float cw = (width - kLabelW - (kN - 1) * 5) / kN;
    for (int i = 0; i < kN; ++i) {
        int minutes = water::kEvery[i];
        if (w::Chip(ui, t, Ui::Id("wt.every", i), Rect(r.left + kLabelW + i * (cw + 5), y, cw, kChipH), EveryLabel(minutes), every == minutes))
            st::SetWaterRemindMin(minutes);
    }

    // goal, hours and glass size: each one steps to the next choice on a click
    y += kChipH + kGap;
    float tw = (width - 2 * 5) / 3;
    if (w::Chip(ui, t, Ui::Id("wt.goal"), Rect(r.left, y, tw, kChipH), L"Goal: " + std::to_wstring(goal), false))
        st::SetWaterGoal(water::NextAbove(water::kGoals, goal));
    if (w::Chip(ui, t, Ui::Id("wt.hours"), Rect(r.left + tw + 5, y, tw, kChipH), HoursLabel(from, to), false)) {
        water::Window next = water::NextWindow(from, to);
        st::SetWaterWindow(next.from, next.to);
    }
    if (w::Chip(ui, t, Ui::Id("wt.ml"), Rect(r.left + 2 * (tw + 5), y, tw, kChipH), L"Glass: " + Volume(glassMl), false))
        st::SetWaterGlassMl(water::NextAbove(water::kGlassMl, glassMl));
}
