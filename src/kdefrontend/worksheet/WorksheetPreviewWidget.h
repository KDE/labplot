/*
	File                 : WorksheetPreviewWidget.cpp
	Project              : LabPlot
	Description          : A widget showing the preview of all worksheets in the project
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WORKSHEETPREVIEWWIDGET_H
#define WORKSHEETPREVIEWWIDGET_H

#include "ui_worksheetpreviewwidget.h"

class AbstractAspect;
class Project;
class Worksheet;

class QContextMenuEvent;
class QResizeEvent;

class WorksheetPreviewWidget : public QWidget {
	Q_OBJECT

public:
	explicit WorksheetPreviewWidget(QWidget* parent = nullptr);
	~WorksheetPreviewWidget() override;

	void setProject(Project*);

private:
	Ui::WorksheetPreviewWidget ui;
	Project* m_project{nullptr};
	bool m_suppressNavigate{false};

	void addPreview(const Worksheet*, int row = -1) const;
	int indexOfWorksheet(const Worksheet*) const;
	void contextMenuEvent(QContextMenuEvent*) override;
	void resizeEvent(QResizeEvent*) override;

private Q_SLOTS:
	void initPreview();
	void currentChanged(int);
	void aspectAdded(const AbstractAspect*);
	void aspectAboutToBeRemoved(const AbstractAspect*);
	void aspectSelected(const AbstractAspect*);
	void aspectDeselected(const AbstractAspect*);
	void updatePreview();
	void updateText();

Q_SIGNALS:
	void currentAspectChanged(AbstractAspect*);
};

#endif // ifndef WORKSHEETPREVIEWWIDGET_H
