/***************************************************************************
    File                 : HelloWorldPlugin.cpp
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

#include "HelloWorldPlugin.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QAction>

HelloWorldPart::HelloWorldPart(const QString &name) 
	: AbstractPart(name), m_view(NULL) {
}

HelloWorldPart::~HelloWorldPart() {
	delete m_view;
}

bool HelloWorldPart::fillProjectMenu(QMenu * menu) {
	Q_UNUSED(menu);
	return false;
}

QWidget *HelloWorldPart::view() const {
	if (!m_view) {
		m_view = new QWidget();
		QVBoxLayout *layout = new QVBoxLayout(m_view);
		layout->addWidget(new QLabel(tr("Hello World!")));
	}
	return m_view;
}

QIcon HelloWorldPart::icon() const
{
	QIcon ico;
	ico.addPixmap(QPixmap(":/helloworldlogo"));
	return ico;
}

/* --------------- */

int HelloWorldPlugin::pluginTargetAppVersion() const {
	return 0x000300;
}

QString HelloWorldPlugin::pluginTargetAppName() const {
	return "SciDAVis";
}

QString HelloWorldPlugin::pluginName() const {
	return "HelloWorldPlugin";
}

AbstractPart * HelloWorldPlugin::makePart() {
	return new HelloWorldPart(tr("HelloWorld %1").arg(1));
}

QAction * HelloWorldPlugin::makeAction(QObject *parent) {
	QAction *newHelloworld = new QAction(tr("New &Hello World Widget"), parent);
	newHelloworld->setShortcut(tr("Ctrl+Shift+W", "new hello world widget shortcut"));
	newHelloworld->setIcon(QIcon(QPixmap(":/helloworldlogo")));
	return newHelloworld;
}

ConfigPageWidget * HelloWorldPlugin::makeConfigPage() {
	return new HelloWorldConfigPage();
}
		
HelloWorldConfigPage::HelloWorldConfigPage() {
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(new QLabel(tr("Hello World!")));
}

HelloWorldConfigPage::~HelloWorldConfigPage() {
}

void HelloWorldConfigPage::apply() {
	// read settings from ui and change them in
}

QString HelloWorldPlugin::configPageLabel() {
	return QObject::tr("Hello World");
}

Q_EXPORT_PLUGIN2(scidavis_helloworldplugin, HelloWorldPlugin)

