/***************************************************************************
    File                 : AspectPrivate.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Private data managed by AbstractAspect.

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
#include "backend/core/AbstractAspect.h"
#include "backend/core/AspectPrivate.h"
#include <QRegExp>
#include <QStringList>

/**
 * \class AbstractAspect::Private
 * \brief Private data managed by AbstractAspect.
 */

AbstractAspect::Private::Private(AbstractAspect * owner, const QString& name)
	: m_name(name.isEmpty() ? "1" : name), m_hidden(false), m_owner(owner), m_parent(0)
{
	m_creation_time = QDateTime::currentDateTime();
}

AbstractAspect::Private::~Private()
{
	foreach(AbstractAspect * child, m_children)
			delete child;
}

void AbstractAspect::Private::insertChild(int index, AbstractAspect* child)
{
	m_children.insert(index, child);
	// Always remove from any previous parent before adding to a new one!
	// Can't handle this case here since two undo commands have to be created.
	Q_ASSERT(child->m_aspect_private->m_parent == 0);
	child->m_aspect_private->m_parent = m_owner;
	connect(child, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)));
	connect(child, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)));
	connect(child, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)),
			m_owner, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)));
	connect(child, SIGNAL(aspectAdded(const AbstractAspect *)),
			m_owner, SIGNAL(aspectAdded(const AbstractAspect *)));
	connect(child, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)),
			m_owner, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)));
	connect(child, SIGNAL(aspectRemoved(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)),
			m_owner, SIGNAL(aspectRemoved(const AbstractAspect *, const AbstractAspect *, const AbstractAspect *)));
	connect(child, SIGNAL(aspectHiddenAboutToChange(const AbstractAspect *)),
			m_owner, SIGNAL(aspectHiddenAboutToChange(const AbstractAspect *)));
	connect(child, SIGNAL(aspectHiddenChanged(const AbstractAspect*)),
			m_owner, SIGNAL(aspectHiddenChanged(const AbstractAspect*)));
	connect(child, SIGNAL(statusInfo(const QString&)),
			m_owner, SIGNAL(statusInfo(const QString&)));
}

int AbstractAspect::Private::indexOfChild(const AbstractAspect *child) const
{
	for(int i=0; i<m_children.size(); i++)
		if(m_children.at(i) == child) return i;
	return -1;
}

int AbstractAspect::Private::removeChild(AbstractAspect* child)
{
	int index = indexOfChild(child);
	Q_ASSERT(index != -1);
	m_children.removeAll(child);
	QObject::disconnect(child, 0, m_owner, 0);
	child->m_aspect_private->m_parent = 0;
	return index;
}

QString AbstractAspect::Private::uniqueNameFor(const QString &current_name) const
{
	QStringList child_names;
	foreach(AbstractAspect * child, m_children)
		child_names << child->name();

	if (!child_names.contains(current_name))
		return current_name;

	QString base = current_name;
	int last_non_digit;
	for (last_non_digit = base.size()-1; last_non_digit>=0 &&
			base[last_non_digit].category() == QChar::Number_DecimalDigit; --last_non_digit)
		base.chop(1);
	if (last_non_digit >=0 && base[last_non_digit].category() != QChar::Separator_Space)
		base.append(" ");

	int new_nr = current_name.right(current_name.size() - base.size()).toInt();
	QString new_name;
	do
		new_name = base + QString::number(++new_nr);
	while (child_names.contains(new_name));

	return new_name;
}
