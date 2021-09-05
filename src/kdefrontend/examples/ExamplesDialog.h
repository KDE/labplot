/*
    File                 : ExamplesDialog.h
    Project              : LabPlot
    Description          : dialog showing the available example projects
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXAMPLESDIALOG_H
#define EXAMPLESDIALOG_H

#include <QDialog>

class ExamplesWidget;

class ExamplesDialog : public QDialog {
	Q_OBJECT

public:
	explicit ExamplesDialog(QWidget*);
	~ExamplesDialog() override;

	QString path() const;

private:
	ExamplesWidget* m_examplesWidget;

};

#endif // EXAMPLESDIALOG_H
