#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include "il2cpp_resolver.h"
#include "hooks.h"
#include "features/aimbot.h"
#include "features/autododge.h"
#include "features/xray.h"
#include "features/graphics.h"
#include "features/esp.h"
#include "features/fps_unlocker.h"
#include "features/esports.h"

#define TAG "NullsMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ── Feature toggle state (written by Java mod menu via JNI) ──────────────────
ModState g_state = {
    .aimbot       = false,
    .autoDodge    = false,
    .xray         = false,
    .potatoGfx    = false,
    .fpsUnlock    = false,
    .maxViewDist  = false,
    .ammoDisplay  = false,
    .esports      = false,
    .injected     = false,
};

// ── Init thread: waits for libil2cpp.so to load then installs hooks ──────────
static void* init_thread(void*) {
    LOGI("Init thread started");

    // Wait for game lib to appear (up to 30 seconds)
    for (int i = 0; i < 300; ++i) {
        if (IL2CPP::GetLibBase() != 0) break;
        usleep(100000); // 100ms
    }

    if (!IL2CPP::GetLibBase()) {
        LOGE("Timed out waiting for libil2cpp.so");
        return nullptr;
    }

    LOGI("libil2cpp.so found, installing hooks...");

    Hooks::Install();
    FpsUnlocker::Install();

    g_state.injected = true;
    LOGI("All hooks installed. NullsMod active.");
    return nullptr;
}

// ── JNI_OnLoad: entry point when .so is loaded ───────────────────────────────
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("NullsMod JNI_OnLoad");

    pthread_t thread;
    pthread_create(&thread, nullptr, init_thread, nullptr);
    pthread_detach(thread);

    return JNI_VERSION_1_6;
}

// ── JNI exports: called from Java ModMenu ────────────────────────────────────
extern "C" {

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setAimbot(JNIEnv*, jclass, jboolean val) {
    g_state.aimbot = val;
    LOGI("Aimbot: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setAutoDodge(JNIEnv*, jclass, jboolean val) {
    g_state.autoDodge = val;
    LOGI("AutoDodge: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setXray(JNIEnv*, jclass, jboolean val) {
    g_state.xray = val;
    Xray::SetEnabled(val);
    LOGI("Xray: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setPotatoGfx(JNIEnv*, jclass, jboolean val) {
    g_state.potatoGfx = val;
    Graphics::SetPotato(val);
    LOGI("PotatoGfx: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setFpsUnlock(JNIEnv*, jclass, jboolean val) {
    g_state.fpsUnlock = val;
    FpsUnlocker::SetEnabled(val);
    LOGI("FpsUnlock: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setMaxViewDist(JNIEnv*, jclass, jboolean val) {
    g_state.maxViewDist = val;
    LOGI("MaxViewDist: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setAmmoDisplay(JNIEnv*, jclass, jboolean val) {
    g_state.ammoDisplay = val;
    LOGI("AmmoDisplay: %s", val ? "ON" : "OFF");
}

JNIEXPORT void JNICALL
Java_com_nullsmod_NativeBridge_setEsports(JNIEnv*, jclass, jboolean val) {
    g_state.esports = val;
    Esports::SetEnabled(val);
    LOGI("Esports: %s", val ? "ON" : "OFF");
}

JNIEXPORT jboolean JNICALL
Java_com_nullsmod_NativeBridge_isInjected(JNIEnv*, jclass) {
    return (jboolean)g_state.injected;
}

} // extern "C"
