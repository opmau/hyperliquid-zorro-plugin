//=============================================================================
// test_price_source_stats.cpp - Price-source visibility tests [OPM-1113]
//=============================================================================
// LAYER: Test | TESTS: src/services/hl_price_source_stats.h (production code,
//                      linked directly - not a re-implementation)
//
// The defect: a price lookup can be answered from the live feed, from a stale
// cached quote, or from an HTTP order-book request, and only the first is the
// healthy path. Which one answered was visible only at diagnostic level 2, so a
// session serving every quote over HTTP produced a level-1 log identical to one
// running entirely off the feed.
//
// The contract these tests pin: one bounded summary line per reporting interval,
// carrying a fallback share that separates a healthy session from a degraded one.
//=============================================================================

#include "../test_framework.h"
#include "../../src/services/hl_price_source_stats.h"
#include <cstring>

using namespace hl::test;
using namespace hl::market;

// Long enough to be realistic, round enough to reason about.
static const unsigned long INTERVAL = 60000ul;

// Count `n` lookups of one source into a fresh tally.
static void tallyN(PriceSourceStats& s, PriceSource src, unsigned long n) {
    for (unsigned long i = 0; i < n; ++i) tally(s, src);
}

//=============================================================================
// TALLY
//=============================================================================

TEST_CASE(tally_counts_each_source_separately) {
    PriceSourceStats s;
    tally(s, PriceSource::WsFresh);
    tally(s, PriceSource::WsFresh);
    tally(s, PriceSource::WsAfterWait);
    tally(s, PriceSource::WsStale);
    tally(s, PriceSource::HttpOk);
    tally(s, PriceSource::HttpFailed);
    tally(s, PriceSource::Unavailable);

    ASSERT_EQ(s.wsFresh,     2ul);
    ASSERT_EQ(s.wsAfterWait, 1ul);
    ASSERT_EQ(s.wsStale,     1ul);
    ASSERT_EQ(s.httpOk,      1ul);
    ASSERT_EQ(s.httpFailed,  1ul);
    ASSERT_EQ(s.unavailable, 1ul);
}

TEST_CASE(fresh_tally_starts_at_zero) {
    PriceSourceStats s;
    ASSERT_EQ(lookupsCounted(s), 0ul);
    ASSERT_EQ(s.lastReportTick, 0ul);
    ASSERT_FALSE(s.started);
}

TEST_CASE(lookups_counted_sums_every_source) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh,     10);
    tallyN(s, PriceSource::WsAfterWait,  1);
    tallyN(s, PriceSource::WsStale,      2);
    tallyN(s, PriceSource::HttpOk,       3);
    tallyN(s, PriceSource::HttpFailed,   4);
    tallyN(s, PriceSource::Unavailable,  5);
    ASSERT_EQ(lookupsCounted(s), 25ul);
}

//=============================================================================
// FALLBACK SHARE - the number that separates a healthy session from a
// degraded one, and the whole point of the tally.
//=============================================================================

TEST_CASE(fallback_share_of_empty_tally_is_zero) {
    PriceSourceStats s;
    ASSERT_EQ(fallbackSharePercent(s), 0);   // no division by zero
}

TEST_CASE(fallback_share_of_all_feed_hits_is_zero) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh, 500);
    ASSERT_EQ(fallbackSharePercent(s), 0);
}

TEST_CASE(fallback_share_of_all_http_is_one_hundred) {
    PriceSourceStats s;
    tallyN(s, PriceSource::HttpOk, 500);
    ASSERT_EQ(fallbackSharePercent(s), 100);
}

TEST_CASE(fallback_share_counts_stale_reads) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh, 50);
    tallyN(s, PriceSource::WsStale, 50);
    ASSERT_EQ(fallbackSharePercent(s), 50);
}

TEST_CASE(fallback_share_counts_failures_and_gaps) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh,     50);
    tallyN(s, PriceSource::HttpFailed,  25);
    tallyN(s, PriceSource::Unavailable, 25);
    ASSERT_EQ(fallbackSharePercent(s), 50);
}

// Data that arrived during the short first-data wait came from the feed. A
// burst of it at startup is expected, so counting it as fallback would report a
// healthy session as degraded for its first interval.
TEST_CASE(fallback_share_excludes_data_that_arrived_during_the_wait) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh,     90);
    tallyN(s, PriceSource::WsAfterWait, 10);
    ASSERT_EQ(fallbackSharePercent(s), 0);
}

TEST_CASE(fallback_share_rounds_to_nearest_percent) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh, 2);
    tallyN(s, PriceSource::HttpOk,  1);
    ASSERT_EQ(fallbackSharePercent(s), 33);   // 33.33 -> 33

    PriceSourceStats t;
    tallyN(t, PriceSource::WsFresh, 1);
    tallyN(t, PriceSource::HttpOk,  2);
    ASSERT_EQ(fallbackSharePercent(t), 67);   // 66.67 -> 67
}

