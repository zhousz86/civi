/* Unit tests for the telltale state model (CIVI-PLT-002, -HMI-008). */
#include "telltale.h"
#include <stdio.h>
#include <string.h>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { (void)printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++failures; } } while (0)

int main(void)
{
    tt_state_t s;
    tt_init(&s, 1000);
    CHECK(s.source_lost);                         /* lost until first heartbeat */
    CHECK(tt_effective(&s, TT_CHECK));
    CHECK(!tt_supervise(&s, 1100, 300));          /* time alone never clears loss */
    CHECK(s.source_lost);

    char buf[] = "@telltales\nseq:n:1\nseatbelt:b:true\nabs:b:false\n";
    CHECK(tt_apply_buffer(&s, buf, 1150));
    CHECK(!s.source_lost);
    CHECK(tt_effective(&s, TT_SEATBELT));
    CHECK(!tt_effective(&s, TT_CHECK));

    CHECK(!tt_supervise(&s, 1400, 300));          /* within timeout */
    CHECK(tt_supervise(&s, 1500, 300));           /* heartbeat stale -> fail-safe */
    CHECK(tt_effective(&s, TT_CHECK));
    CHECK(tt_effective(&s, TT_SEATBELT));         /* last known lamps stay lit */

    CHECK(!tt_apply_line(&s, "seq:n:1", 1600));   /* same seq: not a heartbeat */
    CHECK(s.source_lost);
    CHECK(tt_apply_line(&s, "seq:n:2", 1700));    /* new seq: recovered */
    CHECK(!s.source_lost);

    CHECK(!tt_apply_line(&s, "garbage", 1800));   /* malformed lines ignored */
    CHECK(!tt_apply_line(&s, "unknown_lamp:b:true", 1800));

    (void)printf("tst_telltale: %s\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
