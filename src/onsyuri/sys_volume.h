/* -*- C++ -*-
 *
 *  sys_volume.h - System volume overlay for Linux handhelds
 *
 *  Auto backend (default):
 *    - SDL_AUDIODRIVER=pulse/pipewire: pass-through (the mixer already applies
 *      system volume; do not software-scale, do not steal VOLUMEUP/DOWN).
 *    - Known sysfs volume file (Anbernic openbor_volume and clones): the codec
 *      is locked at full scale, so apply software gain from that file, and
 *      intercept volume keys to write it back.
 *    - Otherwise: pass-through.
 *
 *  Override via env ONS_SYS_VOLUME_BACKEND (auto|sysfs|software|passthrough),
 *  ONS_SYS_VOLUME_PATH, ONS_SYS_VOLUME_MAX.
 */

#ifndef __SYS_VOLUME_H__
#define __SYS_VOLUME_H__

#if defined(ANDROID)
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

namespace sys_volume {

// Call after every successful Mix_OpenAudio; installs the post-mix gain when needed.
void attachMixer();

// Returns true when the key is a volume key consumed by the overlay.
bool handleKey(const SDL_KeyboardEvent &key);

int percent();
const char *backendName();

}

#endif // __SYS_VOLUME_H__
