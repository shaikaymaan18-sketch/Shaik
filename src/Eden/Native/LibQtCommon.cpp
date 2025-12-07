// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CarboxylQuickInterface.h"
#include "CarboxylApplication.h"
#include "qt_common/abstract/frontend.h"

namespace QtCommon::Frontend {

StandardButton ShowMessage(Icon icon, const QString& title, const QString& text,
                           StandardButtons buttons, QObject* parent) {
    auto res = g_carboxylApp->interface()->showMessageBox(
        CarboxylEnums::Icon(int(icon)), title, text,
        QPlatformDialogHelper::StandardButton(int(buttons)), parent);
    return StandardButton(res);
}

const QString GetOpenFileName(const QString& title, const QString& dir, const QString& filter,
                              QString* selectedFilter, Options options) {
    // TODO
    // return QFileDialog::getOpenFileName((QWidget *) rootObject, title, dir, filter,
    // selectedFilter, QFileDialog::Options(int(options)));
    return QString();
}

const QString GetSaveFileName(const QString& title, const QString& dir, const QString& filter,
                              QString* selectedFilter, Options options) {
    // return QFileDialog::getSaveFileName((QWidget *) rootObject, title, dir, filter,
    // selectedFilter, QFileDialog::Options(int(options)));
    return QString();
}


const QString GetExistingDirectory(const QString& caption, const QString& dir, Options options) {
    // return QFileDialog::getExistingDirectory((QWidget *) rootObject, caption, dir,
    // QFileDialog::Options(int(options)));
    return QString();
}

} // namespace QtCommon::Frontend
