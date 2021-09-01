/*
    File                 : GoToDialog.h
    Project              : LabPlot
    Description          : Dialog to provide the cell coordinates to navigate to
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>
class QLineEdit;

class GoToDialog : public QDialog {
	Q_OBJECT

public:
	explicit GoToDialog(QWidget* parent = nullptr);
	~GoToDialog() override;

	int row();
	int column();

private:
	QLineEdit* leRow;
	QLineEdit* leColumn;
};

#endif
