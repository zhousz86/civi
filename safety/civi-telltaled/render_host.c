/* Host build: prints telltale state changes so the logic can be exercised with the
 * desktop simulator (tools/vsim). On host the QML cluster draws a visual mirror. */
#include "render.h"
#include <stdio.h>

static const char *colour_name(tt_colour_t c)
{
    switch (c) {
    case TT_RED:   return "red";
    case TT_AMBER: return "amber";
    case TT_GREEN: return "green";
    default:       return "blue";
    }
}

int tt_render_init(const tt_render_cfg_t *cfg)
{
    (void)printf("civi-telltaled(host): strip %dx%d on display %d\n", cfg->width, cfg->height, cfg->display_index);
    return 0;
}

void tt_render_draw(const tt_state_t *s)
{
    (void)printf("[telltales]%s", s->source_lost ? " SOURCE-LOST" : "");
    for (int i = 0; i < (int)TT_COUNT; ++i) {
        if (tt_effective(s, (tt_id_t)i)) {
            (void)printf(" %s(%s)", TT_DEFS[i].name, colour_name(TT_DEFS[i].colour));
        }
    }
    (void)printf("\n");
    (void)fflush(stdout);
}

void tt_render_shutdown(void) {}
