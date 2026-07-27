package com.nullsmod;

import android.animation.*;
import android.content.Context;
import android.graphics.*;
import android.os.*;
import android.view.*;
import android.widget.*;
import java.util.*;

/**
 * Self-contained draggable mod menu drawn as a View.
 * No XML layout needed — everything built in code so injection works
 * without needing res/ from the host APK.
 */
public class ModMenuView extends LinearLayout {

    // ── Feature toggle state ──────────────────────────────────────────────────
    private final boolean[] state = new boolean[Feature.values().length];
    private boolean menuOpen  = false;
    private boolean injected  = false;

    // ── Drag support ──────────────────────────────────────────────────────────
    private float touchX, touchY;
    private int   rawX,  rawY;
    private WindowManager.LayoutParams mParams;
    private WindowManager              mWm;

    // ── Colors ────────────────────────────────────────────────────────────────
    private static final int BG_COLOR    = Color.argb(210, 10, 10, 18);
    private static final int ACCENT      = Color.argb(255, 80, 200, 255);
    private static final int GREEN       = Color.argb(255, 50, 220, 100);
    private static final int RED         = Color.argb(255, 220, 60, 60);
    private static final int TEXT_COLOR  = Color.WHITE;

    enum Feature {
        INJECT("⚡ Inject"),
        AIMBOT("🎯 Aimbot"),
        AUTO_DODGE("🔄 Auto-Dodge"),
        XRAY("👁 X-Ray"),
        POTATO_GFX("🥔 Potato GFX"),
        FPS_UNLOCK("🔓 FPS Unlock"),
        MAX_VIEW("🔭 Max View Dist"),
        AMMO_ESP("💡 Enemy Ammo"),
        ESPORTS("🏆 Esports");

        final String label;
        Feature(String l) { this.label = l; }
    }

    // ── UI children ──────────────────────────────────────────────────────────
    private TextView     mStatusDot;
    private LinearLayout mToggleContainer;
    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private final List<ToggleRow> mRows = new ArrayList<>();

    public ModMenuView(Context ctx) {
        super(ctx);
        setOrientation(VERTICAL);
        setBackgroundColor(BG_COLOR);
        setPadding(dp(8), dp(8), dp(8), dp(8));

        buildHeader(ctx);
        buildToggleContainer(ctx);
        setupDrag();
        startInjectionCheck();
    }

