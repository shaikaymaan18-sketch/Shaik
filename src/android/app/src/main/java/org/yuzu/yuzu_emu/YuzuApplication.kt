// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

package org.yuzu.yuzu_emu

import android.app.Application
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import org.yuzu.yuzu_emu.features.input.NativeInput
import java.io.File
import java.io.FileOutputStream
import java.security.KeyStore
import javax.net.ssl.TrustManagerFactory
import javax.net.ssl.X509TrustManager
import org.yuzu.yuzu_emu.utils.DirectoryInitialization
import org.yuzu.yuzu_emu.utils.DocumentsTree
import org.yuzu.yuzu_emu.utils.GpuDriverHelper
import org.yuzu.yuzu_emu.utils.Log
import org.yuzu.yuzu_emu.utils.PowerStateUpdater

fun Context.getPublicFilesDir(): File = getExternalFilesDir(null) ?: filesDir

class YuzuApplication : Application() {
    private fun createNotificationChannels() {
        val name: CharSequence = getString(R.string.app_notification_channel_name)
        val description = getString(R.string.app_notification_channel_description)
        val foregroundService = NotificationChannel(
            getString(R.string.app_notification_channel_id),
            name,
            NotificationManager.IMPORTANCE_DEFAULT
        )
        foregroundService.description = description
        foregroundService.setSound(null, null)
        foregroundService.vibrationPattern = null

        val noticeChannel = NotificationChannel(
            getString(R.string.notice_notification_channel_id),
            getString(R.string.notice_notification_channel_name),
            NotificationManager.IMPORTANCE_HIGH
        )
        noticeChannel.description = getString(R.string.notice_notification_channel_description)
        noticeChannel.setSound(null, null)

        // Register the channel with the system; you can't change the importance
        // or other notification behaviors after this
        val notificationManager = getSystemService(NotificationManager::class.java)
        notificationManager.createNotificationChannel(noticeChannel)
        notificationManager.createNotificationChannel(foregroundService)
    }

    override fun onCreate() {
        super.onCreate()
        application = this
        documentsTree = DocumentsTree()
        DirectoryInitialization.start()
        NativeLibrary.playTimeManagerInit()
        GpuDriverHelper.initializeDriverParameters()
        NativeInput.reloadInputDevices()
        NativeLibrary.logDeviceInfo()
        PowerStateUpdater.start()
        Log.logDeviceInfo()

        // Initialize CA certificates for HTTPS
        if (NativeLibrary.isUpdateCheckerEnabled()) {
            initializeCACertificates()
        }

        createNotificationChannels()
    }

    // required for httplib and update checker
    private fun initializeCACertificates() {
        try {
            val trustManagerFactory = TrustManagerFactory.getInstance(
                TrustManagerFactory.getDefaultAlgorithm()
            )
            trustManagerFactory.init(null as KeyStore?)

            val trustManagers = trustManagerFactory.trustManagers
            if (trustManagers.isEmpty()) {
                Log.error("[SSL] No trust managers found")
                return
            }
            val x509TrustManager = trustManagers[0] as X509TrustManager
            val acceptedIssuers = x509TrustManager.acceptedIssuers

            if (acceptedIssuers.isEmpty()) {
                Log.error("[SSL] No CA certificates found")
                return
            }

            val certFile = File(filesDir, "cacert.pem")
            FileOutputStream(certFile).use { outputStream ->
                for (cert in acceptedIssuers) {
                    outputStream.write("-----BEGIN CERTIFICATE-----\n".toByteArray())
                    val encoded = android.util.Base64.encodeToString(
                        cert.encoded,
                        android.util.Base64.DEFAULT
                    )
                    outputStream.write(encoded.toByteArray())
                    outputStream.write("-----END CERTIFICATE-----\n".toByteArray())
                }
            }

            NativeLibrary.setCACertificatePath(certFile.absolutePath)
            Log.info("[SSL] Initialized ${acceptedIssuers.size} CA certificates at: ${certFile.absolutePath}")
        } catch (e: Exception) {
            Log.error("[SSL] Failed to initialize CA certificates: ${e.message}")
        }
    }

    companion object {
        var documentsTree: DocumentsTree? = null
        lateinit var application: YuzuApplication

        val appContext: Context
            get() = application.applicationContext
    }
}
