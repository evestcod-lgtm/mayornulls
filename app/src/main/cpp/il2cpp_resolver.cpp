#include "il2cpp_resolver.h"
#include <dlfcn.h>
#include <android/log.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>

#define TAG "NullsMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static uintptr_t s_libBase = 0;

// ── Find libil2cpp.so base address from /proc/self/maps ──────────────────────
uintptr_t IL2CPP::GetLibBase() {
    if (s_libBase) return s_libBase;

    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find("libil2cpp.so") != std::string::npos && line.find("r-xp") != std::string::npos) {
            uintptr_t start = std::stoull(line, nullptr, 16);
            s_libBase = start;
            LOGI("libil2cpp.so base: 0x%lx", (unsigned long)start);
            return start;
        }
    }
    LOGE("libil2cpp.so not found in maps");
    return 0;
}

// ── Resolve an absolute address from an offset ───────────────────────────────
uintptr_t IL2CPP::Resolve(uintptr_t offset) {
    uintptr_t base = GetLibBase();
    if (!base) return 0;
    return base + offset;
}

// ── Inline hook (ARM64): patch 4 bytes at target with B instruction ──────────
// For production, swap this with Dobby or LSPlant.
bool IL2CPP::Hook(uintptr_t target, void* replacement, void** original) {
    if (!target || !replacement) return false;

    // Make page writable
    uintptr_t page  = target & ~(uintptr_t)(getpagesize() - 1);
    if (mprotect((void*)page, getpagesize() * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("mprotect failed at 0x%lx", (unsigned long)target);
        return false;
    }

    // Save original 16 bytes for trampoline
    if (original) {
        static uint8_t trampolines[64][32];
        static int trampolineIdx = 0;
        uint8_t* tramp = trampolines[trampolineIdx++ % 64];
        memcpy(tramp, (void*)target, 16);

        // Append: LDR X17, #8; BR X17; <original_addr>
        uint32_t* t = (uint32_t*)tramp + 4;
        t[0] = 0x58000051; // LDR X17, #8
        t[1] = 0xD61F0220; // BR X17
        uintptr_t origAddr = target + 16;
        memcpy(&t[2], &origAddr, 8);

        mprotect((void*)((uintptr_t)tramp & ~(uintptr_t)(getpagesize()-1)),
                 getpagesize(), PROT_READ | PROT_EXEC);
        *original = (void*)tramp;
    }

    // Write ARM64 absolute jump: LDR X17, #8; BR X17; <dest>
    uint32_t* patch = (uint32_t*)target;
    patch[0] = 0x58000051; // LDR X17, #8
    patch[1] = 0xD61F0220; // BR X17
    uintptr_t dest = (uintptr_t)replacement;
    memcpy(&patch[2], &dest, 8);

    // Flush icache
    __builtin___clear_cache((char*)target, (char*)(target + 16));

    LOGI("Hooked 0x%lx -> %p", (unsigned long)target, replacement);
    return true;
}

// ── Read/write process memory safely ─────────────────────────────────────────
bool IL2CPP::ReadMemory(uintptr_t addr, void* buf, size_t size) {
    if (!addr || !buf) return false;
    memcpy(buf, (void*)addr, size);
    return true;
}

bool IL2CPP::WriteMemory(uintptr_t addr, const void* buf, size_t size) {
    if (!addr || !buf) return false;
    uintptr_t page = addr & ~(uintptr_t)(getpagesize() - 1);
    mprotect((void*)page, getpagesize() * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy((void*)addr, buf, size);
    __builtin___clear_cache((char*)addr, (char*)(addr + size));
    return true;
}
