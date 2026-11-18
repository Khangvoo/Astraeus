#pragma once
// The water reminder's rules, with nothing of windows in them: when a nudge is due, which hours it keeps to, and the few
// choices the water tab cycles through. faces_state.cpp keeps the settings and the clock; this only decides.
#include <cstddef>

namespace water {

constexpr int kDefaultFrom = 8, kDefaultTo = 22; // remind from 08:00 up to 22:00
constexpr int kGoals[] = {4, 6, 8, 10, 12, 14}; // glasses a day, what the tab steps through
constexpr int kGlassMl[] = {200, 250, 330, 500};
constexpr int kDefaultGlassMl = 250;
constexpr int kEvery[] = {0, 30, 45, 60, 120}; // minutes between nudges, 0 is off

struct Window {
    int from, to;
};
constexpr Window kWindows[] = {{8, 22}, {7, 23}, {9, 18}, {0, 0}}; // {0, 0}: all day

// Whether an hour (0..23) is inside the window. from == to means all day; from > to runs past midnight.
inline bool InWindow(int hour, int from, int to) {
    if (from == to) return true;
    if (from < to) return hour >= from && hour < to;
    return hour >= from || hour < to;
}

// The next value in the list above the current one; past the last (or below all) it wraps to the first.
template <size_t N>
int NextAbove(const int (&list)[N], int current) {
    for (size_t i = 0; i < N; ++i)
        if (list[i] > current) return list[i];
    return list[0];
}

// The window after this one in kWindows; one that isn't in the list goes to the first.
inline Window NextWindow(int from, int to) {
    constexpr size_t n = sizeof(kWindows) / sizeof(kWindows[0]);
    for (size_t i = 0; i < n; ++i)
        if (kWindows[i].from == from && kWindows[i].to == to) return kWindows[(i + 1) % n];
    return kWindows[0];
}

// Called once a second. last is the wall-clock second of the last nudge or sip (0: the clock isn't running yet).
// True when a nudge is due now. goalMet is only asked when everything else says "now": it reads settings.
template <class GoalMet>
bool Due(long long& last, long long now, int hour, int everyMin, int from, int to, GoalMet goalMet) {
    if (everyMin <= 0) { // off: the clock starts fresh when it's switched on
        last = 0;
        return false;
    }
    if (last == 0) { // just started: the first nudge is a whole interval away
        last = now;
        return false;
    }
    if (now - last < (long long)everyMin * 60) return false;
    if (!InWindow(hour, from, to)) return false; // overnight: waits, and nudges when the window opens
    last = now; // asked, whatever the answer, so a met goal isn't asked again each second
    return !goalMet();
}

} // namespace water
