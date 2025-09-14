# SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
# SPDX-License-Identifier: GPL-3.0-or-later

# SPDX-FileCopyrightText: Copyright 2025 crueter
# SPDX-License-Identifier: GPL-3.0-or-later

set_source_files_properties(Constants.qml
    PROPERTIES
    QT_QML_SINGLETON_TYPE true
)

EdenModule(
    NAME Constants
    URI Eden.Constants
    QML_FILES Constants.qml
)
