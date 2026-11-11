/* civi-telltaled — cluster telltale renderer (CIVI-PLT-001/-002/-003, -HMI-008).
 *
 *   civi-telltaled [-d display] [-w width] [-h height] [-z zorder] [-t timeout_ms] [-a asset_dir]
 *
 * Reads the flat PPS object <root>/vs/telltales (name:b:true ..., seq:n:N heartbeat).
 * On QNX it blocks on "?wait,delta" with a short select() timeout so heartbeat
 * supervision runs even when no data arrives; on host it polls the simulator's file. */
#define _POSIX_C_SOURCE 200809L
#include "render.h"
#include "telltale.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t g_run = 1;
static void on_signal(int sig) { (void)sig; g_run = 0; }

static void sleep_ms(long ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    (void)nanosleep(&ts, NULL);
}

static uint64_t now_ms(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

int main(int argc, char **argv)
{
    tt_render_cfg_t cfg = { 0, 1920, 84, 1000, "/usr/share/civi/telltales" };
    uint32_t timeout_ms = 300;
    int opt;
    while ((opt = getopt(argc, argv, "d:w:h:z:t:a:")) != -1) {
        switch (opt) {
        case 'd': cfg.display_index = atoi(optarg); break;
        case 'w': cfg.width = atoi(optarg); break;
        case 'h': cfg.height = atoi(optarg); break;
        case 'z': cfg.zorder = atoi(optarg); break;
        case 't': timeout_ms = (uint32_t)strtoul(optarg, NULL, 10); break;
        case 'a': cfg.asset_dir = optarg; break;
        default:
            (void)fprintf(stderr, "usage: %s [-d display] [-w w] [-h h] [-z z] [-t timeout_ms] [-a assets]\n", argv[0]);
            return 2;
        }
    }
    (void)signal(SIGTERM, on_signal);
    (void)signal(SIGINT, on_signal);

    const char *root = getenv("CIVI_PPS_ROOT");
#ifdef CIVI_TARGET_QNX
    if (root == NULL) { root = "/pps/civi"; }
    char path[256];
    (void)snprintf(path, sizeof path, "%s/vs/telltales?wait,delta", root);
#else
    if (root == NULL) { root = "/tmp/civi-pps"; }
    char path[256];
    (void)snprintf(path, sizeof path, "%s/vs/telltales", root);
#endif

    if (tt_render_init(&cfg) != 0) {
        return 1;
    }
    tt_state_t st;
    tt_init(&st, now_ms());
    tt_render_draw(&st);   /* first frame: CHECK lit until the source is proven alive */

    char buf[4096];
#ifdef CIVI_TARGET_QNX
    int fd = -1;
#endif
    while (g_run) {
        bool changed = false;
#ifdef CIVI_TARGET_QNX
        if (fd < 0) {
            fd = open(path, O_RDONLY);
        }
        if (fd >= 0) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);
            struct timeval tv = { 0, 50 * 1000 };
            if (select(fd + 1, &rfds, NULL, NULL, &tv) > 0) {
                const ssize_t n = read(fd, buf, sizeof buf - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    changed = tt_apply_buffer(&st, buf, now_ms());
                } else {
                    (void)close(fd);
                    fd = -1;
                }
            }
        } else {
            sleep_ms(50);
        }
#else
        FILE *f = fopen(path, "r");
        if (f != NULL) {
            const size_t n = fread(buf, 1, sizeof buf - 1, f);
            (void)fclose(f);
            buf[n] = '\0';
            changed = tt_apply_buffer(&st, buf, now_ms());
        }
        sleep_ms(50);
#endif
        changed = tt_supervise(&st, now_ms(), timeout_ms) || changed;
        if (changed) {
            tt_render_draw(&st);
        }
    }
    tt_render_shutdown();
    return 0;
}
