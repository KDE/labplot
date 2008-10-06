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
#include "core/AbstractAspect.h"
#include "core/AspectPrivate.h"
#include <QRegExp>
#include <QStringList>

QSettings * AbstractAspect::Private::g_settings =
#ifdef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
#ifdef Q_OS_MAC
	new QSettings(QSettings::IniFormat, QSettings::UserScope, "SciDAVis", "SciDAVis");
#else
	new QSettings(QSettings::NativeFormat, QSettings::UserScope, "SciDAVis", "SciDAVis");
#endif
#else
// TODO ! add labplot/KDE specific config saving here
	new QSettings(QSettings::NativeFormat, QSettings::UserScope, "LabPlot", "LabPlot");
#endif
QHash<QString, QVariant> AbstractAspect::Private::g_defaults;

AbstractAspect::Private::Private(AbstractAspect * owner, const QString& name)
	: m_name(name), m_caption_spec("%n%C{ - }%c"), m_owner(owner), m_parent(0)
{
	m_creation_time = QDateTime::currentDateTime();
}

AbstractAspect::Private::~Private()
{
	foreach(AbstractAspect * child, m_children)
		delete child;
}

void AbstractAspect::Private::addChild(AbstractAspect* child)
{
	insertChild(m_children.count(), child);
}

void AbstractAspect::Private::insertChild(int index, AbstractAspect* child)
{
	emit m_owner->aspectAboutToBeAdded(m_owner, index);
	m_children.insert(index, child);
	// Always remove from any previous parent before adding to a new one!
	// Can't handle this case here since two undo commands have to be created.
	Q_ASSERT(child->m_aspect_private->m_parent == 0);
	child->m_aspect_private->m_parent = m_owner;
	connect(child, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectDescriptionAboutToChange(const AbstractAspect *)));
	connect(child, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectDescriptionChanged(const AbstractAspect *)));
	connect(child, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, int)), 
			m_owner, SIGNAL(aspectAboutToBeAdded(const AbstractAspect *, int)));
	connect(child, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *, int)), 
			m_owner, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *, int)));
	connect(child, SIGNAL(aspectAdded(const AbstractAspect *, int)), 
			m_owner, SIGNAL(aspectAdded(const AbstractAspect *, int)));
	connect(child, SIGNAL(aspectRemoved(const AbstractAspect *, int)), 
			m_owner, SIGNAL(aspectRemoved(const AbstractAspect *, int)));
	connect(child, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect *)));
	connect(child, SIGNAL(aspectAdded(const AbstractAspect *)), 
			m_owner, SIGNAL(aspectAdded(const AbstractAspect *)));
	connect(child, SIGNAL(statusInfo(const QString&)),
			m_owner, SIGNAL(statusInfo(const QString&)));
	emit m_owner->aspectAdded(m_owner, index);
	emit child->aspectAdded(child);
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
	emit m_owner->aspectAboutToBeRemoved(m_owner, index);
	emit child->aspectAboutToBeRemoved(child);
	m_children.removeAll(child);
	QObject::disconnect(child, 0, m_owner, 0);
	child->m_aspect_private->m_parent = 0;
	emit m_owner->aspectRemoved(m_owner, index);
	return index;
}

int AbstractAspect::Private::childCount() const
{
	return m_children.count();
}

AbstractAspect* AbstractAspect::Private::child(int index)
{
	Q_ASSERT(index >= 0 && index <= childCount());
	return m_children.at(index);
}

QString AbstractAspect::Private::name() const
{
	return m_name;
}

void AbstractAspect::Private::setName(const QString &value)
{
	emit m_owner->aspectDescriptionAboutToChange(m_owner);
	m_name = value;
	emit m_owner->aspectDescriptionChanged(m_owner);
}

QString AbstractAspect::Private::comment() const
{
	return m_comment;
}

void AbstractAspect::Private::setComment(const QString &value)
{
	emit m_owner->aspectDescriptionAboutToChange(m_owner);
	m_comment = value;
	emit m_owner->aspectDescriptionChanged(m_owner);
}

QString AbstractAspect::Private::captionSpec() const
{
	return m_caption_spec;
}

void AbstractAspect::Private::setCaptionSpec(const QString &value)
{
	emit m_owner->aspectDescriptionAboutToChange(m_owner);
	m_caption_spec = value;
	emit m_owner->aspectDescriptionChanged(m_owner);
}

void AbstractAspect::Private::setCreationTime(const QDateTime &time)
{
	m_creation_time = time;
}

int AbstractAspect::Private::indexOfMatchingBrace(const QString &str, int start)
{
	int result = str.indexOf('}', start);
	if (result < 0)
		result = start;
	return result;
}

QString AbstractAspect::Private::caption() const
{
	QString result = m_caption_spec;
	QRegExp magic("%(.)");
	for(int pos=magic.indexIn(result, 0); pos >= 0; pos=magic.indexIn(result, pos)) {
		QString replacement;
		int length;
		switch(magic.cap(1).at(0).toAscii()) {
			case '%': replacement = "%"; length=2; break;
			case 'n': replacement = m_name; length=2; break;
			case 'c': replacement = m_comment; length=2; break;
			case 't': replacement = m_creation_time.toString(); length=2; break;
			case 'C':
						 length = indexOfMatchingBrace(result, pos) - pos + 1;
						 replacement = m_comment.isEmpty() ? "" : result.mid(pos+3, length-4);
						 break;
		}
		result.replace(pos, length, replacement);
		pos += replacement.size();
	}
	return result;
}

QDateTime AbstractAspect::Private::creationTime() const
{
	return m_creation_time;
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
