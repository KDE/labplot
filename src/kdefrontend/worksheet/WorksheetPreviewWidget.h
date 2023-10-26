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

class WorksheetPreviewWidget : public QWidget {
	Q_OBJECT

public:
	explicit WorksheetPreviewWidget(QWidget* parent = nullptr);
	~WorksheetPreviewWidget() override;

	void setProject(Project*);

private:
	Ui::WorksheetPreviewWidget ui;
	Project* m_project{nullptr};

private Q_SLOTS:
	void aspectAdded(const AbstractAspect*);
	void aspectRemoved(const AbstractAspect*);

Q_SIGNALS:
	void currentAspectChanged(AbstractAspect*);
};

#endif // ifndef WORKSHEETPREVIEWWIDGET_H
