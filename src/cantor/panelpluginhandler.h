/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010 Alexander Rieder <alexanderrieder@gmail.com>
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
