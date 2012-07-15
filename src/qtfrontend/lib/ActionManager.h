/***************************************************************************
    File                 : ActionManager.h
    Project              : SciDAVis
    Description          : Manages QActions and their shortcuts
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QAction>
#include <QList>
#include <QMap>
#include "lib/macros.h"
class QString;
class QKeySequence;

//! Manages QActions and their shortcuts
/**
 * An ActionManager object is meant to manage all actions for one
 * widget class. All actions are addressed by their internal name 
 * (a string that must be unique within the widget class). The manager
 * stores multiple QActions per internal name, i.e., one action
 * per instance of the widget class. The text of the action is the
 * localized string of the action while the internal name is never
 * translated as it is meant to be used to identify the action in the
 * configuration of the application. The localized text is taken
 * from the last added action or removed action for each internal name.
 * The managed widgets can change their language as often as needed 
 * but should have the same language all the time. Otherwise, the 
 * language of actionText() will depend on the order of the action addition. 
 * Actions that are deleted (e.g., when their widget is deleted) are automatically
 * removed from the manager. The keyboard shortcuts assigned to an internal name
 * are preserved even when no action of the type exists. 
 * If setShortcuts() has been called before addAction() the action's shortcuts 
 * are replaced in addAction(). If setShortcuts() has not been called 
 * before the first call to addAction() (for a specific name that is) the
 * shortcuts of the first added action will be taken for all other added actions.
 *
 * The typical usage of ActionManager is:
 * - read configured shortcuts from the configuration files of the application and
 *   call ActionManager::setShortcuts() for each of them
 * - create an instance of the widget class
 *   -# create actions for the widget
 *   -# call ActionManager::addAction() for each action
 *   -# if the configuration files contained a shortcut for the action, it will
 *      be set for the added action, otherwise the default shortcut set in
 *      the widget initialization code will be preserved
 * - create more instances of the widget class and register their actions
 *   in the same way
 * - call ActionManager::setShortcuts() to change the shortcut for all actions
 *   addressed by the same internal name in all existing widget instances
 *
 * There is one more thing to consider:
 * As long as addShortcut() is called and addAction() is not, actionText() will
 * return the internal name instead of the localized name. It might therefore
 * be a good idea to create an instance of the corresponding widget at application
 * startup, create all actions for it, and immediately delete it again. 
 */
class ActionManager : public QObject
{
	Q_OBJECT

	public:
		ActionManager();
		~ActionManager();
		
		void addAction(QAction * action, const QString& internal_name);
		QList<QKeySequence> shortcuts(const QString& internal_name) const;
		void setShortcuts(const QString& internal_name, const QList<QKeySequence>& sequences);
		QString actionText(const QString& internal_name) const;
		QList<QString> internalNames() const;
		CLASS_ACCESSOR(QString, m_title, title, Title);

	public slots:
		void removeAction(QAction * action);
		void removeAction(QObject * action);
	
	private:
		QMap< QString, QList<QAction *> * > m_action_registry;
		QMap< QString, QList<QKeySequence> > m_action_shortcuts;
		QMap< QString, QString > m_action_texts;
		QString m_title;
};

#endif // ACTIONMANAGER_H