// The regression this module exists for: two sessions doing the same amount of
// work must not produce the same number.
TEST_CASE(healthy_and_degraded_sessions_are_distinguishable) {
    PriceSourceStats healthy;
    tallyN(healthy, PriceSource::WsFresh, 1000);

    PriceSourceStats degraded;
    tallyN(degraded, PriceSource::HttpOk, 1000);

    ASSERT_EQ(lookupsCounted(healthy), lookupsCounted(degraded));
    ASSERT_NE(fallbackSharePercent(healthy), fallbackSharePercent(degraded));
}

//=============================================================================
// REPORTING SCHEDULE
//=============================================================================

TEST_CASE(report_not_due_within_interval) {
    ASSERT_FALSE(reportDue(10000ul, 0ul, INTERVAL));
    ASSERT_FALSE(reportDue(59999ul, 0ul, INTERVAL));
}

TEST_CASE(report_due_at_interval_boundary) {
    ASSERT_TRUE(reportDue(60000ul, 0ul, INTERVAL));
}

TEST_CASE(report_due_after_interval) {
    ASSERT_TRUE(reportDue(500000ul, 100000ul, INTERVAL));
}

// GetTickCount() wraps to 0 after ~49.7 days. Unsigned subtraction stays correct
// across the wrap; a signed comparison would either report a report as
// permanently overdue or never due again for the rest of the process.
TEST_CASE(report_schedule_survives_tick_wraparound) {
    const unsigned long justBeforeWrap = 0xFFFFFF00ul;   // 256ms short of wrapping

    ASSERT_FALSE(reportDue(0x0000000Ful, justBeforeWrap, INTERVAL));  // 271ms elapsed
    ASSERT_TRUE(reportDue(0x0000FFFFul, justBeforeWrap, INTERVAL));   // ~65.8s elapsed
}

//=============================================================================
// LINE FORMAT - fixed so it stays greppable across releases
//=============================================================================

TEST_CASE(format_report_renders_every_counter) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh,     70);
    tallyN(s, PriceSource::WsAfterWait,  1);
    tallyN(s, PriceSource::WsStale,      9);
    tallyN(s, PriceSource::HttpOk,      15);
    tallyN(s, PriceSource::HttpFailed,   4);
    tallyN(s, PriceSource::Unavailable,  1);

    char buf[192];
    formatReport(s, buf, sizeof(buf));
    ASSERT_STREQ(buf, "ws=70 wait=1 stale=9 http=15 failed=4 none=1 (fallback 29%)");
}

TEST_CASE(format_report_of_a_healthy_interval) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh, 240);

    char buf[192];
    formatReport(s, buf, sizeof(buf));
    ASSERT_STREQ(buf, "ws=240 wait=0 stale=0 http=0 failed=0 none=0 (fallback 0%)");
}

TEST_CASE(format_report_tolerates_a_null_buffer) {
    PriceSourceStats s;
    formatReport(s, nullptr, 0);       // must not crash
    formatReport(s, nullptr, 64);
    char buf[8];
    formatReport(s, buf, 0);
    ASSERT_TRUE(true);
}

//=============================================================================
// RESET
//=============================================================================

TEST_CASE(reset_zeroes_counts_and_stamps_the_interval) {
    PriceSourceStats s;
    tallyN(s, PriceSource::WsFresh, 5);
    tallyN(s, PriceSource::HttpOk,  5);

    resetCounts(s, 123456ul);

    ASSERT_EQ(lookupsCounted(s), 0ul);
    ASSERT_EQ(s.lastReportTick, 123456ul);
    ASSERT_TRUE(s.started);
}

//=============================================================================
// RECORD - the whole cycle: count, report on schedule, start over
//=============================================================================

TEST_CASE(first_lookup_starts_the_clock_without_reporting) {
    PriceSourceStats s;
    char buf[192];

    // A large tick against a zero lastReportTick would otherwise look overdue
    // and emit a line describing a single sample.
    const char* line = recordInto(s, PriceSource::WsFresh, 5000000ul, INTERVAL, buf, sizeof(buf));

    ASSERT_NULL(line);
    ASSERT_TRUE(s.started);
    ASSERT_EQ(s.lastReportTick, 5000000ul);
    ASSERT_EQ(s.wsFresh, 1ul);
}

TEST_CASE(lookups_within_the_interval_are_counted_but_not_reported) {
    PriceSourceStats s;
    char buf[192];

    recordInto(s, PriceSource::WsFresh, 1000ul, INTERVAL, buf, sizeof(buf));
    for (unsigned long t = 2000; t < 60000; t += 1000)
        ASSERT_NULL(recordInto(s, PriceSource::WsFresh, t, INTERVAL, buf, sizeof(buf)));

    ASSERT_EQ(s.wsFresh, 59ul);
}

