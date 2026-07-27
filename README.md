# NullsMod — Null's Brawl Mod Menu

No root. GitHub Actions CI/CD build. Drop in APK, get patched APK out.

---

## Features

| Feature | What it does |
|---|---|
| ⚡ Inject | Status indicator — green when native hooks are live |
| 🎯 Aimbot | Locks aim to nearest enemy, auto-shoots in range |
| 🔄 Auto-Dodge | Predicts projectile trajectories, moves perp to dodge |
| 👁 X-Ray | Ghost bushes (0.12 alpha), reveals invisible Brawlers |
| 🥔 Potato GFX | Forces Quality=0, kills shadows + particles for max FPS |
| 🔓 FPS Unlock | Sets targetFrameRate=-1, fights game's reset every 500ms |
| 🔭 Max View Dist | Pushes Camera farClipPlane to 300 units |
| 💡 Enemy Ammo | Reads enemy ammo/maxAmmo every frame, displays in overlay |
| 🏆 Esports | FOV 70°, farClip 300, clean overlay mode |

---

## Quick Start

### Build via GitHub Actions

1. Fork this repo
2. Go to **Actions → Build NullsMod APK → Run workflow**
3. Paste your Null's Brawl APK URL into `apk_url` field
4. Wait ~10 minutes, download the artifact

### Build locally

```bash
# Prerequisites: JDK 17, Android SDK, NDK r25c, Python 3, apktool

# 1. Put Null's Brawl APK next to this README as base.apk
# 2. Decompile
apktool d base.apk -o decompiled --no-src -f

# 3. Build native library
cd app && ./gradlew assembleRelease

# 4. Copy .so into decompiled tree
cp app/build/intermediates/cmake/release/obj/arm64-v8a/libnullsmod.so \
   ../decompiled/lib/arm64-v8a/
cp app/build/intermediates/cmake/release/obj/armeabi-v7a/libnullsmod.so \
   ../decompiled/lib/armeabi-v7a/

# 5. Inject smali + patch manifest
cd ..
python3 patch/inject.py decompiled patch/smali_inject
python3 patch/patch_manifest.py decompiled/AndroidManifest.xml

# 6. Repack + sign
apktool b decompiled -o patched.apk --use-aapt2
zipalign -v 4 patched.apk aligned.apk
# Sign with your keystore (or use uber-apk-signer with debug key)
```

---

## ⚠️ Updating Offsets (REQUIRED for each Null's Brawl version)

The offsets in `il2cpp_resolver.h` are placeholders. You MUST update them
for your specific APK build or nothing will work.

### Finding offsets with IL2CppDumper

```bash
# 1. Extract libil2cpp.so and global-metadata.dat from APK
#    libil2cpp.so  → lib/arm64-v8a/libil2cpp.so
#    global-metadata.dat → assets/bin/Data/Managed/Metadata/global-metadata.dat

# 2. Run IL2CppDumper
./Il2CppDumper libil2cpp.so global-metadata.dat output/

# 3. Open output/script.json, search for method names:
#    "BrawlerComponent.Update"       → offset under "Address"
#    "AttackProjectile.Update"
#    "Application.set_targetFrameRate"
#    "Camera.set_farClipPlane"
#    "QualitySettings.set_shadowQuality"
#    "BushRenderer.SetAlpha"
#    "FogOfWar.IsVisible"
#    "EntityManager.GetList"
#    "PlayerInput.TriggerShoot"

# 4. Paste values into il2cpp_resolver.h → Offsets namespace
```

### Finding field offsets (Position, Ammo, etc.)

Use Frida + the dumped header (`il2cpp.h` from IL2CppDumper output):

```javascript
// Frida script — run while game is open
var BrawlerComponent = Il2Cpp.domain.assembly("Assembly-CSharp")
    .image.class("BrawlerComponent");

BrawlerComponent.fields.forEach(f => {
    console.log(f.name, "offset:", f.offset.toString(16));
});
```

Update `FieldOff::` constants in `aimbot.cpp` and `autododge.cpp` accordingly.

---

## Architecture

```
NullsMod
├── Native (C++, libnullsmod.so)
│   ├── main.cpp          JNI_OnLoad, JNI exports, ModState
│   ├── il2cpp_resolver   Base addr, inline ARM64 hook, r/w memory
│   ├── hooks.cpp         Installs all hooks on game methods
│   └── features/
│       ├── aimbot        BrawlerUpdate hook — aim + shoot + view dist
│       ├── autododge     ProjectileUpdate hook — trajectory + dodge
│       ├── xray          BushAlpha + FogIsVisible hooks
│       ├── graphics      QualitySet hook + direct potato calls
│       ├── esp           Ammo ring buffer (native → Java)
│       ├── fps_unlocker  Poll thread, targetFrameRate=-1
│       └── esports       FOV + farClip tweak
│
├── Java (overlay, no root)
│   ├── NativeBridge      JNI wrappers
│   ├── ModService        Foreground service, hosts overlay
│   └── ModMenuView       Draggable toggle menu, status dot
│
└── Patch pipeline
    ├── inject.py         Smali injection into host APK
    ├── patch_manifest.py Permission + service entry injection
    └── smali_inject/     Stub smali classes copied into APK
```

---

## Notes

- **No root needed** — the .so is injected directly into the APK, not a running process.
- **arm64-v8a + armeabi-v7a** both compiled; old devices covered.
- **Inline hook** is a basic ARM64 trampoline. For production: integrate Dobby
  (`add_subdirectory(Dobby)` in CMakeLists) for more reliable hooking.
- **Offsets** break every Null's Brawl update. Re-dump with IL2CppDumper each time.
- The mod menu is draggable. Tap the header to expand/collapse.
- Status dot is red until `isInjected()` returns true (hooks installed).
