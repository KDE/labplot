/***************************************************************************
    File                 : MatrixSizeDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Matrix dimensions dialog

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
#ifndef MATRIXSIZEDIALOG_H
#define MATRIXSIZEDIALOG_H

#include <QDialog>

class QGroupBox;
class QPushButton;
class QSpinBox;
class QLineEdit;

//! Matrix dimensions dialog
class MatrixSizeDialog : public QDialog
{
    Q_OBJECT

public:
	//! Constructor
	/**
	 * \param parent parent widget
	 * \param fl window flags
	 */
    MatrixSizeDialog( QWidget* parent = 0, Qt::WFlags fl = 0 );

public slots:
	//! Accept changes and quit
	void accept();
	//! Set the number of columns
	void setColumns(int c);
	//! Set the number of rows
	void setRows(int r);
	//! Set the start and end coordinates
	/**
	 * \param xs start x value
	 * \param xe end x value
	 * \param ys start y value
	 * \param ye end y value
	 */
	void setCoordinates(double xs, double xe, double ys, double ye);

signals:
	//! Emit the new matrix dimensions
	/**
	 * \param rows number of rows
	 * \param cols number of columns
	 */
	void changeDimensions(int rows, int cols);
	//! Emit the new coordinates
	/**
	 * \param fromX start x value
	 * \param toX end x value
	 * \param fromY start y value
	 * \param toY end y value
	 */
	void changeCoordinates(double fromX, double toX, double fromY, double toY);

private:
    QPushButton* buttonOk;
	QPushButton* buttonCancel;
    QGroupBox* groupBox1, *groupBox2;
	QSpinBox *boxCols, *boxRows;
	QLineEdit *boxXStart, *boxYStart, *boxXEnd, *boxYEnd;
};

#endif // MATRIXSIZEDIALOG_H
