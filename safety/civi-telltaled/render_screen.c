/* QNX Screen back-end for civi-telltaled (QNX SDP 8.0).
 *
 * CPU-rendered into a mapped window buffer: no GPU, no EGL, no shader compiler in the
 * telltale path, so an Adreno hang or a GPU reset caused by infotainment cannot take the
 * telltales with it (CIVI-PLT-001 GPU-access isolation). On Snapdragon cockpit SoCs the
 * window can be bound to a dedicated display-controller layer with
 * SCREEN_PROPERTY_PIPELINE (pipeline ids come from the BSP's graphics.conf), which keeps
 * composition of the strip independent of the HMI's layers. */
#include "render.h"

#include <screen/screen.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TT_ICON_PX 64
#define TT_MAX_DISPLAYS 8

static screen_context_t g_ctx;
static screen_window_t g_win;
static int g_w, g_h;
static uint32_t g_icons[TT_COUNT][TT_ICON_PX * TT_ICON_PX];
static int g_has_icon[TT_COUNT];

static uint32_t colour_argb(tt_colour_t c)
{
    switch (c) {
    case TT_RED:   return 0xFFFF3B30u;
    case TT_AMBER: return 0xFFFFB000u;
    case TT_GREEN: return 0xFF2BD96Bu;
    default:       return 0xFF3D8BFFu;
    }
}

/* Approved ISO 2575 artwork is supplied by the OEM as raw 64x64 ARGB32 files. */
static void load_icons(const char *dir)
{
    char path[256];
    for (int i = 0; i < (int)TT_COUNT; ++i) {
        g_has_icon[i] = 0;
        if (dir == NULL) {
            continue;
        }
        (void)snprintf(path, sizeof path, "%s/%s.rgba", dir, TT_DEFS[i].name);
        FILE *f = fopen(path, "rb");
        if (f != NULL) {
            const size_t n = fread(g_icons[i], sizeof(uint32_t), (size_t)TT_ICON_PX * TT_ICON_PX, f);
            g_has_icon[i] = (n == (size_t)TT_ICON_PX * TT_ICON_PX) ? 1 : 0;
            (void)fclose(f);
        }
    }
}

int tt_render_init(const tt_render_cfg_t *cfg)
{
    g_w = cfg->width;
    g_h = cfg->height;
    load_icons(cfg->asset_dir);

    if (screen_create_context(&g_ctx, SCREEN_APPLICATION_CONTEXT) != 0) {
        perror("screen_create_context");
        return -1;
    }
    int ndisp = 0;
    (void)screen_get_context_property_iv(g_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &ndisp);
    if (ndisp <= cfg->display_index || ndisp > TT_MAX_DISPLAYS) {
        (void)fprintf(stderr, "civi-telltaled: display %d not present (%d displays)\n", cfg->display_index, ndisp);
        return -1;
    }
    screen_display_t displays[TT_MAX_DISPLAYS];
    (void)screen_get_context_property_pv(g_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)displays);

    if (screen_create_window(&g_win, g_ctx) != 0) {
        perror("screen_create_window");
        return -1;
    }
    (void)screen_set_window_property_pv(g_win, SCREEN_PROPERTY_DISPLAY, (void **)&displays[cfg->display_index]);
    int usage = SCREEN_USAGE_WRITE | SCREEN_USAGE_NATIVE;
    int format = SCREEN_FORMAT_RGBA8888;
    int size[2] = { g_w, g_h };
    int pos[2] = { 0, 0 };
    int zorder = cfg->zorder;
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_USAGE, &usage);
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_FORMAT, &format);
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_SIZE, size);
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_POSITION, pos);
    (void)screen_set_window_property_iv(g_win, SCREEN_PROPERTY_ZORDER, &zorder);
    (void)screen_set_window_property_cv(g_win, SCREEN_PROPERTY_ID_STRING, 18, "civi-telltale-strip");
    if (screen_create_window_buffers(g_win, 2) != 0) {
        perror("screen_create_window_buffers");
        return -1;
    }
    return 0;
}

static void fill(uint32_t *px, int stride_px, int x0, int y0, int w, int h, uint32_t argb)
{
    for (int y = y0; y < y0 + h && y < g_h; ++y) {
        for (int x = x0; x < x0 + w && x < g_w; ++x) {
            px[y * stride_px + x] = argb;
        }
    }
}

void tt_render_draw(const tt_state_t *s)
{
    screen_buffer_t bufs[2];
    void *ptr = NULL;
    int stride = 0;
    if (screen_get_window_property_pv(g_win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)bufs) != 0) {
        return;
    }
    (void)screen_get_buffer_property_pv(bufs[0], SCREEN_PROPERTY_POINTER, &ptr);
    (void)screen_get_buffer_property_iv(bufs[0], SCREEN_PROPERTY_STRIDE, &stride);
    if (ptr == NULL || stride <= 0) {
        return;
    }
    uint32_t *px = (uint32_t *)ptr;
    const int sp = stride / 4;
    fill(px, sp, 0, 0, g_w, g_h, 0xFF05080Bu);

    const int slot = 86, gap = 26;
    const int total = (int)TT_COUNT * slot + ((int)TT_COUNT - 1) * gap;
    int x = (g_w - total) / 2;
    const int y = (g_h - TT_ICON_PX) / 2;
    for (int i = 0; i < (int)TT_COUNT; ++i, x += slot + gap) {
        if (!tt_effective(s, (tt_id_t)i)) {
            continue;   /* unlit telltales are not drawn (no "ghost" symbols) */
        }
        if (g_has_icon[i]) {
            for (int r = 0; r < TT_ICON_PX; ++r) {
                (void)memcpy(&px[(y + r) * sp + x + (slot - TT_ICON_PX) / 2], &g_icons[i][r * TT_ICON_PX],
                             (size_t)TT_ICON_PX * sizeof(uint32_t));
            }
        } else {
            fill(px, sp, x, y, slot, TT_ICON_PX, colour_argb(TT_DEFS[i].colour));  /* placeholder lamp */
        }
    }
    int dirty[4] = { 0, 0, g_w, g_h };
    (void)screen_post_window(g_win, bufs[0], 1, dirty, 0);
}

void tt_render_shutdown(void)
{
    (void)screen_destroy_window(g_win);
    (void)screen_destroy_context(g_ctx);
}
