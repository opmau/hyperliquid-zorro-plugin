//=============================================================================
// hl_price_source_stats.h - Which source answered each price lookup
//=============================================================================
// LAYER: Services
// DEPENDENCIES: <cstdio>, <cstddef> only
// THREAD SAFETY: The pure functions are stateless. The process-wide tally
//                behind recordPriceSource() is unguarded: it is updated from
//                the price-lookup path, which runs on the broker's calling
//                thread, and a lost increment costs one diagnostic line's
//                accuracy and nothing else.
//
// A price lookup can be answered from the live feed, from a stale cached quote,
// or from an HTTP order-book request, and the three differ by orders of
// magnitude in how long they hold up the caller. Only the first is the healthy
// path. Without a tally, the choice was visible only at diagnostic level 2, so
// a session answering every lookup over HTTP produced a level-1 log identical
// to one running entirely off the feed — the degradation that matters most was
// the one that left no trace.
//
// Volume is bounded by time, not by call count: one line per reporting
// interval regardless of how many symbols are quoted or how fast the strategy
// ticks.
//
// Header-only and free of globals, HTTP and Zorro on purpose, so the unit tests
// link THIS code rather than a copy of it. hl_market_service.cpp is the shell
// that supplies the clock and writes the line out. [OPM-1113]
//=============================================================================

#pragma once

#include <cstdio>
#include <cstddef>

namespace hl {
namespace market {

/// Where a single price lookup was answered from.
enum class PriceSource {
    WsFresh,      ///< Feed cache hit inside the caller's max age — healthy path
    WsAfterWait,  ///< Cache was empty; feed data arrived during the short wait
    WsStale,      ///< Cache past max age, served because HTTP was on cooldown
    HttpOk,       ///< Fetched over the HTTP order-book fallback
    HttpFailed,   ///< HTTP fallback attempted, no usable quote came back
    Unavailable   ///< No quote available from any source
};

/// Counts accumulated since the last report.
struct PriceSourceStats {
    unsigned long wsFresh;
    unsigned long wsAfterWait;
    unsigned long wsStale;
    unsigned long httpOk;
    unsigned long httpFailed;
    unsigned long unavailable;
    unsigned long lastReportTick;
    bool started;

    PriceSourceStats()
        : wsFresh(0), wsAfterWait(0), wsStale(0), httpOk(0), httpFailed(0),
          unavailable(0), lastReportTick(0), started(false) {}
};

/// Add one lookup to the tally.
inline void tally(PriceSourceStats& s, PriceSource src) {
    switch (src) {
        case PriceSource::WsFresh:     ++s.wsFresh;     break;
        case PriceSource::WsAfterWait: ++s.wsAfterWait; break;
        case PriceSource::WsStale:     ++s.wsStale;     break;
        case PriceSource::HttpOk:      ++s.httpOk;      break;
        case PriceSource::HttpFailed:  ++s.httpFailed;  break;
        case PriceSource::Unavailable: ++s.unavailable; break;
    }
}

/// Lookups counted since the last report.
inline unsigned long lookupsCounted(const PriceSourceStats& s) {
    return s.wsFresh + s.wsAfterWait + s.wsStale
         + s.httpOk + s.httpFailed + s.unavailable;
}

/// Share of lookups the live feed could not answer with a current quote, as a
/// whole percent. Stale reads, HTTP fallbacks and outright failures all count:
/// each means the feed had nothing current when it was asked. Data that arrived
/// during the short first-data wait does not — it came from the feed, and a
/// burst of it at startup is expected rather than a symptom.
inline int fallbackSharePercent(const PriceSourceStats& s) {
    unsigned long total = lookupsCounted(s);
    if (total == 0) return 0;
    unsigned long fallback = s.wsStale + s.httpOk + s.httpFailed + s.unavailable;
    return (int)((fallback * 100 + total / 2) / total);
}

/// True when the reporting interval has elapsed. Tick arithmetic is unsigned on
/// purpose: GetTickCount() wraps to 0 after ~49.7 days, and `now - last` stays
/// correct across the wrap where a signed comparison would not.
inline bool reportDue(unsigned long nowTick, unsigned long lastReportTick,
                      unsigned long intervalMs) {
    return (unsigned long)(nowTick - lastReportTick) >= intervalMs;
}

/// Render the tally as one log line. The format is fixed so it stays greppable
/// across releases.
inline void formatReport(const PriceSourceStats& s, char* buf, size_t bufLen) {
    if (!buf || bufLen == 0) return;
    snprintf(buf, bufLen,
             "ws=%lu wait=%lu stale=%lu http=%lu failed=%lu none=%lu (fallback %d%%)",
             s.wsFresh, s.wsAfterWait, s.wsStale,
             s.httpOk, s.httpFailed, s.unavailable,
             fallbackSharePercent(s));
}

/// Zero the counts and start a fresh interval at nowTick, so the next line
/// covers the interval that follows rather than one overlapping it.
inline void resetCounts(PriceSourceStats& s, unsigned long nowTick) {
    s.wsFresh = s.wsAfterWait = s.wsStale = 0;
    s.httpOk = s.httpFailed = s.unavailable = 0;
    s.lastReportTick = nowTick;
    s.started = true;
}

/// Count one lookup into `s` and, when the interval has elapsed, render the
/// line into `buf` and start a fresh interval. Returns buf when a line is due,
/// otherwise nullptr, so a caller emits exactly one bounded line per interval.
/// The first lookup only starts the clock — it never reports, which would
/// otherwise produce a line describing a single sample.
inline const char* recordInto(PriceSourceStats& s, PriceSource src,
                              unsigned long nowTick, unsigned long intervalMs,
                              char* buf, size_t bufLen) {
    if (!s.started) {
        s.started = true;
        s.lastReportTick = nowTick;
    }
    tally(s, src);
    if (!reportDue(nowTick, s.lastReportTick, intervalMs)) return nullptr;
    formatReport(s, buf, bufLen);
    resetCounts(s, nowTick);
    return buf;
}

/// The process-wide tally. A function-local static in an inline function is one
/// object across every translation unit that uses it.
inline PriceSourceStats& priceSourceStats() {
    static PriceSourceStats s;
    return s;
}

/// recordInto() against the process-wide tally.
inline const char* recordPriceSource(PriceSource src, unsigned long nowTick,
                                     unsigned long intervalMs,
                                     char* buf, size_t bufLen) {
    return recordInto(priceSourceStats(), src, nowTick, intervalMs, buf, bufLen);
}

} // namespace market
} // namespace hl
