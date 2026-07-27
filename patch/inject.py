#!/usr/bin/env python3
"""
inject.py — Smali injection into decompiled Null's Brawl APK.

Usage:
    python3 inject.py <decompiled_dir> <smali_inject_dir>

What it does:
1. Finds the game's Application class (or MainActivity as fallback)
2. Injects a loadLibrary("nullsmod") + startService(ModService) call
   into onCreate()
3. Copies our smali files into smali/com/nullsmod/
"""

import os, sys, shutil, re

def find_application_smali(decomp_dir):
    """Find the main Application subclass in smali directories."""
    candidates = []
    for root, _, files in os.walk(decomp_dir):
        for f in files:
            if not f.endswith(".smali"):
                continue
            path = os.path.join(root, f)
            with open(path, "r", errors="replace") as fh:
                content = fh.read()
            # Application subclass has .super Landroid/app/Application;
            if ".super Landroid/app/Application;" in content and "onCreate" in content:
                candidates.append(path)

    if not candidates:
        # Fallback: find UnityPlayerActivity
        for root, _, files in os.walk(decomp_dir):
            for f in files:
                if "UnityPlayerActivity" in f or "MainActivity" in f:
                    candidates.append(os.path.join(root, f))

    return candidates[0] if candidates else None


def inject_into_smali(smali_path):
    """Inject our hook call into the first .method public onCreate."""
    with open(smali_path, "r", errors="replace") as fh:
        content = fh.read()

    # Our injected code:
    # 1. Load libnullsmod.so
    # 2. Start ModService (if SYSTEM_ALERT_WINDOW granted)
    injection = """
    # ─── NullsMod injection ───────────────────────────────────────────
    const-string v0, "nullsmod"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    new-instance v0, Landroid/content/Intent;
    const-class v1, Lcom/nullsmod/ModService;
    invoke-direct {v0, p0, v1}, Landroid/content/Intent;-><init>(Landroid/content/Context;Ljava/lang/Class;)V
    invoke-virtual {p0, v0}, Landroid/app/Application;->startService(Landroid/content/Intent;)Landroid/content/ComponentName;
    # ─── End NullsMod injection ──────────────────────────────────────
"""

    # Find .method public onCreate(Landroid/os/Bundle;)V or similar
    # and inject right after the first invoke-super call
    pattern = r'(\.method public onCreate\([^)]*\)V.*?invoke-super \{[^}]+\},[^\n]+\n)'

    def replacer(m):
        return m.group(1) + injection

    new_content, count = re.subn(pattern, replacer, content, count=1, flags=re.DOTALL)

    if count == 0:
        # Fallback: inject into any onCreate
        new_content = content.replace(
            ".method public onCreate()V\n",
            ".method public onCreate()V\n" + injection
        )

    with open(smali_path, "w") as fh:
        fh.write(new_content)

    print(f"[inject.py] Injected into: {smali_path}")


def copy_smali_files(decomp_dir, smali_inject_dir):
    """Copy our smali class files into the decompiled tree."""
    # Find an existing smali dir (smali, smali_classes2, etc.)
    smali_root = None
    for d in ["smali_classes3", "smali_classes2", "smali"]:
        candidate = os.path.join(decomp_dir, d)
        if os.path.isdir(candidate):
            smali_root = candidate
            break

    if not smali_root:
        smali_root = os.path.join(decomp_dir, "smali")
        os.makedirs(smali_root, exist_ok=True)

    dest = os.path.join(smali_root, "com", "nullsmod")
    os.makedirs(dest, exist_ok=True)

    for f in os.listdir(smali_inject_dir):
        if f.endswith(".smali"):
            src = os.path.join(smali_inject_dir, f)
            shutil.copy2(src, os.path.join(dest, f))
            print(f"[inject.py] Copied smali: {f}")


def main():
    if len(sys.argv) < 3:
        print("Usage: inject.py <decompiled_dir> <smali_inject_dir>")
        sys.exit(1)

    decomp_dir       = sys.argv[1]
    smali_inject_dir = sys.argv[2]

    print(f"[inject.py] Target: {decomp_dir}")

    # 1. Copy our smali classes
    copy_smali_files(decomp_dir, smali_inject_dir)

    # 2. Find and patch Application class
    app_smali = find_application_smali(decomp_dir)
    if app_smali:
        inject_into_smali(app_smali)
    else:
        print("[inject.py] WARNING: Could not find Application class. Manual injection needed.")

    print("[inject.py] Done.")


if __name__ == "__main__":
    main()
