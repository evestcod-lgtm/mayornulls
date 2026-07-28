#pragma once
#include "../il2cpp_resolver.h"

// QualitySettings hook
extern void (*QualitySetOrig)(int level, bool applyExpensive, void* method);
void QualitySetHook(int level, bool applyExpensive, void* method);

// Camera farClipPlane hook — declared here, defined in graphics.cpp
extern void (*CameraFarClipOrig)(float val, void* method);
void CameraFarClipHook(float val, void* method);

namespace Graphics {
    void SetPotato(bool enabled);
}
