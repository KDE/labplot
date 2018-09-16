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

#ifndef _PANEL_PLUGIN_H
#define _PANEL_PLUGIN_H

#include <KXMLGUIClient>
#include <QObject>
#include <KService/KPluginInfo>

#include <cantor/backend.h>

#include <cantor/cantor_export.h>

namespace Cantor
{
class Session;
class PanelPluginPrivate;

/**
 * A plugin provides some additional features for the worksheet
 */
class CANTOR_EXPORT PanelPlugin : public QObject /*, public KXMLGUIClient*/
{
  Q_OBJECT
  public:
    /**
     * Create a new PanelPlugin
     * @param parent the parent Object @see QObject
     **/
    explicit PanelPlugin( QObject* parent );
    /**
     * Destructor
     */
    ~PanelPlugin() override;

    /**
     * Sets the properties of this PanelPlugin
     * according to KPluginInfo
     * @param info KPluginInfo
     */
    void setPluginInfo(KPluginInfo info);

    /**
     * Returns a list of all extensions, the current backend
     * must provide to make this PanelPlugin work. If it doesn't
     * this PanelPlugin won't be enabled
     * @return list of required extensions
    */
    QStringList requiredExtensions();


    /**
     * Returns the capabilities, the current backend
     * must provide to make this PanelPlugin work. If it doesn't
     * this PanelPlugin won't be enabled
     * @return the required capabilities
    */
    virtual Backend::Capabilities requiredCapabilities();


    /**
     * Returns the name of the plugin
     * @return name of the plugin
     */
    QString name();

    /**
     * returns the widget, provided by this plugin
     * @return the widget, provided by this plugin
     **/
    virtual QWidget* widget() = 0;

    void setParentWidget(QWidget* widget);
    QWidget* parentWidget();

    /**
     * sets the session this plugin operates on
     **/
    void setSession(Session* session);

    /**
     * returns the session
     */
    Session* session();

  Q_SIGNALS:
    void requestRunCommand(const QString& cmd);
    void visibilityRequested();

  protected:
    virtual void onSessionChanged();

  private:
    PanelPluginPrivate* d;
};

}

#endif /* _PANEL_PLUGIN_H */
