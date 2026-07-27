#!/usr/bin/env python3
"""
patch_manifest.py — Adds overlay + foreground service permissions to host APK manifest.

Usage:
    python3 patch_manifest.py <AndroidManifest.xml>
"""

import sys, re

PERMISSIONS = [
    'android.permission.SYSTEM_ALERT_WINDOW',
    'android.permission.FOREGROUND_SERVICE',
    'android.permission.FOREGROUND_SERVICE_SPECIAL_USE',
    'android.permission.VIBRATE',
]

SERVICE_ENTRY = '''
    <service
        android:name="com.nullsmod.ModService"
        android:exported="false"
        android:foregroundServiceType="specialUse">
        <property
            android:name="android.app.PROPERTY_SPECIAL_USE_FGS_SUBTYPE"
            android:value="Mod menu overlay" />
    </service>'''

def main():
    if len(sys.argv) < 2:
        print("Usage: patch_manifest.py <AndroidManifest.xml>")
        sys.exit(1)

    path = sys.argv[1]
    with open(path, "r", errors="replace") as f:
        content = f.read()

    # Add permissions if missing
    for perm in PERMISSIONS:
        tag = f'<uses-permission android:name="{perm}" />'
        if perm not in content:
            content = content.replace(
                '<application',
                tag + '\n    <application',
                1
            )
            print(f"[manifest] Added permission: {perm}")

    # Inject service entry before </application>
    if "com.nullsmod.ModService" not in content:
        content = content.replace(
            '</application>',
            SERVICE_ENTRY + '\n</application>',
            1
        )
        print("[manifest] Injected ModService entry")

    with open(path, "w") as f:
        f.write(content)

    print("[manifest] Done.")

if __name__ == "__main__":
    main()
