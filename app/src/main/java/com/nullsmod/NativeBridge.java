package com.nullsmod;

/**
 * JNI bridge to libnullsmod.so
 * All methods are static; the native lib is loaded once by ModService.
 */
public final class NativeBridge {

    static {
        try {
            System.loadLibrary("nullsmod");
        } catch (UnsatisfiedLinkError e) {
            android.util.Log.e("NullsMod", "Failed to load libnullsmod.so: " + e.getMessage());
        }
    }

    public static native void setAimbot(boolean val);
    public static native void setAutoDodge(boolean val);
    public static native void setXray(boolean val);
    public static native void setPotatoGfx(boolean val);
    public static native void setFpsUnlock(boolean val);
    public static native void setMaxViewDist(boolean val);
    public static native void setAmmoDisplay(boolean val);
    public static native void setEsports(boolean val);
    public static native boolean isInjected();
}
