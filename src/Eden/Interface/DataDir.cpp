// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DataDir.h"
#include "core/hle/service/acc/profile_manager.h"
#include "qt_common/qt_common.h"
#include "qt_common/util/content.h"

#include <QDesktopServices>
#include <QFutureWatcher>
#include <QtConcurrentRun>

DataDir::DataDir(QObject* parent) : QObject{parent} {}

void DataDir::clear()
{
    std::string user_id = selectProfile();
    QtCommon::Content::ClearDataDir(m_dir, user_id);
    scan();
}

void DataDir::open()
{
    std::string user_id = selectProfile();
    QDesktopServices::openUrl(QUrl::fromLocalFile(
        QString::fromStdString(FrontendCommon::DataManager::GetDataDirString(m_dir, user_id))));
}

void DataDir::upload()
{
    std::string user_id = selectProfile();
    QtCommon::Content::ExportDataDir(m_dir, user_id, m_exportName);
}

void DataDir::download()
{
    std::string user_id = selectProfile();
    QtCommon::Content::ImportDataDir(m_dir, user_id, std::bind(&DataDir::scan, this));
}

void DataDir::scan() {
    setText(tr("Calculating..."));

    QFutureWatcher<u64> *watcher = new QFutureWatcher<u64>(this);

    connect(watcher, &QFutureWatcher<u64>::finished, this, [=, this]() {
        u64 size = watcher->result();
        setText(
            QString::fromStdString(FrontendCommon::DataManager::ReadableBytesSize(size)));
        watcher->deleteLater();
    });

    watcher->setFuture(
        QtConcurrent::run([this]() { return FrontendCommon::DataManager::DataDirSize(m_dir); }));
}

// TODO: Select profile :)
std::string DataDir::selectProfile() {
    const auto uuid = QtCommon::system->GetProfileManager().FindExistingProfileUUIDs()[0];
    auto user_id = uuid.AsU128();

    return fmt::format("{:016X}{:016X}", user_id[1], user_id[0]);
}

QString DataDir::exportName() const {
    return m_exportName;
}

void DataDir::setExportName(const QString& newExportName) {
    if (m_exportName == newExportName)
        return;
    m_exportName = newExportName;
    emit exportNameChanged(m_exportName);
}

FrontendCommon::DataManager::DataDir DataDir::dir() const {
    return m_dir;
}

void DataDir::setDir(FrontendCommon::DataManager::DataDir newDir) {
    if (m_dir == newDir)
        return;
    m_dir = newDir;
    emit dirChanged(m_dir);
}

QString DataDir::text() const {
    return m_text;
}

void DataDir::setText(const QString& newText) {
    if (m_text == newText)
        return;
    m_text = newText;
    emit textChanged(m_text);
}
