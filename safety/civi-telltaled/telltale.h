/* Telltale state model for civi-telltaled.
 * Written to a MISRA-C:2012-friendly subset (fixed-size storage, no dynamic memory,
 * no recursion) so it can be carried into the cluster's safety argument. */
#ifndef CIVI_TELLTALE_H
#define CIVI_TELLTALE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TT_TURN_LEFT = 0, TT_SEATBELT, TT_BRAKE, TT_ABS, TT_AIRBAG,
    TT_ESC, TT_TPMS, TT_CHECK, TT_HIGH_BEAM, TT_TURN_RIGHT,
    TT_COUNT
} tt_id_t;

typedef enum { TT_RED, TT_AMBER, TT_GREEN, TT_BLUE } tt_colour_t;

typedef struct {
    const char *name;        /* attribute name in the vs/telltales PPS object */
    tt_colour_t colour;      /* ISO 2575 colour class */
} tt_def_t;

typedef struct {
    bool on[TT_COUNT];
    uint32_t seq;            /* publisher heartbeat */
    uint64_t last_seq_change_ms;
    bool source_lost;        /* heartbeat stale => fail-safe indication */
} tt_state_t;

extern const tt_def_t TT_DEFS[TT_COUNT];

void tt_init(tt_state_t *s, uint64_t now_ms);
/* Apply one PPS line "name:b:true" / "seq:n:42". Returns true if visible state changed. */
bool tt_apply_line(tt_state_t *s, const char *line, uint64_t now_ms);
/* Apply a whole PPS read buffer (NUL-terminated). Returns true if visible state changed. */
bool tt_apply_buffer(tt_state_t *s, char *buf, uint64_t now_ms);
/* Heartbeat supervision: declares source loss after timeout_ms without a new seq;
 * only a new heartbeat clears it. Returns true if visible state changed. */
bool tt_supervise(tt_state_t *s, uint64_t now_ms, uint32_t timeout_ms);
/* Effective lamp state: source loss lights CHECK so the driver is never shown a
 * silently frozen cluster. */
bool tt_effective(const tt_state_t *s, tt_id_t id);

#endif
