// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

plugins {
    id("com.android.application") version "8.13.1" apply false
    id("com.android.library") version "8.13.1" apply false
    id("org.jetbrains.kotlin.android") version "2.2.21" apply false
    id("org.jetbrains.kotlin.plugin.parcelize") version "2.2.21" apply false
    kotlin("plugin.serialization") version "1.9.20" apply false
    id("org.jlleitschuh.gradle.ktlint") version "14.0.1" apply false
    id("com.github.triplet.play") version "3.12.2" apply false
    id("idea")
}

tasks.register("clean").configure {
    delete(rootProject.layout.buildDirectory)
}

// Do not index .cache/cpm on Android Studio
idea {
    module {
        excludeDirs.add(file("$rootDir/.cache/cpm"))
    }
}