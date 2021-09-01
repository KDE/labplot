/*
    SPDX-License-Identifier: GPL-2.0-or-later

    ---
    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef _PANELPLUGINHANDLER_H
#define _PANELPLUGINHANDLER_H

#include <QObject>
#include <cantor/cantor_export.h>

namespace Cantor
{
class PanelPluginHandlerPrivate;
class PanelPlugin;
class Session;

/**
 * Simple interface that exports a list of known PanelPlugins.
 * Needed as the Panel must be handled by the Shell while plugins
 * belong to the Part.
 */

class CANTOR_EXPORT PanelPluginHandler : public QObject
{
  Q_OBJECT
  public:
    explicit PanelPluginHandler(QObject* parent);
    ~PanelPluginHandler() override;

    QList<PanelPlugin*> plugins();

    void addPlugin(PanelPlugin* plugin);

    void setSession(Session* session);

  Q_SIGNALS:
    void pluginsChanged();

  private:
    void loadPlugins();

  private:
    PanelPluginHandlerPrivate* d;

};

}

#endif /* _PANELPLUGINHANDLER_H */
