/*
    File                 : GridDialog.h
    Project              : LabPlot
    Description          : dialog for editing the grid properties for the worksheet view
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GRIDDIALOG_H
#define GRIDDIALOG_H

#include <QDialog>
#include "commonfrontend/worksheet/WorksheetView.h"

class QComboBox;
class QSpinBox;
class KColorButton;

class GridDialog : public QDialog {
	Q_OBJECT

public:
	explicit GridDialog(QWidget*);
	~GridDialog() override;
	void save(WorksheetView::GridSettings&);

private:
	QComboBox* cbStyle;
	QSpinBox* sbHorizontalSpacing;
	QSpinBox* sbVerticalSpacing;
	KColorButton* kcbColor;
	QSpinBox* sbOpacity;
};

#endif
