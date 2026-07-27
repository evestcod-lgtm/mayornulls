# Stub smali — actual Java class is compiled into the .so's companion DEX
# This redirects the Application's startService to our compiled class.
# The real implementation is in ModService.java → compiled by Gradle → dexed.
# Place the actual classes.dex in the APK's root as classes4.dex.

.class public Lcom/nullsmod/ModService;
.super Landroid/app/Service;
.source "ModService.java"

.method public constructor <init>()V
    .registers 1
    invoke-direct {p0}, Landroid/app/Service;-><init>()V
    return-void
.end method

.method public onBind(Landroid/content/Intent;)Landroid/os/IBinder;
    .registers 2
    const/4 v0, 0x0
    return-object v0
.end method

.method public onStartCommand(Landroid/content/Intent;II)I
    .registers 4
    const/4 v0, 0x1   # START_STICKY
    return v0
.end method
