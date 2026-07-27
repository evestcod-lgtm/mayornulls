#pragma once
#include "../il2cpp_resolver.h"

extern void (*BushAlphaOrig)(void* self, float alpha, void* method);
extern bool (*FogIsVisibleOrig)(void* self, Vec3 pos, void* method);

void BushAlphaHook(void* self, float alpha, void* method);
bool FogIsVisibleHook(void* self, Vec3 pos, void* method);

namespace Xray {
    void SetEnabled(bool val);
}
