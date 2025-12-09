// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include <CarboxylApplication.h>
#include <CarboxylQuickInterface.h>

#include "qt_common/abstract/frontend.h"

#include "LibQtCommon.h"

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

QuickProgressDialog::QuickProgressDialog(const QString& labelText,
                                         const QString& cancelButtonText, int minimum,
                                         int maximum, QObject* parent, Qt::WindowFlags f)
    : QtProgressDialog(labelText, cancelButtonText, minimum, maximum, parent, f),
      m_dialog(new CarboxylProgressDialog(labelText, cancelButtonText, minimum, maximum, parent)) {}

bool QuickProgressDialog::wasCanceled() const {
    return m_dialog->wasCanceled();
}

void QuickProgressDialog::setWindowModality(Qt::WindowModality modality) {}

void QuickProgressDialog::setMinimumDuration(int durationMs) {
    m_dialog->setMinimumDuration(durationMs);
}

void QuickProgressDialog::setAutoClose(bool autoClose) {
    m_dialog->setAutoClose(autoClose);
}

void QuickProgressDialog::setAutoReset(bool autoReset) {
    m_dialog->setAutoReset(autoReset);
}

void QuickProgressDialog::setTitle(QString title) {
    m_dialog->setTitle(title);
}

void QuickProgressDialog::setLabelText(QString text) {
    m_dialog->setLabelText(text);
}

void QuickProgressDialog::setMinimum(int min) {
    m_dialog->setMinimum(min);
}

void QuickProgressDialog::setMaximum(int max) {
    m_dialog->setMaximum(max);
}

void QuickProgressDialog::setValue(int value) {
    m_dialog->setValue(value);
}

bool QuickProgressDialog::close() {
    m_dialog->close();
    return true;
}

void QuickProgressDialog::show() {
    m_dialog->show();
}

std::unique_ptr<QtProgressDialog> newProgressDialog(const QString& labelText,
                                                    const QString& cancelButtonText, int minimum,
                                                    int maximum, Qt::WindowFlags f) {
    return std::make_unique<QuickProgressDialog>(labelText, cancelButtonText, minimum, maximum, rootObject);
}

QtProgressDialog* newProgressDialogPtr(const QString& labelText, const QString& cancelButtonText,
                                       int minimum, int maximum,
                                       Qt::WindowFlags f) {
    return new QuickProgressDialog(labelText, cancelButtonText, minimum, maximum, rootObject);
}


} // namespace QtCommon::Frontend
