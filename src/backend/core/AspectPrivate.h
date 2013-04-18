/***************************************************************************
    File                 : AspectPrivate.h
    Project              : SciDAVis
    --------------------------------------------------------------------
	Copyright            : (C) 2013 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2007 by Knut Franke (knut.franke*gmx.de), Tilman Benkert (thzs*gmx.net)
							(replace * with @ in the email addresses)
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
#ifndef ASPECT_PRIVATE_H
#define ASPECT_PRIVATE_H

#include "AbstractAspect.h"

#include <QString>
#include <QDateTime>
#include <QList>

class AbstractAspect::Private {
	public:
		Private(AbstractAspect * owner, const QString &name);
		~Private();

		void insertChild(int index, AbstractAspect* child);
		int indexOfChild(const AbstractAspect *child) const;
		int removeChild(AbstractAspect* child);

		QString caption() const;
		QString uniqueNameFor(const QString &current_name) const;

	public:
		QList<AbstractAspect*> m_children;
		QString m_name, m_comment, m_caption_spec;
		QDateTime m_creation_time;
		bool m_hidden;
		AbstractAspect * m_owner;
		AbstractAspect * m_parent;

	private:
		static int indexOfMatchingBrace(const QString &str, int start);
};

#endif // ifndef ASPECT_PRIVATE_H
