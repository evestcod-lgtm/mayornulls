#include "esp.h"
#include "../il2cpp_resolver.h"
#include <pthread.h>
#include <cstring>

namespace AmmoESP {

Entry entries[MAX_ENTRIES] = {};
int   count = 0;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void RecordAmmo(void* entity, int ammo, int maxAmmo) {
    pthread_mutex_lock(&lock);

    // Update existing or add new
    for (int i = 0; i < count; ++i) {
        if (entries[i].entity == entity) {
            entries[i].ammo    = ammo;
            entries[i].maxAmmo = maxAmmo;
            pthread_mutex_unlock(&lock);
            return;
        }
    }

    if (count < MAX_ENTRIES) {
        entries[count++] = { entity, ammo, maxAmmo };
    }

    pthread_mutex_unlock(&lock);
}

void GetSnapshot(Entry* out, int* outCount) {
    pthread_mutex_lock(&lock);
    memcpy(out, entries, count * sizeof(Entry));
    *outCount = count;
    pthread_mutex_unlock(&lock);
}

} // namespace AmmoESP
