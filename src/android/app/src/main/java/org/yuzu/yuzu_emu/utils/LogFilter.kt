// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

package org.yuzu.yuzu_emu.utils

import android.content.Context
import android.net.Uri
import androidx.core.content.FileProvider
import androidx.documentfile.provider.DocumentFile
import java.io.BufferedReader
import java.io.File
import java.io.InputStreamReader
import java.io.OutputStreamWriter
import java.util.regex.Pattern

object LogFilter {

    /**
     * Filters log content to show only warnings, errors, and critical messages
     * @param context Android context
     * @param inputUri URI of the input log file
     * @param outputFile File object for the output filtered log file
     * @return true if filtering was successful, false otherwise
     */
    fun filterLogs(context: Context, inputUri: Uri, outputFile: File): Boolean {
        return try {
            val inputDocFile = DocumentFile.fromSingleUri(context, inputUri)

            if (inputDocFile == null || !inputDocFile.exists()) {
                Log.error("[LogFilter] Input file does not exist: $inputUri")
                return false
            }

            Log.info("[LogFilter] Starting filtering from $inputUri to ${outputFile.absolutePath}")

            // Define the regex pattern for warnings, errors, and criticals
            val pattern = Pattern.compile(".*<(?:Warning|Error|Critical)>.*", Pattern.CASE_INSENSITIVE)

            var filteredLines = 0
            var totalLines = 0

            context.contentResolver.openInputStream(inputUri)?.use { inputStream ->
                outputFile.outputStream().use { outputStream ->
                    val reader = BufferedReader(InputStreamReader(inputStream))
                    val writer = OutputStreamWriter(outputStream)

                    var line: String?
                    while (reader.readLine().also { line = it } != null) {
                        totalLines++
                        if (pattern.matcher(line!!).matches()) {
                            writer.write(line)
                            writer.write("\n")
                            filteredLines++
                        }
                    }

                    writer.flush()
                }
            }

            Log.info("[LogFilter] Filtered $filteredLines lines out of $totalLines total lines")
            Log.info("[LogFilter] Output file size: ${outputFile.length()} bytes")
            true
        } catch (e: Exception) {
            Log.error("[LogFilter] Error filtering logs: ${e.message}")
            e.printStackTrace()
            false
        }
    }

    /**
     * Creates a filtered log file in the app's cache directory
     * @param context Android context
     * @param originalUri Original log file URI
     * @return Pair of File and URI, or null if failed
     */
    fun createFilteredLogFile(context: Context, originalUri: Uri): Pair<File, Uri>? {
        return try {
            val originalFile = DocumentFile.fromSingleUri(context, originalUri)
            if (originalFile == null || !originalFile.exists()) {
                Log.error("[LogFilter] Original file does not exist: $originalUri")
                return null
            }

            // Create filtered file name
            val originalName = originalFile.name ?: "eden_log.txt"
            val filteredName = if (originalName.endsWith(".txt")) {
                originalName.replace(".txt", "_filtered.txt")
            } else {
                "${originalName}_filtered"
            }

            // Create file in app's cache directory
            val cacheDir = File(context.cacheDir, "logs")
            if (!cacheDir.exists()) {
                cacheDir.mkdirs()
            }

            val filteredFile = File(cacheDir, filteredName)

            // Delete existing file if it exists
            if (filteredFile.exists()) {
                filteredFile.delete()
            }

            // Create the file
            filteredFile.createNewFile()

            Log.info("[LogFilter] Created filtered file: ${filteredFile.absolutePath}")

            // Use FileProvider to get a content URI for the file
            val uri = FileProvider.getUriForFile(
                context,
                "${context.packageName}.provider",
                filteredFile
            )

            Pair(filteredFile, uri)
        } catch (e: Exception) {
            Log.error("[LogFilter] Error creating filtered file: ${e.message}")
            e.printStackTrace()
            null
        }
    }
}