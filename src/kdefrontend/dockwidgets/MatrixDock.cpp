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
#include "kdefrontend/TemplateHandler.h"

#include <KConfig>

#include <QDir>

 /*!
  \class MatrixDock
  \brief Provides a widget for editing the properties of the matrices currently selected in the project explorer.

  \ingroup kdefrontend
*/
MatrixDock::MatrixDock(QWidget* parent): BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	ui.cbFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbFormat->addItem(i18n("Automatic (g)"), QVariant('g'));
	ui.cbFormat->addItem(i18n("Automatic (G)"), QVariant('G'));

	ui.cbHeader->addItem(i18n("Rows and Columns"));
	ui.cbHeader->addItem(i18n("xy-Values"));
	ui.cbHeader->addItem(i18n("Rows, Columns and xy-Values"));

	//Validators
	ui.leXStart->setValidator( new QDoubleValidator(ui.leXStart) );
	ui.leXEnd->setValidator( new QDoubleValidator(ui.leXEnd) );
	ui.leYStart->setValidator( new QDoubleValidator(ui.leYStart) );
	ui.leYEnd->setValidator( new QDoubleValidator(ui.leYEnd) );

	connect(ui.leName, &QLineEdit::textChanged, this, &MatrixDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &MatrixDock::commentChanged);
	connect(ui.sbColumnCount, SIGNAL(valueChanged(int)), this, SLOT(columnCountChanged(int)));
	connect(ui.sbRowCount, SIGNAL(valueChanged(int)), this, SLOT(rowCountChanged(int)));
	connect(ui.leXStart, SIGNAL(returnPressed()), this, SLOT(xStartChanged()));
	connect(ui.leXEnd, SIGNAL(returnPressed()), this, SLOT(xEndChanged()));
	connect(ui.leYStart, SIGNAL(returnPressed()), this, SLOT(yStartChanged()));
	connect(ui.leYEnd, SIGNAL(returnPressed()), this, SLOT(yEndChanged()));
	connect(ui.cbFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(numericFormatChanged(int)));
	connect(ui.sbPrecision, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)));
	connect(ui.cbHeader, SIGNAL(currentIndexChanged(int)), this, SLOT(headerFormatChanged(int)));

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Matrix);
	ui.gridLayout->addWidget(templateHandler, 22, 0, 1, 4);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void MatrixDock::setMatrices(QList<Matrix*> list) {
	m_initializing = true;
	m_matrixList = list;
	m_matrix = list.first();
	m_aspect = list.first();

	if (list.size() == 1) {
		ui.leName->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_matrix->name());
		ui.teComment->setText(m_matrix->comment());
	} else {
		//disable these fields if there is more than one matrix
		ui.leName->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first Matrix in the list, if there are >1 matrixs
	this->load();

	// undo functions
	connect(m_matrix, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(matrixDescriptionChanged(const AbstractAspect*)));

	connect(m_matrix, SIGNAL(rowCountChanged(int)), this, SLOT(matrixRowCountChanged(int)));
	connect(m_matrix, SIGNAL(columnCountChanged(int)), this, SLOT(matrixColumnCountChanged(int)));

	connect(m_matrix, SIGNAL(xStartChanged(double)), this, SLOT(matrixXStartChanged(double)));
	connect(m_matrix, SIGNAL(xEndChanged(double)), this, SLOT(matrixXEndChanged(double)));
	connect(m_matrix, SIGNAL(yStartChanged(double)), this, SLOT(matrixYStartChanged(double)));
	connect(m_matrix, SIGNAL(yEndChanged(double)), this, SLOT(matrixYEndChanged(double)));

	connect(m_matrix, SIGNAL(numericFormatChanged(char)), this, SLOT(matrixNumericFormatChanged(char)));
	connect(m_matrix, SIGNAL(precisionChanged(int)), this, SLOT(matrixPrecisionChanged(int)));
	connect(m_matrix, SIGNAL(headerFormatChanged(Matrix::HeaderFormat)), this, SLOT(matrixHeaderFormatChanged(Matrix::HeaderFormat)));

	m_initializing = false;
}

//*************************************************************
//****** SLOTs for changes triggered in MatrixDock *******
//*************************************************************
//mapping to the logical coordinates
void MatrixDock::xStartChanged() {
	if (m_initializing)
		return;

	QString str = ui.leXStart->text().trimmed();
	if (str.isEmpty()) return;
	bool ok;
	SET_NUMBER_LOCALE
	const double value{ numberLocale.toDouble(str, &ok) };
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setXStart(value);
	}
}

void MatrixDock::xEndChanged() {
	if (m_initializing)
		return;

	QString str = ui.leXEnd->text().trimmed();
	if (str.isEmpty()) return;
	bool ok;
	SET_NUMBER_LOCALE
	const double value{ numberLocale.toDouble(str, &ok) };
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setXEnd(value);
	}
}

void MatrixDock::yStartChanged() {
	if (m_initializing)
		return;

	QString str = ui.leYStart->text().trimmed();
	if (str.isEmpty()) return;
	bool ok;
	SET_NUMBER_LOCALE
	const double value{ numberLocale.toDouble(str, &ok) };
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setYStart(value);
	}
}

void MatrixDock::yEndChanged() {
	if (m_initializing)
		return;

	QString str = ui.leYEnd->text().trimmed();
	if (str.isEmpty()) return;
	bool ok;
	SET_NUMBER_LOCALE
	const double value{ numberLocale.toDouble(str, &ok) };
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setYEnd(value);
	}
}

