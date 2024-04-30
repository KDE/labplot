// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QProcess>

/// @returns an executable name that is safe to call
QString safeExecutableName(const QString &executableName, const QStringList &paths = {});
/// convenience wrapper to start a process on the host (as opposed to potentially inside a sandbox - e.g. flatpak)
void startHostProcess(QProcess &proc, QProcess::OpenMode mode = QProcess::ReadWrite);
void startHostProcess(QProcess &proc, const QString &program, const QStringList &arguments = {}, QProcess::OpenMode mode = QProcess::ReadWrite);
