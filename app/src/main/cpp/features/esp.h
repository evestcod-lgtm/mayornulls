#pragma once

namespace AmmoESP {
    struct Entry { void* entity; int ammo; int maxAmmo; };
    constexpr int MAX_ENTRIES = 32;
    extern Entry entries[MAX_ENTRIES];
    extern int   count;

    void RecordAmmo(void* entity, int ammo, int maxAmmo);
    void GetSnapshot(Entry* out, int* outCount);
}
