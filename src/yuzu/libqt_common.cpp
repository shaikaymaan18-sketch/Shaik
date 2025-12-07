// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFileDialog>
#include <QMessageBox>

#include "qt_common/abstract/frontend.h"
#include "qt_common/qt_common.h"

namespace QtCommon::Frontend {

StandardButton ShowMessage(
    Icon icon, const QString &title, const QString &text, StandardButtons buttons, QObject *parent)
{
    QMessageBox *box = new QMessageBox(QMessageBox::Icon(int(icon)), title, text, QMessageBox::StandardButtons(int(buttons)), (QWidget *) parent);
    return StandardButton(box->exec());
}

const QString GetOpenFileName(const QString &title,
                              const QString &dir,
                              const QString &filter,
                              QString *selectedFilter,
                              Options options)
{
    return QFileDialog::getOpenFileName((QWidget *) rootObject, title, dir, filter, selectedFilter, QFileDialog::Options(int(options)));
}

const QString GetSaveFileName(const QString &title,
                              const QString &dir,
                              const QString &filter,
                              QString *selectedFilter,
                              Options options)
{
    return QFileDialog::getSaveFileName((QWidget *) rootObject, title, dir, filter, selectedFilter, QFileDialog::Options(int(options)));
}

const QString GetExistingDirectory(const QString& caption, const QString& dir,
                                   Options options) {
    return QFileDialog::getExistingDirectory((QWidget *) rootObject, caption, dir, QFileDialog::Options(int(options)));
}

} // namespace QtCommon::Frontend
