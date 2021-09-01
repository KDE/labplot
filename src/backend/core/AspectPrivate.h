/*
    File                 : AspectPrivate.h
    Project              : LabPlot
    Description          : Private data managed by AbstractAspect.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2013 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2007 Knut Franke (knut.franke@gmx.de)
    SPDX-FileCopyrightText: 2007 Tilman Benkert (thzs@gmx.net)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
