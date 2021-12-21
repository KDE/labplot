/*
    File                 : Workbook.h
    Project              : LabPlot
    Description          : Aspect providing a container for storing data
				in form of spreadsheets and matrices
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	QVector<AspectType> pasteTypes() const override;
	void processDropEvent(const QVector<quintptr>&) override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

public Q_SLOTS:
	void childSelected(const AbstractAspect*) override;

private:
	mutable WorkbookView* m_view{nullptr};

private Q_SLOTS:
	void childDeselected(const AbstractAspect*) override;

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
	void workbookItemSelected(int);
};

#endif
