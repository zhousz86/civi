#include "telltale.h"
#include <string.h>
#include <stdlib.h>

const tt_def_t TT_DEFS[TT_COUNT] = {
    { "turn_left",  TT_GREEN }, { "seatbelt", TT_RED },   { "brake",  TT_RED },
    { "abs",        TT_AMBER }, { "airbag",   TT_RED },   { "esc",    TT_AMBER },
    { "tpms",       TT_AMBER }, { "check",    TT_AMBER }, { "high_beam", TT_BLUE },
    { "turn_right", TT_GREEN },
};

void tt_init(tt_state_t *s, uint64_t now_ms)
{
    (void)memset(s, 0, sizeof *s);
    s->last_seq_change_ms = now_ms;
    s->source_lost = true;       /* until the first heartbeat arrives */
}

bool tt_apply_line(tt_state_t *s, const char *line, uint64_t now_ms)
{
    const char *c1 = strchr(line, ':');
    if (c1 == NULL || c1 == line) {
        return false;
    }
    const char *c2 = strchr(c1 + 1, ':');
    if (c2 == NULL) {
        return false;
    }
    const size_t klen = (size_t)(c1 - line);
    const char *val = c2 + 1;

    if (klen == 3U && strncmp(line, "seq", 3U) == 0) {
        const uint32_t seq = (uint32_t)strtoul(val, NULL, 10);
        if (seq != s->seq) {
            s->seq = seq;
            s->last_seq_change_ms = now_ms;
            if (s->source_lost) {
                s->source_lost = false;
                return true;
            }
        }
        return false;
    }
    for (int i = 0; i < (int)TT_COUNT; ++i) {
        if (strlen(TT_DEFS[i].name) == klen && strncmp(line, TT_DEFS[i].name, klen) == 0) {
            const bool on = (strncmp(val, "true", 4U) == 0) || (val[0] == '1');
            if (on != s->on[i]) {
                s->on[i] = on;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool tt_apply_buffer(tt_state_t *s, char *buf, uint64_t now_ms)
{
    bool changed = false;
    char *save = NULL;
    for (char *line = strtok_r(buf, "\n", &save); line != NULL; line = strtok_r(NULL, "\n", &save)) {
        if (line[0] != '@') {
            changed = tt_apply_line(s, line, now_ms) || changed;
        }
    }
    return changed;
}

bool tt_supervise(tt_state_t *s, uint64_t now_ms, uint32_t timeout_ms)
{
    /* Supervision only ever *declares* loss. Recovery requires a fresh heartbeat
     * (tt_apply_line on "seq"), never the mere passage of time — otherwise the
     * renderer would report a healthy source before the first sample arrived. */
    if (!s->source_lost && (now_ms - s->last_seq_change_ms) > (uint64_t)timeout_ms) {
        s->source_lost = true;
        return true;
    }
    return false;
}

bool tt_effective(const tt_state_t *s, tt_id_t id)
{
    if (id == TT_CHECK && s->source_lost) {
        return true;
    }
    return s->on[id];
}
