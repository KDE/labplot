/*
 *    SPDX-License-Identifier: GPL-2.0-or-later
 *    SPDX-FileCopyrightText: 2010 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _PANELPLUGINHANDLER_H
#define _PANELPLUGINHANDLER_H

#include <QObject>
#include "panelplugin.h"
#include "cantor_export.h"

namespace Cantor
{
  class PanelPluginHandlerPrivate;
  class Session;

  /**
   * Simple interface that exports a list of known PanelPlugins.
   * Needed as the Panel must be handled by the Shell
   */

  class CANTOR_EXPORT PanelPluginHandler : public QObject
  {
    Q_OBJECT
  public:
    explicit PanelPluginHandler(QObject* parent);
    ~PanelPluginHandler() override;

    QList<PanelPlugin*> allPlugins();
    QList<PanelPlugin*> plugins(Session*);

    using PanelStates = QMap<QString, Cantor::PanelPlugin::State>;
    QList<PanelPlugin*> activePluginsForSession(Session*, const PanelStates&);

    void loadPlugins();

  private:
    PanelPluginHandlerPrivate* d;

  };

}

#endif /* _PANELPLUGINHANDLER_H */
