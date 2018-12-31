/***************************************************************************
    File                 : AspectPrivate.h
    Project              : LabPlot
    Description          : Private data managed by AbstractAspect.
    --------------------------------------------------------------------
    Copyright            : (C) 2013 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007 by Knut Franke (knut.franke@gmx.de)
    Copyright            : (C) 2007 Tilman Benkert (thzs@gmx.net)

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

#include <QDateTime>
#include <QList>

class AbstractAspect;

class AbstractAspectPrivate {
public:
	explicit AbstractAspectPrivate(AbstractAspect* owner, const QString& name);
	~AbstractAspectPrivate();

	void insertChild(int index, AbstractAspect*);
	int indexOfChild(const AbstractAspect*) const;
	int removeChild(AbstractAspect*);

public:
	QVector<AbstractAspect*> m_children;
	QString m_name;
	QString m_comment;
	QDateTime m_creation_time;
	bool m_hidden{false};
	AbstractAspect* const q;
	AbstractAspect* m_parent{nullptr};
	bool m_undoAware{true};
	bool m_isLoading{false};
};

#endif // ifndef ASPECT_PRIVATE_H
