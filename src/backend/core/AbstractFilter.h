/***************************************************************************
    File                 : AbstractFilter.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007,2008 by Knut Franke, Tilman Benkert
    Email (use @ for *)  : knut.franke*gmx.de, thzs*gmx.net
    Description          : Base class for all analysis operations.

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
#ifndef ABSTRACT_FILTER_H
#define ABSTRACT_FILTER_H

#include "AbstractAspect.h"
#include <QVector>

class AbstractColumn;

class AbstractFilter : public AbstractAspect {
	Q_OBJECT

public:
	explicit AbstractFilter(const QString& name) : AbstractAspect(name) {}
	~AbstractFilter() override {}

	virtual int inputCount() const = 0;
	virtual int outputCount() const = 0;
	int highestConnectedInput() const;
	bool input(int port, const AbstractColumn* source);
	bool input(const AbstractFilter* sources);
	const AbstractColumn* input(int port) const;
	virtual QString inputLabel(int port) const;
	virtual AbstractColumn* output(int port = 0) = 0;
	virtual const AbstractColumn* output(int port = 0) const = 0;

	int portIndexOf(const AbstractColumn* column);

protected:
	virtual bool inputAcceptable(int port, const AbstractColumn* source);
	virtual void inputAboutToBeDisconnected(const AbstractColumn* source);

protected slots:
	virtual void inputDescriptionAboutToChange(const AbstractColumn* source);
	void inputDescriptionAboutToChange(const AbstractAspect* aspect);
	virtual void inputDescriptionChanged(const AbstractColumn* source);
	void inputDescriptionChanged(const AbstractAspect* aspect);
	virtual void inputPlotDesignationAboutToChange(const AbstractColumn* source);

	virtual void inputPlotDesignationChanged(const AbstractColumn* source);
	virtual void inputModeAboutToChange(const AbstractColumn* source);
	virtual void inputModeChanged(const AbstractColumn* source);
	virtual void inputDataAboutToChange(const AbstractColumn* source);
	virtual void inputDataChanged(const AbstractColumn* source);

	virtual void inputRowsAboutToBeInserted(const AbstractColumn* source, int before, int count) {
		Q_UNUSED(source); Q_UNUSED(before); Q_UNUSED(count);
	}
	virtual void inputRowsInserted(const AbstractColumn* source, int before, int count) {
		Q_UNUSED(source); Q_UNUSED(before); Q_UNUSED(count);
	}
	virtual void inputRowsAboutToBeRemoved(const AbstractColumn* source, int first, int count) {
		Q_UNUSED(source); Q_UNUSED(first); Q_UNUSED(count);
	}
	virtual void inputRowsRemoved(const AbstractColumn* source, int first, int count) {
		Q_UNUSED(source); Q_UNUSED(first); Q_UNUSED(count);
	}
	virtual void inputMaskingAboutToChange(const AbstractColumn* source) {
		Q_UNUSED(source);
	}
	virtual void inputMaskingChanged(const AbstractColumn* source) {
		Q_UNUSED(source);
	}
	void inputAboutToBeDestroyed(const AbstractColumn* source) {
		input(portIndexOf(source), nullptr);
	}

protected:
	QVector<const AbstractColumn*> m_inputs;
};

#endif // ifndef ABSTRACT_FILTER_H

