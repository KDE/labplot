/*
    File                 : MatrixDock.h
    Project              : LabPlot
    Description          : widget for matrix properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MATRIXDOCK_H
#define MATRIXDOCK_H

#include "backend/matrix/Matrix.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_matrixdock.h"

class Matrix;
class KConfig;

class MatrixDock : public BaseDock {
	Q_OBJECT

public:
	explicit MatrixDock(QWidget*);
	void setMatrices(QList<Matrix*>);

private:
	Ui::MatrixDock ui;
	QList<Matrix*> m_matrixList;
	Matrix* m_matrix{nullptr};

	void load();
	void loadConfig(KConfig&);

private slots:
	//SLOTs for changes triggered in MatrixDock

	void rowCountChanged(int);
	void columnCountChanged(int);

	void xStartChanged();
	void xEndChanged();
	void yStartChanged();
	void yEndChanged();

	void numericFormatChanged(int);
	void precisionChanged(int);
	void headerFormatChanged(int);

	//SLOTs for changes triggered in Matrix
	void matrixXStartChanged(double);
	void matrixXEndChanged(double);
	void matrixYStartChanged(double);
	void matrixYEndChanged(double);

	void matrixRowCountChanged(int);
	void matrixColumnCountChanged(int);

	void matrixNumericFormatChanged(char);
	void matrixPrecisionChanged(int);
	void matrixHeaderFormatChanged(Matrix::HeaderFormat);

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif // MATRIXDOCK_H
