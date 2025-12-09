// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qt_progress_dialog.h"

namespace QtCommon::Frontend {

QtProgressDialog::QtProgressDialog(const QString&,
                                   const QString&,
                                   int,
                                   int,
                                   QObject* parent,
                                   Qt::WindowFlags)
    : QObject(parent)
{}

QtProgressDialog::~QtProgressDialog() {}
}
