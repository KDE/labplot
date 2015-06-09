/***************************************************************************
    File                 : MAtrixDock.cpp
    Project              : LabPlot
    Description          : widget for matrix properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Alexander Semke (alexander.semke@web.de)

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

#include "MatrixDock.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "backend/matrix/Matrix.h"
#include "kdefrontend/TemplateHandler.h"
#include <QDir>

 /*!
  \class MatrixDock
  \brief Provides a widget for editing the properties of the matrices currently selected in the project explorer.

  \ingroup kdefrontend
*/

MatrixDock::MatrixDock(QWidget* parent): QWidget(parent), m_initializing(false) {
	ui.setupUi(this);

	ui.cbFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbFormat->addItem(i18n("Automatic (g)"), QVariant('g'));
	ui.cbFormat->addItem(i18n("Automatic (G)"), QVariant('G'));

	ui.cbHeader->addItem(i18n("Rows and Columns"));
	ui.cbHeader->addItem(i18n("xy-Values"));
	ui.cbHeader->addItem(i18n("Rows, Columns and xy-Values"));

	//Validators
	ui.kleXStart->setValidator( new QDoubleValidator(ui.kleXStart) );
	ui.kleXEnd->setValidator( new QDoubleValidator(ui.kleXEnd) );
	ui.kleYStart->setValidator( new QDoubleValidator(ui.kleYStart) );
	ui.kleYEnd->setValidator( new QDoubleValidator(ui.kleYEnd) );

	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()));
	connect(ui.kleXStart, SIGNAL(returnPressed()), this, SLOT(xStartChanged()));
	connect(ui.kleXEnd, SIGNAL(returnPressed()), this, SLOT(xEndChanged()));
	connect(ui.kleYStart, SIGNAL(returnPressed()), this, SLOT(yStartChanged()));
	connect(ui.kleYEnd, SIGNAL(returnPressed()), this, SLOT(yEndChanged()));
	connect(ui.cbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(numericFormatChanged(int)));
	connect(ui.sbPrecision, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)));
	connect(ui.cbHeader, SIGNAL(currentIndexChanged(int)), this, SLOT(headerFormatChanged(int)));
	connect(ui.sbColumnCount, SIGNAL(valueChanged(int)), this, SLOT(columnCountChanged(int)));
	connect(ui.sbRowCount, SIGNAL(valueChanged(int)), this, SLOT(rowCountChanged(int)));

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Matrix);
	ui.gridLayout->addWidget(templateHandler, 22, 0, 1, 4);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

/*!

*/
void MatrixDock::setMatrices(QList<Matrix*> list){
	m_initializing = true;
	m_matrixList = list;
	m_matrix = list.first();

	if (list.size()==1){
		ui.leName->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_matrix->name());
		ui.leComment->setText(m_matrix->comment());
	}else{
		//disable the fields "Name" and "Comment" if there are more the one matrix
		ui.leName->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first Matrix in the list, if there are >1 matrixs
	this->load();

	// undo functions
	connect(m_matrix, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(matrixDescriptionChanged(const AbstractAspect*)));

	connect(m_matrix, SIGNAL(xStartChanged(double)), this, SLOT(matrixXStartChanged(double)));
	connect(m_matrix, SIGNAL(xEndChanged(double)), this, SLOT(matrixXEndChanged(double)));
	connect(m_matrix, SIGNAL(yStartChanged(double)), this, SLOT(matrixYStartChanged(double)));
	connect(m_matrix, SIGNAL(yEndChanged(double)), this, SLOT(matrixYEndChanged(double)));

	connect(m_matrix, SIGNAL(numericFormatChanged(char)), this, SLOT(matrixNumericFormatChanged(char)));
	connect(m_matrix, SIGNAL(precisionChanged(int)), this, SLOT(matrixPrecisionChanged(int)));
// 	connect(m_matrix, SIGNAL(rowCountChanged(int)),this, SLOT(matrixRowCountChanged(int)));
// 	connect(m_matrix, SIGNAL(columnCountChanged(int)),this, SLOT(matrixColumnCountChanged(int)));

	m_initializing = false;
}

//*************************************************************
//****** SLOTs for changes triggered in MatrixDock *******
//*************************************************************
void MatrixDock::nameChanged(){
	if (m_initializing)
		return;

	m_matrix->setName(ui.leName->text());
}

void MatrixDock::commentChanged(){
	if (m_initializing)
		return;

	m_matrix->setComment(ui.leComment->text());
}


//mapping to the logical coordinates
void MatrixDock::xStartChanged() {
	if (m_initializing)
		return;

	QString str = ui.kleXStart->text().trimmed();
	if (str.isEmpty()) return;
	double value = str.toDouble();
	foreach(Matrix* matrix, m_matrixList)
		matrix->setXStart(value);
}

void MatrixDock::xEndChanged() {
	if (m_initializing)
		return;

	QString str = ui.kleXEnd->text().trimmed();
	if (str.isEmpty()) return;
	double value = str.toDouble();
	foreach(Matrix* matrix, m_matrixList)
		matrix->setXEnd(value);
}

void MatrixDock::yStartChanged() {
	if (m_initializing)
		return;

	QString str = ui.kleYStart->text().trimmed();
	if (str.isEmpty()) return;
	double value = str.toDouble();
	foreach(Matrix* matrix, m_matrixList)
		matrix->setYStart(value);
}

