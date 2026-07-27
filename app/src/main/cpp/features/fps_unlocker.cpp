#include "fps_unlocker.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

#define TAG "NullsMod/FPS"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

static bool s_enabled      = false;
static bool s_threadActive = false;

// ── Poll loop: keep forcing targetFrameRate = -1 every 500ms ─────────────────
// Brawl Stars periodically resets this; we fight back.
static void* fps_poll(void*) {
    typedef void (*SetFPS_t)(int, void*);
    uintptr_t fn = IL2CPP::Resolve(Offsets::Application_set_targetFPS);
    if (!fn) return nullptr;

    while (s_threadActive) {
        if (s_enabled) {
            ((SetFPS_t)fn)(-1, nullptr); // -1 = unlimited
        }
        usleep(500000); // 500ms
    }
    return nullptr;
}

namespace FpsUnlocker {

void Install() {
    s_threadActive = true;
    pthread_t t;
    pthread_create(&t, nullptr, fps_poll, nullptr);
    pthread_detach(t);
    LOGI("FPS unlocker thread started");
}

void SetEnabled(bool val) {
    s_enabled = val;
    if (!val) {
        // Reset to 30fps (Brawl Stars default)
        typedef void (*SetFPS_t)(int, void*);
        uintptr_t fn = IL2CPP::Resolve(Offsets::Application_set_targetFPS);
        if (fn) ((SetFPS_t)fn)(30, nullptr);
    }
    LOGI("FPS unlocker: %s", val ? "ON" : "OFF");
}

} // namespace FpsUnlocker
