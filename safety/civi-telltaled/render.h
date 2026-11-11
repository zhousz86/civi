#ifndef CIVI_TT_RENDER_H
#define CIVI_TT_RENDER_H
#include "telltale.h"

typedef struct {
    int display_index;   /* Screen display carrying the cluster */
    int width, height;   /* strip size in pixels (top of cluster) */
    int zorder;          /* above every HMI window */
    const char *asset_dir;
} tt_render_cfg_t;

int  tt_render_init(const tt_render_cfg_t *cfg);
void tt_render_draw(const tt_state_t *s);
void tt_render_shutdown(void);
#endif