void MatrixDock::yEndChanged() {
	if (m_initializing)
		return;

	QString str = ui.kleYEnd->text().trimmed();
	if (str.isEmpty()) return;
	double value = str.toDouble();
	foreach(Matrix* matrix, m_matrixList)
		matrix->setYEnd(value);
}

//format
void MatrixDock::numericFormatChanged(int index) {
	if (m_initializing)
		return;

	char format = ui.cbFormat->itemData(index).toChar().toLatin1();
	foreach(Matrix* matrix, m_matrixList)
		matrix->setNumericFormat(format);
}

void MatrixDock::precisionChanged(int precision) {
	if (m_initializing)
		return;

	foreach(Matrix* matrix, m_matrixList)
		matrix->setPrecision(precision);
}

void MatrixDock::headerFormatChanged(int value) {
	if (m_initializing)
		return;

	Matrix::HeaderFormat format = (Matrix::HeaderFormat)value;
	foreach(Matrix* matrix, m_matrixList)
		matrix->setHeaderFormat(format);
}

void MatrixDock::rowCountChanged(int rows){
	if (m_initializing)
		return;

// 	foreach(Matrix* matrix, m_matrixList)
// 		matrix->setRowCount(rows);
}

void MatrixDock::columnCountChanged(int columns){
	if (m_initializing)
		return;

// 	foreach(Matrix* matrix, m_matrixList)
// 		matrix->setColumnCount(columns);
}

//*************************************************************
//******** SLOTs for changes triggered in Matrix *********
//*************************************************************
void MatrixDock::matrixDescriptionChanged(const AbstractAspect* aspect) {
	if (m_matrix != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

//mapping to the logical coordinates
void MatrixDock::matrixXStartChanged(double value) {
	m_initializing = true;
	ui.kleXStart->setText(QString::number(value));
	m_initializing = false;
}

void MatrixDock::matrixXEndChanged(double value) {
	m_initializing = true;
	ui.kleXEnd->setText(QString::number(value));
	m_initializing = false;
}

void MatrixDock::matrixYStartChanged(double value) {
	m_initializing = true;
	ui.kleYStart->setText(QString::number(value));
	m_initializing = false;
}

void MatrixDock::matrixYEndChanged(double value) {
	m_initializing = true;
	ui.kleYEnd->setText(QString::number(value));
	m_initializing = false;
}

//matrix dimensions
void MatrixDock::matrixRowCountChanged(int count) {
	m_initializing = true;
	ui.sbRowCount->setValue(count);
	m_initializing = false;
}

void MatrixDock::matrixColumnCountChanged(int count) {
	m_initializing = true;
	ui.sbColumnCount->setValue(count);
	m_initializing = false;
}

//format
void MatrixDock::matrixNumericFormatChanged(char format) {
	m_initializing = true;
	int index = ui.cbFormat->findData((int)format);
	ui.cbFormat->setCurrentIndex(index);
	m_initializing = false;
}

void MatrixDock::matrixPrecisionChanged(int precision) {
	m_initializing = true;
	ui.sbPrecision->setValue(precision);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void MatrixDock::load() {
	//format
	ui.cbFormat->setCurrentIndex(ui.cbFormat->findData((int)m_matrix->numericFormat()));
	ui.sbPrecision->setValue(m_matrix->precision());
	ui.cbHeader->setCurrentIndex(m_matrix->headerFormat());

	//x-range
	ui.kleXStart->setText(QString::number(m_matrix->xStart()));
	ui.kleXEnd->setText(QString::number(m_matrix->xEnd()));
	ui.sbRowCount->setValue(m_matrix->rowCount());

	//y-range
	ui.kleYStart->setText(QString::number(m_matrix->yStart()));
	ui.kleYEnd->setText(QString::number(m_matrix->yEnd()));
	ui.sbColumnCount->setValue(m_matrix->columnCount());

}

void MatrixDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_matrixList.size();
	if (size>1)
		m_matrix->beginMacro(i18n("%1 matrices: template \"%2\" loaded", size, name));
	else
		m_matrix->beginMacro(i18n("%1: template \"%2\" loaded", m_matrix->name(), name));

	this->loadConfig(config);

	m_matrix->endMacro();
}

/*!
	loads saved matrix properties from \c config.
 */
void MatrixDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group("Matrix");

	//format
	ui.cbFormat->setCurrentIndex( ui.cbFormat->findData(group.readEntry("numericFormat", QString(m_matrix->numericFormat()))) );
	ui.sbPrecision->setValue(group.readEntry("Precision", m_matrix->precision()));
	ui.cbHeader->setCurrentIndex(group.readEntry("HeaderFormat", (int)m_matrix->headerFormat()));

	//x-range
	ui.sbRowCount->setValue(group.readEntry("RowCount", m_matrix->rowCount()));

	//y-range
	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", m_matrix->columnCount()));
}

/*!
	saves matrix properties to \c config.
 */
void MatrixDock::saveConfigAsTemplate(KConfig& config){
	KConfigGroup group = config.group("Matrix");

	//format
	group.writeEntry("NumericFormat", ui.cbFormat->itemData(ui.cbFormat->currentIndex()));
	group.writeEntry("Precision", ui.sbPrecision->value());
	group.writeEntry("HeaderFormat", ui.cbHeader->currentIndex());

	//x-range
	group.writeEntry("ColumnCount", ui.sbColumnCount->value());

	//y-range
	group.writeEntry("RowCount", ui.sbRowCount->value());
	config.sync();
}