//format
void MatrixDock::numericFormatChanged(int index) {
	if (m_initializing)
		return;

	char format = ui.cbFormat->itemData(index).toChar().toLatin1();
	for (auto* matrix : m_matrixList)
		matrix->setNumericFormat(format);
}

void MatrixDock::precisionChanged(int precision) {
	if (m_initializing)
		return;

	for (auto* matrix : m_matrixList)
		matrix->setPrecision(precision);
}

void MatrixDock::headerFormatChanged(int value) {
	if (m_initializing)
		return;

	auto format = (Matrix::HeaderFormat)value;
	for (auto* matrix : m_matrixList)
		matrix->setHeaderFormat(format);
}

void MatrixDock::rowCountChanged(int rows) {
	if (m_initializing)
		return;

	for (auto* matrix : m_matrixList)
		matrix->setRowCount(rows);
}

void MatrixDock::columnCountChanged(int columns) {
	if (m_initializing)
		return;

	for (auto* matrix : m_matrixList)
		matrix->setColumnCount(columns);
}

//*************************************************************
//******** SLOTs for changes triggered in Matrix *********
//*************************************************************
void MatrixDock::matrixDescriptionChanged(const AbstractAspect* aspect) {
	if (m_matrix != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.teComment->text())
		ui.teComment->setText(aspect->comment());

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

//mapping to the logical coordinates
void MatrixDock::matrixXStartChanged(double value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leXStart->setText(numberLocale.toString(value));
	m_initializing = false;
}

void MatrixDock::matrixXEndChanged(double value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leXEnd->setText(numberLocale.toString(value));
	m_initializing = false;
}

void MatrixDock::matrixYStartChanged(double value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leYStart->setText(numberLocale.toString(value));
	m_initializing = false;
}

void MatrixDock::matrixYEndChanged(double value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leYEnd->setText(numberLocale.toString(value));
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

void MatrixDock::matrixHeaderFormatChanged(Matrix::HeaderFormat format) {
	m_initializing = true;
	ui.cbHeader->setCurrentIndex((int)format);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void MatrixDock::load() {
	//matrix dimensions
	ui.sbRowCount->setValue(m_matrix->rowCount());
	ui.sbColumnCount->setValue(m_matrix->columnCount());

	//mapping to the logical coordinates
	SET_NUMBER_LOCALE
	ui.leXStart->setText(numberLocale.toString(m_matrix->xStart()));
	ui.leXEnd->setText(numberLocale.toString(m_matrix->xEnd()));
	ui.leYStart->setText(numberLocale.toString(m_matrix->yStart()));
	ui.leYEnd->setText(numberLocale.toString(m_matrix->yEnd()));

	//format
	ui.cbFormat->setCurrentIndex(ui.cbFormat->findData((int)m_matrix->numericFormat()));
	ui.sbPrecision->setValue(m_matrix->precision());
	ui.cbHeader->setCurrentIndex(static_cast<int>(m_matrix->headerFormat()));
}

void MatrixDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	const int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	const int size = m_matrixList.size();
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
void MatrixDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("Matrix");

	//matrix dimensions
	ui.sbRowCount->setValue(group.readEntry("RowCount", m_matrix->rowCount()));
	ui.sbColumnCount->setValue(group.readEntry("ColumnCount", m_matrix->columnCount()));

	//mapping to the logical coordinates
	SET_NUMBER_LOCALE
	ui.leXStart->setText( numberLocale.toString(group.readEntry("XStart", m_matrix->xStart())) );
	ui.leXEnd->setText( numberLocale.toString(group.readEntry("XEnd", m_matrix->xEnd())) );
	ui.leYStart->setText( numberLocale.toString(group.readEntry("YStart", m_matrix->yStart())) );
	ui.leYEnd->setText( numberLocale.toString(group.readEntry("YEnd", m_matrix->yEnd())) );

	//format
	ui.cbFormat->setCurrentIndex( ui.cbFormat->findData(group.readEntry("NumericFormat", QString(m_matrix->numericFormat()))) );
	ui.sbPrecision->setValue(group.readEntry("Precision", m_matrix->precision()));
	ui.cbHeader->setCurrentIndex(group.readEntry("HeaderFormat", (int)m_matrix->headerFormat()));
}

/*!
	saves matrix properties to \c config.
 */
void MatrixDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Matrix");

	//matrix dimensions
	group.writeEntry("RowCount", ui.sbRowCount->value());
	group.writeEntry("ColumnCount", ui.sbColumnCount->value());

	//mapping to the logical coordinates
	SET_NUMBER_LOCALE
	group.writeEntry("XStart", numberLocale.toDouble(ui.leXStart->text()) );
	group.writeEntry("XEnd", numberLocale.toDouble(ui.leXEnd->text()) );
	group.writeEntry("YStart", numberLocale.toDouble(ui.leYStart->text()) );
	group.writeEntry("YEnd", numberLocale.toDouble(ui.leYEnd->text()) );

	//format
	group.writeEntry("NumericFormat", ui.cbFormat->itemData(ui.cbFormat->currentIndex()));
	group.writeEntry("Precision", ui.sbPrecision->value());
	group.writeEntry("HeaderFormat", ui.cbHeader->currentIndex());

	config.sync();
}
