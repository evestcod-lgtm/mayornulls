package com.nullsmod;

import android.app.*;
import android.content.*;
import android.graphics.*;
import android.os.*;
import android.view.*;
import android.view.accessibility.*;
import android.widget.*;
import androidx.core.app.NotificationCompat;

/**
 * Foreground service that draws the mod menu overlay.
 * Started by the smali hook injected into Brawl's Application.onCreate.
 */
public class ModService extends Service {

    private static final String CHANNEL_ID = "nullsmod_channel";
    private WindowManager       mWindowManager;
    private ModMenuView         mMenuView;

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        startForeground(1, buildNotification());

        mWindowManager = (WindowManager) getSystemService(WINDOW_SERVICE);
        mMenuView      = new ModMenuView(this);

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
            WindowManager.LayoutParams.WRAP_CONTENT,
            WindowManager.LayoutParams.WRAP_CONTENT,
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                ? WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
                : WindowManager.LayoutParams.TYPE_PHONE,
            WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN,
            PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 16;
        params.y = 120;

        mWindowManager.addView(mMenuView, params);
    }

    @Override
    public void onDestroy() {
        if (mMenuView != null) mWindowManager.removeView(mMenuView);
        super.onDestroy();
    }

    @Override public IBinder onBind(Intent i) { return null; }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel ch = new NotificationChannel(
                CHANNEL_ID, "NullsMod", NotificationManager.IMPORTANCE_MIN);
            getSystemService(NotificationManager.class).createNotificationChannel(ch);
        }
    }

    private Notification buildNotification() {
        return new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("NullsMod Active")
            .setContentText("Tap to open mod menu")
            .setPriority(NotificationCompat.PRIORITY_MIN)
            .setSmallIcon(android.R.drawable.ic_menu_manage)
            .build();
    }
}
