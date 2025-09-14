# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

set_source_files_properties(Util.qml
    PROPERTIES
    QT_QML_SINGLETON_TYPE true
)

EdenModule(
    NAME Util
    URI Eden.Util
    QML_FILES
        Util.qml
)
