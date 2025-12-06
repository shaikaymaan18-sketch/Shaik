// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.utils

import android.net.Uri
import android.provider.DocumentsContract
import java.io.File

object PathUtil {

    /**
     * Converts a content:// URI from the Storage Access Framework to a real filesystem path.
     *
     */
    fun getPathFromUri(uri: Uri): String? {
        val docId = try {
            DocumentsContract.getTreeDocumentId(uri)
        } catch (_: Exception) {
            return null
        }

        if (docId.startsWith("primary:")) {
            val relativePath = docId.removePrefix("primary:")
            return "/storage/emulated/0/$relativePath"
        }

        // external SD cards and other volumes)
        val split = docId.split(":")
        if (split.size >= 2) {
            val volumeId = split[0]
            val relativePath = split.getOrElse(1) { "" }
            val possiblePaths = listOf(
                "/storage/$volumeId/$relativePath",
                "/mnt/media_rw/$volumeId/$relativePath"
            )
            for (path in possiblePaths) {
                val file = File(path)
                if (file.exists() && file.isDirectory) {
                    return path
                }
            }
        }

        return null
    }

    /**
     * Validates that a path is a valid, writable directory.
     * Creates the directory if it doesn't exist.
     */
    fun validateDirectory(path: String): Boolean {
        val dir = File(path)

        if (!dir.exists()) {
            if (!dir.mkdirs()) {
                return false
            }
        }

        return dir.isDirectory && dir.canWrite()
    }

    /**
     * Copies a directory recursively from source to destination.
     */
    fun copyDirectory(source: File, destination: File, overwrite: Boolean = true): Boolean {
        return try {
            source.copyRecursively(destination, overwrite)
            true
        } catch (_: Exception) {
            false
        }
    }

    /**
     * Checks if a directory has any content.
     */
    fun hasContent(path: String): Boolean {
        val dir = File(path)
        return dir.exists() && dir.listFiles()?.isNotEmpty() == true
    }


    fun truncatePathForDisplay(path: String, maxLength: Int = 40): String {
        return if (path.length > maxLength) {
            "...${path.takeLast(maxLength - 3)}"
        } else {
            path
        }
    }
}