    // ── Header bar ────────────────────────────────────────────────────────────
    private void buildHeader(Context ctx) {
        LinearLayout header = new LinearLayout(ctx);
        header.setOrientation(HORIZONTAL);
        header.setGravity(android.view.Gravity.CENTER_VERTICAL);
        header.setPadding(dp(4), dp(4), dp(4), dp(4));

        mStatusDot = new TextView(ctx);
        mStatusDot.setText("●");
        mStatusDot.setTextColor(RED);
        mStatusDot.setTextSize(14);
        mStatusDot.setPadding(0, 0, dp(6), 0);

        TextView title = new TextView(ctx);
        title.setText("NullsMod");
        title.setTextColor(ACCENT);
        title.setTextSize(13);
        title.setTypeface(Typeface.DEFAULT_BOLD);

        TextView arrow = new TextView(ctx);
        arrow.setText(menuOpen ? "▲" : "▼");
        arrow.setTextColor(TEXT_COLOR);
        arrow.setTextSize(11);
        arrow.setLayoutParams(new LinearLayout.LayoutParams(0, LayoutParams.WRAP_CONTENT, 1));
        arrow.setGravity(android.view.Gravity.END);

        header.addView(mStatusDot);
        header.addView(title);
        header.addView(arrow);

        header.setOnClickListener(v -> {
            menuOpen = !menuOpen;
            arrow.setText(menuOpen ? "▲" : "▼");
            mToggleContainer.setVisibility(menuOpen ? VISIBLE : GONE);
            animateExpand(menuOpen);
        });

        addView(header);

        // Divider
        View div = new View(ctx);
        div.setBackgroundColor(Color.argb(60, 255, 255, 255));
        div.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT, 1));
        addView(div);
    }

    // ── Toggle rows ───────────────────────────────────────────────────────────
    private void buildToggleContainer(Context ctx) {
        mToggleContainer = new LinearLayout(ctx);
        mToggleContainer.setOrientation(VERTICAL);
        mToggleContainer.setVisibility(GONE);

        for (Feature f : Feature.values()) {
            ToggleRow row = new ToggleRow(ctx, f);
            mToggleContainer.addView(row);
            mRows.add(row);
        }
        addView(mToggleContainer);
    }

    // ── Inner toggle row ──────────────────────────────────────────────────────
    private class ToggleRow extends LinearLayout {
        private final Feature  feature;
        private final TextView indicator;

        ToggleRow(Context ctx, Feature f) {
            super(ctx);
            this.feature = f;
            setOrientation(HORIZONTAL);
            setGravity(android.view.Gravity.CENTER_VERTICAL);
            setPadding(dp(2), dp(5), dp(2), dp(5));

            // Bullet indicator
            indicator = new TextView(ctx);
            indicator.setText("●");
            indicator.setTextColor(Color.GRAY);
            indicator.setTextSize(10);
            indicator.setPadding(0, 0, dp(6), 0);

            // Label
            TextView label = new TextView(ctx);
            label.setText(f.label);
            label.setTextColor(TEXT_COLOR);
            label.setTextSize(12);
            label.setLayoutParams(new LayoutParams(0, LayoutParams.WRAP_CONTENT, 1));

            // Toggle switch look-alike
            TextView sw = new TextView(ctx);
            sw.setText("OFF");
            sw.setTextColor(Color.GRAY);
            sw.setTextSize(10);
            sw.setBackgroundColor(Color.argb(80, 100, 100, 100));
            sw.setPadding(dp(6), dp(2), dp(6), dp(2));

            addView(indicator);
            addView(label);
            addView(sw);

            setOnClickListener(v -> {
                boolean next = !state[f.ordinal()];
                state[f.ordinal()] = next;

                // Update visuals
                indicator.setTextColor(next ? GREEN : Color.GRAY);
                sw.setText(next ? "ON" : "OFF");
                sw.setTextColor(next ? GREEN : Color.GRAY);
                sw.setBackgroundColor(next
                    ? Color.argb(80, 50, 220, 100)
                    : Color.argb(80, 100, 100, 100));

                // Fire native
                dispatchFeature(f, next);
            });
        }

        void setInjectedStyle() {
            if (feature == Feature.INJECT) {
                state[Feature.INJECT.ordinal()] = true;
                indicator.setTextColor(GREEN);
            }
        }
    }

    // ── Dispatch toggle to native bridge ─────────────────────────────────────
    private void dispatchFeature(Feature f, boolean val) {
        switch (f) {
            case INJECT:      /* handled by injection itself */   break;
            case AIMBOT:      NativeBridge.setAimbot(val);        break;
            case AUTO_DODGE:  NativeBridge.setAutoDodge(val);     break;
            case XRAY:        NativeBridge.setXray(val);           break;
            case POTATO_GFX:  NativeBridge.setPotatoGfx(val);    break;
            case FPS_UNLOCK:  NativeBridge.setFpsUnlock(val);     break;
            case MAX_VIEW:    NativeBridge.setMaxViewDist(val);   break;
            case AMMO_ESP:    NativeBridge.setAmmoDisplay(val);   break;
            case ESPORTS:     NativeBridge.setEsports(val);       break;
        }
    }

    // ── Poll native injection status ──────────────────────────────────────────
    private void startInjectionCheck() {
        mHandler.postDelayed(new Runnable() {
            @Override public void run() {
                boolean now = NativeBridge.isInjected();
                if (now != injected) {
                    injected = now;
                    mStatusDot.setTextColor(injected ? GREEN : RED);
                    if (injected) {
                        for (ToggleRow r : mRows) r.setInjectedStyle();
                    }
                }
                mHandler.postDelayed(this, injected ? 5000 : 500);
            }
        }, 500);
    }

    // ── Drag ─────────────────────────────────────────────────────────────────
    private void setupDrag() {
        setOnTouchListener((v, e) -> {
            switch (e.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    touchX = e.getRawX(); touchY = e.getRawY();
                    rawX   = mParams != null ? mParams.x : 16;
                    rawY   = mParams != null ? mParams.y : 120;
                    return true;
                case MotionEvent.ACTION_MOVE:
                    if (mParams != null && mWm != null) {
                        mParams.x = rawX + (int)(e.getRawX() - touchX);
                        mParams.y = rawY + (int)(e.getRawY() - touchY);
                        mWm.updateViewLayout(this, mParams);
                    }
                    return true;
            }
            return false;
        });
    }

    public void setWindowManager(WindowManager wm, WindowManager.LayoutParams p) {
        mWm     = wm;
        mParams = p;
    }

    // ── Expand animation ──────────────────────────────────────────────────────
    private void animateExpand(boolean expand) {
        float from = expand ? 0f : 1f, to = expand ? 1f : 0f;
        ValueAnimator anim = ValueAnimator.ofFloat(from, to);
        anim.setDuration(150);
        anim.addUpdateListener(a -> {
            float v = (float) a.getAnimatedValue();
            mToggleContainer.setAlpha(v);
            mToggleContainer.setScaleY(v);
        });
        anim.start();
    }

    // ── dp helper ────────────────────────────────────────────────────────────
    private int dp(int v) {
        return (int)(v * getResources().getDisplayMetrics().density);
    }
}
