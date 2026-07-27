#pragma once

extern void (*QualitySetOrig)(int level, bool applyExpensive, void* method);
void QualitySetHook(int level, bool applyExpensive, void* method);

namespace Graphics {
    void SetPotato(bool enabled);
}