TEST_CASE(interval_elapsed_emits_one_line_and_starts_over) {
    PriceSourceStats s;
    char buf[192];

    recordInto(s, PriceSource::WsFresh, 1000ul, INTERVAL, buf, sizeof(buf));   // starts clock
    tallyN(s, PriceSource::HttpOk, 3);

    const char* line = recordInto(s, PriceSource::HttpOk, 61000ul, INTERVAL, buf, sizeof(buf));

    ASSERT_NOT_NULL(line);
    ASSERT_STREQ(line, "ws=1 wait=0 stale=0 http=4 failed=0 none=0 (fallback 80%)");
    ASSERT_EQ(lookupsCounted(s), 0ul);        // counts cleared for the next interval
    ASSERT_EQ(s.lastReportTick, 61000ul);
}

TEST_CASE(each_interval_reports_only_its_own_lookups) {
    PriceSourceStats s;
    char buf[192];

    recordInto(s, PriceSource::WsFresh, 0ul, INTERVAL, buf, sizeof(buf));
    tallyN(s, PriceSource::WsFresh, 99);
    const char* first = recordInto(s, PriceSource::WsFresh, 60000ul, INTERVAL, buf, sizeof(buf));
    ASSERT_NOT_NULL(first);
    ASSERT_STREQ(first, "ws=101 wait=0 stale=0 http=0 failed=0 none=0 (fallback 0%)");

    // Second interval: the feed has gone, every lookup is served over HTTP.
    tallyN(s, PriceSource::HttpOk, 9);
    const char* second = recordInto(s, PriceSource::HttpOk, 120000ul, INTERVAL, buf, sizeof(buf));
    ASSERT_NOT_NULL(second);
    ASSERT_STREQ(second, "ws=0 wait=0 stale=0 http=10 failed=0 none=0 (fallback 100%)");
}

// A stalled tick loop is the case the diagnostic is for: few lookups, long gaps.
// Bounding the report by time rather than by call count means a line still
// appears when the caller has almost stopped asking.
TEST_CASE(a_near_idle_caller_still_produces_a_line) {
    PriceSourceStats s;
    char buf[192];

    recordInto(s, PriceSource::HttpOk, 0ul, INTERVAL, buf, sizeof(buf));
    const char* line = recordInto(s, PriceSource::HttpOk, 300000ul, INTERVAL, buf, sizeof(buf));

    ASSERT_NOT_NULL(line);
    ASSERT_STREQ(line, "ws=0 wait=0 stale=0 http=2 failed=0 none=0 (fallback 100%)");
}

TEST_CASE(a_busy_caller_produces_one_line_per_interval_not_per_lookup) {
    PriceSourceStats s;
    char buf[192];
    unsigned long lines = 0;

    // Five minutes of lookups, ten per second.
    for (unsigned long t = 0; t <= 300000ul; t += 100ul)
        if (recordInto(s, PriceSource::WsFresh, t, INTERVAL, buf, sizeof(buf))) ++lines;

    ASSERT_EQ(lines, 5ul);      // not 3001
}

TEST_CASE(process_wide_tally_is_one_object) {
    ASSERT_TRUE(&priceSourceStats() == &priceSourceStats());
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-1113: Price Source Visibility Tests ===\n\n");

    printf("-- tally --\n");
    RUN_TEST(tally_counts_each_source_separately);
    RUN_TEST(fresh_tally_starts_at_zero);
    RUN_TEST(lookups_counted_sums_every_source);

    printf("\n-- fallback share --\n");
    RUN_TEST(fallback_share_of_empty_tally_is_zero);
    RUN_TEST(fallback_share_of_all_feed_hits_is_zero);
    RUN_TEST(fallback_share_of_all_http_is_one_hundred);
    RUN_TEST(fallback_share_counts_stale_reads);
    RUN_TEST(fallback_share_counts_failures_and_gaps);
    RUN_TEST(fallback_share_excludes_data_that_arrived_during_the_wait);
    RUN_TEST(fallback_share_rounds_to_nearest_percent);
    RUN_TEST(healthy_and_degraded_sessions_are_distinguishable);

    printf("\n-- reporting schedule --\n");
    RUN_TEST(report_not_due_within_interval);
    RUN_TEST(report_due_at_interval_boundary);
    RUN_TEST(report_due_after_interval);
    RUN_TEST(report_schedule_survives_tick_wraparound);

    printf("\n-- line format --\n");
    RUN_TEST(format_report_renders_every_counter);
    RUN_TEST(format_report_of_a_healthy_interval);
    RUN_TEST(format_report_tolerates_a_null_buffer);

    printf("\n-- reset --\n");
    RUN_TEST(reset_zeroes_counts_and_stamps_the_interval);

    printf("\n-- record cycle --\n");
    RUN_TEST(first_lookup_starts_the_clock_without_reporting);
    RUN_TEST(lookups_within_the_interval_are_counted_but_not_reported);
    RUN_TEST(interval_elapsed_emits_one_line_and_starts_over);
    RUN_TEST(each_interval_reports_only_its_own_lookups);
    RUN_TEST(a_near_idle_caller_still_produces_a_line);
    RUN_TEST(a_busy_caller_produces_one_line_per_interval_not_per_lookup);
    RUN_TEST(process_wide_tally_is_one_object);

    return printTestSummary();
}
