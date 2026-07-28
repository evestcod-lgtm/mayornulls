#pragma once
#include "../il2cpp_resolver.h"
#include "esp.h"

// Trampoline — defined in aimbot.cpp, declared extern in hooks.cpp
extern void (*BrawlerUpdateOrig)(void* self, void* method);
void BrawlerUpdateHook(void* self, void* method);
