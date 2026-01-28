// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>
#include "frontend_common/data_manager.h"

class DataDir : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(FrontendCommon::DataManager::DataDir dir READ dir WRITE setDir NOTIFY dirChanged FINAL)
    Q_PROPERTY(QString exportName READ exportName WRITE setExportName NOTIFY exportNameChanged FINAL)
public:
    explicit DataDir(QObject* parent = nullptr);

    QString text() const;
    void setText(const QString& newText);

    FrontendCommon::DataManager::DataDir dir() const;
    void setDir(FrontendCommon::DataManager::DataDir newDir);

    QString exportName() const;
    void setExportName(const QString& newExportName);

public slots:
    void clear();
    void open();
    void upload();
    void download();
    void scan();

private:
    QString m_text;
    FrontendCommon::DataManager::DataDir m_dir;
    QString m_exportName;

    std::string selectProfile();

signals:
    void textChanged(QString text);
    void dirChanged(FrontendCommon::DataManager::DataDir dir);
    void exportNameChanged(QString exportName);
};
