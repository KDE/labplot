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

#include "panelpluginhandler.h"
using namespace Cantor;

#include <QDebug>
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginInfo>

#include "session.h"
#include "panelplugin.h"
#include "backend.h"

class Cantor::PanelPluginHandlerPrivate
{
  public:
    QList<Cantor::PanelPlugin*> plugins;
    Cantor::Session* session;
};

PanelPluginHandler::PanelPluginHandler( QObject* parent ) : QObject(parent) ,
                                                            d(new PanelPluginHandlerPrivate)
{
    setObjectName(QLatin1String("PanelPluginHandler"));
    d->session=0;
}

PanelPluginHandler::~PanelPluginHandler()
{
    delete d;
}

void PanelPluginHandler::loadPlugins()
{
    if(d->session==0)
        return;
    qDebug()<<"loading panel plugins for session of type "<<d->session->backend()->name();

    KService::List services;
    KServiceTypeTrader* trader = KServiceTypeTrader::self();

    services = trader->query(QLatin1String("Cantor/PanelPlugin"));

    foreach (const KService::Ptr &service,   services)
    {
        QString error;

        qDebug()<<"found service"<<service->name();
        Cantor::PanelPlugin* plugin=service->createInstance<Cantor::PanelPlugin>(this,  QVariantList(),   &error);
        if (plugin==0)
        {
            qDebug()<<"error loading panelplugin"<<service->name()<<":  "<<error;
            continue;
        }

        qDebug()<<"created it";

        KPluginInfo info(service);
        plugin->setPluginInfo(info);

        qDebug()<<"plugin "<<service->name()<<" requires "<<plugin->requiredExtensions();
        bool supported=true;
        foreach(const QString& req, plugin->requiredExtensions())
            supported=supported && d->session->backend()->extensions().contains(req);

        supported=supported && ( (d->session->backend()->capabilities() & plugin->requiredCapabilities()) == plugin->requiredCapabilities());
        qDebug()<<"plugin "<<service->name()<<" is "<<(supported ? "":" not ")<<" supported";

        if(supported)
        {
            d->plugins.append(plugin);
            plugin->setSession(d->session);
        }else
        {
            plugin->deleteLater();
        }
    }

    emit pluginsChanged();
}

void PanelPluginHandler::setSession(Session* session)
{
    qDeleteAll(d->plugins);
    d->plugins.clear();
    d->session=session;
    loadPlugins();
}

QList<PanelPlugin*> PanelPluginHandler::plugins()
{
    return d->plugins;
}

void PanelPluginHandler::addPlugin(PanelPlugin* plugin)
{
    d->plugins.append(plugin);
}


