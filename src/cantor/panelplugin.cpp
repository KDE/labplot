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

#include "panelplugin.h"
using namespace Cantor;

#include <KPluginInfo>
#include <QDebug>

class Cantor::PanelPluginPrivate
{
  public:
    QString name;
    QStringList requiredExtensions;
    Session* session;
    QWidget* parentWidget;
};

PanelPlugin::PanelPlugin( QObject* parent) : QObject(parent), /* KXMLGUIClient(dynamic_cast<KXMLGUIClient*>(parent)),*/
                                             d(new PanelPluginPrivate)
{
    d->parentWidget=0;
    d->session=0;
}

PanelPlugin::~PanelPlugin()
{
    delete d;
}

void PanelPlugin::setParentWidget(QWidget* widget)
{
    d->parentWidget=widget;
}

QWidget* PanelPlugin::parentWidget()
{
    return d->parentWidget;
}

void PanelPlugin::setPluginInfo(KPluginInfo info)
{
    d->name=info.name();
    d->requiredExtensions=info.property(QLatin1String("RequiredExtensions")).toStringList();
}


QStringList PanelPlugin::requiredExtensions()
{
    return d->requiredExtensions;
}

Backend::Capabilities PanelPlugin::requiredCapabilities()
{
    return Backend::Nothing;
}

QString PanelPlugin::name()
{
    return d->name;
}

Session* PanelPlugin::session()
{
    return d->session;
}

void PanelPlugin::setSession(Session* session)
{
    qDebug()<<"setting session to "<<session;
    d->session=session;
    onSessionChanged();
}

void PanelPlugin::onSessionChanged()
{

}


