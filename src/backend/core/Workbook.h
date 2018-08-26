/***************************************************************************
    File                 : Workbook.h
    Project              : LabPlot
    Description          : Aspect providing a container for storing data
				in form of spreadsheets and matrices
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Alexander Semke(alexander.semke@web.de)

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
#ifndef WORKBOOK_H
#define WORKBOOK_H

#include "backend/core/AbstractPart.h"

class Spreadsheet;
class Matrix;
class WorkbookView;
class QXmlStreamWriter;
class XmlStreamReader;

class Workbook : public AbstractPart {
	Q_OBJECT

public:
	explicit Workbook(const QString& name);

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QWidget* view() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	Spreadsheet* currentSpreadsheet() const;
	Matrix* currentMatrix() const;
	void setChildSelectedInView(int index, bool selected);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public slots:
	void childSelected(const AbstractAspect*) override;

private:
	mutable WorkbookView* m_view;

private slots:
	void childDeselected(const AbstractAspect*) override;

signals:
	void requestProjectContextMenu(QMenu*);
	void workbookItemSelected(int);
};

#endif
