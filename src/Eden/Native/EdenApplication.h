// SPDX-FileCopyrightText: Copyright 2026 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QApplication>
#include "Eden/Interface/QMLConfig.h"

class EdenApplication : public QApplication
{
    Q_OBJECT
    Q_PROPERTY(bool shouldReload MEMBER m_shouldReload)
public:
    EdenApplication(int &argc, char *argv[]);

public slots:
    void reload();
    int run();

private:
    QMLConfig *config;
    bool m_shouldReload = false;
};
