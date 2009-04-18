/***************************************************************************
    File                 : HelloWorldPlugin.h
    Project              : LabPlot/SciDAVis
    Description          : Example plugin
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses) 
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef HELLOWORLDPLUGIN_H
#define HELLOWORLDPLUGIN_H

#include "core/interfaces.h"
#include "core/AbstractPart.h"
#include <QObject>

class HelloWorldPart: public AbstractPart {
    Q_OBJECT

	public:
		HelloWorldPart(const QString &name);
		~HelloWorldPart();
		virtual QWidget *view() const;
		virtual bool fillProjectMenu(QMenu * menu);
		virtual QIcon icon() const;

	private:
		mutable QWidget *m_view;
};

class HelloWorldConfigPage : public ConfigPageWidget
{
	Q_OBJECT

	public:
		HelloWorldConfigPage();
		~HelloWorldConfigPage();

	public slots:
		virtual void apply();
};

class HelloWorldPlugin: public QObject, public VersionedPlugin, public PartMaker, public ConfigPageMaker {
	Q_OBJECT
	Q_INTERFACES(VersionedPlugin PartMaker ConfigPageMaker)

	public:
		virtual int pluginTargetAppVersion() const;
		virtual QString pluginTargetAppName() const;
		virtual QString pluginName() const;
		virtual AbstractPart *makePart();
		virtual QAction *makeAction(QObject *parent);
		virtual ConfigPageWidget *makeConfigPage();
		virtual QString configPageLabel();
};


#endif


