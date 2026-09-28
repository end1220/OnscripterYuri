/* -*- C++ -*-
 *
 *  sys_volume.cpp - System volume overlay for Linux handhelds
 *
 *  Anbernic stock firmware keeps the codec at full scale ("digital volume"
 *  is locked to 63 by /etc/asound.conf) and stores the system volume in
 *  /sys/class/power_supply/axp2202-battery/openbor_volume (0-10); every app
 *  is expected to scale its own PCM by that value.
 */

#include "sys_volume.h"
#include "Utils.h"

#if defined(ANDROID)
#include "SDL_mixer.h"
#else
#include <SDL2/SDL_mixer.h>
#endif

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#if defined(__linux__) && !defined(ANDROID) && !defined(__LIBRETRO__)
#define SYS_VOLUME_ENABLED 1
#include <dirent.h>
#endif

namespace sys_volume {

namespace {

enum class Backend {
    Passthrough,
    Sysfs,
    Software,
};

std::atomic<int> g_percent{100};
bool g_inited = false;
Backend g_backend = Backend::Passthrough;
bool g_owns_keys = false;
std::string g_sysfs_path;
int g_sysfs_max = 10;
Uint16 g_mix_format = 0;
int g_poll_counter = 0;

int clampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

const char *backendStr(Backend b)
{
    switch (b) {
    case Backend::Sysfs: return "sysfs";
    case Backend::Software: return "software";
    default: return "passthrough";
    }
}

#if defined(SYS_VOLUME_ENABLED)

bool fileReadable(const char *path)
{
    if (!path || !path[0]) return false;
    FILE *f = fopen(path, "r");
    if (!f) return false;
    fclose(f);
    return true;
}

bool sdlDriverIsPulse()
{
    const char *drv = getenv("SDL_AUDIODRIVER");
    return drv && (!strcmp(drv, "pulse") || !strcmp(drv, "pipewire"));
}

std::string probeSysfsPath()
{
    static const char *kKnown[] = {
        "/sys/class/power_supply/axp2202-battery/openbor_volume",
        "/sys/class/power_supply/axp20x-battery/openbor_volume",
    };
    for (const char *p : kKnown) {
        if (fileReadable(p)) return p;
    }
    DIR *dir = opendir("/sys/class/power_supply");
    if (!dir) return std::string();
    std::string found;
    while (dirent *ent = readdir(dir)) {
        if (ent->d_name[0] == '.') continue;
        std::string p = std::string("/sys/class/power_supply/") + ent->d_name + "/openbor_volume";
        if (fileReadable(p.c_str())) {
            found = p;
            break;
        }
    }
    closedir(dir);
    return found;
}

int readSysfsLevel()
{
    int v = g_sysfs_max;
    FILE *f = fopen(g_sysfs_path.c_str(), "r");
    if (f) {
        if (fscanf(f, "%d", &v) != 1) v = g_sysfs_max;
        fclose(f);
    }
    return clampInt(v, 0, g_sysfs_max);
}

void storeFromSysfs()
{
    int v = readSysfsLevel();
    int pct = (g_sysfs_max > 0) ? (v * 100 / g_sysfs_max) : 100;
    g_percent.store(clampInt(pct, 0, 100), std::memory_order_relaxed);
}

#endif // SYS_VOLUME_ENABLED

void init()
{
    if (g_inited) return;
    g_inited = true;

#if defined(SYS_VOLUME_ENABLED)
    std::string want = "auto";
    std::string sysfs;
    int sysfs_max = 10;

    if (const char *e = getenv("ONS_SYS_VOLUME_BACKEND")) {
        if (e[0]) want = e;
    }
    if (const char *e = getenv("ONS_SYS_VOLUME_PATH")) {
        if (e[0]) sysfs = e;
    }
    if (const char *e = getenv("ONS_SYS_VOLUME_MAX")) {
        int n = atoi(e);
        if (n > 0) sysfs_max = n;
    }
    g_sysfs_max = sysfs_max;

    Backend b = Backend::Passthrough;
    if (want == "sysfs") {
        if (sysfs.empty()) sysfs = probeSysfsPath();
        b = sysfs.empty() ? Backend::Passthrough : Backend::Sysfs;
    } else if (want == "software") {
        b = Backend::Software;
    } else if (want == "passthrough" || want == "pulse") {
        b = Backend::Passthrough;
    } else {
        // A leftover pulse socket must not hide sysfs volume when SDL is on ALSA.
        if (sysfs.empty()) sysfs = probeSysfsPath();
        if (sdlDriverIsPulse())
            b = Backend::Passthrough;
        else if (!sysfs.empty())
            b = Backend::Sysfs;
    }

    if (b == Backend::Sysfs) {
        g_sysfs_path = sysfs;
        if (!fileReadable(g_sysfs_path.c_str())) {
            utils::printInfo("[SysVolume] %s not readable, fallback to passthrough\n",
                             g_sysfs_path.c_str());
            b = Backend::Passthrough;
            g_sysfs_path.clear();
        }
    }

    g_backend = b;
    g_owns_keys = (b == Backend::Sysfs || b == Backend::Software);
    if (b == Backend::Sysfs)
        storeFromSysfs();
    else
        g_percent.store(100, std::memory_order_relaxed);

    if (b == Backend::Sysfs)
        utils::printInfo("[SysVolume] backend=sysfs path=%s max=%d percent=%d keys=%d\n",
                         g_sysfs_path.c_str(), g_sysfs_max,
                         g_percent.load(std::memory_order_relaxed), (int)g_owns_keys);
    else
        utils::printInfo("[SysVolume] backend=%s percent=%d keys=%d\n", backendStr(b),
                         g_percent.load(std::memory_order_relaxed), (int)g_owns_keys);
#endif
}

void adjust(int delta)
{
#if defined(SYS_VOLUME_ENABLED)
    if (g_backend == Backend::Sysfs) {
        int v = clampInt(readSysfsLevel() + delta, 0, g_sysfs_max);
        FILE *f = fopen(g_sysfs_path.c_str(), "w");
        if (f) {
            fprintf(f, "%d\n", v);
            fclose(f);
        }
        int pct = (g_sysfs_max > 0) ? (v * 100 / g_sysfs_max) : 100;
        g_percent.store(clampInt(pct, 0, 100), std::memory_order_relaxed);
        utils::printInfo("[SysVolume] system volume %d/%d (%d%%)\n", v, g_sysfs_max, pct);
        return;
    }
    if (g_backend == Backend::Software) {
        int step = (g_sysfs_max > 0) ? (100 / g_sysfs_max) : 10;
        if (step < 1) step = 10;
        int p = clampInt(g_percent.load(std::memory_order_relaxed) + delta * step, 0, 100);
        g_percent.store(p, std::memory_order_relaxed);
        utils::printInfo("[SysVolume] software volume %d%%\n", p);
    }
#else
    (void)delta;
#endif
}

void postMix(void *, Uint8 *stream, int len)
{
#if defined(SYS_VOLUME_ENABLED)
    // ~16 buffers between sysfs reads keeps the OSD / other writers in sync
    // without touching the file on every audio callback.
    if (g_backend == Backend::Sysfs && (++g_poll_counter & 15) == 0)
        storeFromSysfs();
#endif
    int pct = g_percent.load(std::memory_order_relaxed);
    if (pct >= 100 || len <= 0) return;

    if (g_mix_format == AUDIO_S16SYS) {
        Sint16 *s = reinterpret_cast<Sint16 *>(stream);
        int n = len / (int)sizeof(Sint16);
        for (int i = 0; i < n; ++i)
            s[i] = (Sint16)((int)s[i] * pct / 100);
    } else if (g_mix_format == AUDIO_F32SYS) {
        float *s = reinterpret_cast<float *>(stream);
        int n = len / (int)sizeof(float);
        float g = pct / 100.0f;
        for (int i = 0; i < n; ++i)
            s[i] *= g;
    }
}

} // namespace

void attachMixer()
{
    init();
    if (g_backend == Backend::Passthrough) {
        Mix_SetPostMix(NULL, NULL);
        return;
    }
    int freq = 0, channels = 0;
    Uint16 format = 0;
    if (!Mix_QuerySpec(&freq, &format, &channels)) return;
    g_mix_format = format;
    if (format != AUDIO_S16SYS && format != AUDIO_F32SYS) {
        utils::printInfo("[SysVolume] unsupported mix format 0x%x, gain disabled\n", format);
        Mix_SetPostMix(NULL, NULL);
        return;
    }
    Mix_SetPostMix(postMix, NULL);
}

bool handleKey(const SDL_KeyboardEvent &key)
{
    if (key.keysym.scancode != SDL_SCANCODE_VOLUMEUP &&
        key.keysym.scancode != SDL_SCANCODE_VOLUMEDOWN)
        return false;
    init();
    if (!g_owns_keys) return false;
    if (key.type == SDL_KEYDOWN && !key.repeat)
        adjust(key.keysym.scancode == SDL_SCANCODE_VOLUMEUP ? +1 : -1);
    return true;
}

int percent()
{
    init();
    return g_percent.load(std::memory_order_relaxed);
}

const char *backendName()
{
    init();
    return backendStr(g_backend);
}

}
