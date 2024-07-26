/*
	File                 : MAtrixDock.cpp
	Project              : LabPlot
	Description          : widget for matrix properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MatrixDock.h"
#include "commonfrontend/matrix/MatrixView.h"
#include "kdefrontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDir>

/*!
 \class MatrixDock
 \brief Provides a widget for editing the properties of the matrices currently selected in the project explorer.

 \ingroup kdefrontend
*/
MatrixDock::MatrixDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	ui.cbFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbFormat->addItem(i18n("Automatic (g)"), QVariant('g'));
	ui.cbFormat->addItem(i18n("Automatic (G)"), QVariant('G'));

	ui.cbHeader->addItem(i18n("Rows and Columns"));
	ui.cbHeader->addItem(i18n("xy-Values"));
	ui.cbHeader->addItem(i18n("Rows, Columns and xy-Values"));

	// Validators
	ui.leXStart->setValidator(new QDoubleValidator(ui.leXStart));
	ui.leXEnd->setValidator(new QDoubleValidator(ui.leXEnd));
	ui.leYStart->setValidator(new QDoubleValidator(ui.leYStart));
	ui.leYEnd->setValidator(new QDoubleValidator(ui.leYEnd));

	connect(ui.sbColumnCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &MatrixDock::columnCountChanged);
	connect(ui.sbRowCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &MatrixDock::rowCountChanged);
	connect(ui.leXStart, &QLineEdit::textChanged, this, &MatrixDock::xStartChanged);
	connect(ui.leXEnd, &QLineEdit::textChanged, this, &MatrixDock::xEndChanged);
	connect(ui.leYStart, &QLineEdit::textChanged, this, &MatrixDock::yStartChanged);
	connect(ui.leYEnd, &QLineEdit::textChanged, this, &MatrixDock::yEndChanged);
	connect(ui.cbFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MatrixDock::numericFormatChanged);
	connect(ui.sbPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &MatrixDock::precisionChanged);
	connect(ui.cbHeader, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MatrixDock::headerFormatChanged);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Matrix"));
	ui.gridLayout->addWidget(templateHandler, 22, 0, 1, 4);
	// templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &MatrixDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &MatrixDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &MatrixDock::info);
}

void MatrixDock::setMatrices(QList<Matrix*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_matrixList = list;
	m_matrix = list.first();
	setAspects(std::move(list));

	// show the properties of the first Matrix in the list, if there are >1 matrixs
	this->load();

	// undo functions

	connect(m_matrix, &Matrix::rowCountChanged, this, &MatrixDock::matrixRowCountChanged);
	connect(m_matrix, &Matrix::columnCountChanged, this, &MatrixDock::matrixColumnCountChanged);

	connect(m_matrix, &Matrix::xStartChanged, this, &MatrixDock::matrixXStartChanged);
	connect(m_matrix, &Matrix::xEndChanged, this, &MatrixDock::matrixXEndChanged);
	connect(m_matrix, &Matrix::yStartChanged, this, &MatrixDock::matrixYStartChanged);
	connect(m_matrix, &Matrix::yEndChanged, this, &MatrixDock::matrixYEndChanged);

	connect(m_matrix, &Matrix::numericFormatChanged, this, &MatrixDock::matrixNumericFormatChanged);
	connect(m_matrix, &Matrix::precisionChanged, this, &MatrixDock::matrixPrecisionChanged);
	connect(m_matrix, &Matrix::headerFormatChanged, this, &MatrixDock::matrixHeaderFormatChanged);
}

//*************************************************************
//****** SLOTs for changes triggered in MatrixDock *******
//*************************************************************
// mapping to the logical coordinates
void MatrixDock::xStartChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

	QString str = text.trimmed();
	if (str.isEmpty())
		return;
	bool ok;
	const double value{QLocale().toDouble(str, &ok)};
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setXStart(value);
	}
}

void MatrixDock::xEndChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

	QString str = text.trimmed();
	if (str.isEmpty())
		return;
	bool ok;
	const double value{QLocale().toDouble(str, &ok)};
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setXEnd(value);
	}
}

void MatrixDock::yStartChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

	QString str = text.trimmed();
	if (str.isEmpty())
		return;
	bool ok;
	const double value{QLocale().toDouble(str, &ok)};
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setYStart(value);
	}
}

void MatrixDock::yEndChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

	QString str = text.trimmed();
	if (str.isEmpty())
		return;
	bool ok;
	const double value{QLocale().toDouble(str, &ok)};
	if (ok) {
		for (auto* matrix : m_matrixList)
			matrix->setYEnd(value);
	}
}

// format
void MatrixDock::numericFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	char format = ui.cbFormat->itemData(index).toChar().toLatin1();
	for (auto* matrix : m_matrixList)
		matrix->setNumericFormat(format);
}

void MatrixDock::precisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* matrix : m_matrixList)
		matrix->setPrecision(precision);
}

void MatrixDock::headerFormatChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	auto format = static_cast<Matrix::HeaderFormat>(value);
	for (auto* matrix : m_matrixList)
		matrix->setHeaderFormat(format);
}

void MatrixDock::rowCountChanged(int rows) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* matrix : m_matrixList)
		matrix->setRowCount(rows);
}

void MatrixDock::columnCountChanged(int columns) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* matrix : m_matrixList)
		matrix->setColumnCount(columns);
}

//*************************************************************
//******** SLOTs for changes triggered in Matrix *********
//*************************************************************
// matrix dimensions
void MatrixDock::matrixRowCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRowCount->setValue(count);
}

void MatrixDock::matrixColumnCountChanged(int count) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbColumnCount->setValue(count);
}

// mapping to the logical coordinates
void MatrixDock::matrixXStartChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leXStart->setText(QLocale().toString(value));
}

void MatrixDock::matrixXEndChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leXEnd->setText(QLocale().toString(value));
}

void MatrixDock::matrixYStartChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leYStart->setText(QLocale().toString(value));
}

void MatrixDock::matrixYEndChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leYEnd->setText(QLocale().toString(value));
}

// format
void MatrixDock::matrixNumericFormatChanged(char format) {
	CONDITIONAL_LOCK_RETURN;
	int index = ui.cbFormat->findData((int)format);
	ui.cbFormat->setCurrentIndex(index);
}

void MatrixDock::matrixPrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPrecision->setValue(precision);
}

void MatrixDock::matrixHeaderFormatChanged(Matrix::HeaderFormat format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHeader->setCurrentIndex((int)format);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void MatrixDock::load() {
	// matrix dimensions
	ui.sbRowCount->setValue(m_matrix->rowCount());
	ui.sbColumnCount->setValue(m_matrix->columnCount());

	// mapping to the logical coordinates
	const auto numberLocale = QLocale();
	ui.leXStart->setText(numberLocale.toString(m_matrix->xStart()));
	ui.leXEnd->setText(numberLocale.toString(m_matrix->xEnd()));
	ui.leYStart->setText(numberLocale.toString(m_matrix->yStart()));
	ui.leYEnd->setText(numberLocale.toString(m_matrix->yEnd()));

	// format
	ui.cbFormat->setCurrentIndex(ui.cbFormat->findData((int)m_matrix->numericFormat()));
	ui.sbPrecision->setValue(m_matrix->precision());
	ui.cbHeader->setCurrentIndex(static_cast<int>(m_matrix->headerFormat()));
}

void MatrixDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
	const int size = m_matrixList.size();
	if (size > 1)
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
	KConfigGroup group = config.group(QStringLiteral("Matrix"));

	// matrix dimensions
	ui.sbRowCount->setValue(group.readEntry(QStringLiteral("RowCount"), m_matrix->rowCount()));
	ui.sbColumnCount->setValue(group.readEntry(QStringLiteral("ColumnCount"), m_matrix->columnCount()));

	// mapping to the logical coordinates
	const auto numberLocale = QLocale();
	ui.leXStart->setText(numberLocale.toString(group.readEntry(QStringLiteral("XStart"), m_matrix->xStart())));
	ui.leXEnd->setText(numberLocale.toString(group.readEntry(QStringLiteral("XEnd"), m_matrix->xEnd())));
	ui.leYStart->setText(numberLocale.toString(group.readEntry(QStringLiteral("YStart"), m_matrix->yStart())));
	ui.leYEnd->setText(numberLocale.toString(group.readEntry(QStringLiteral("YEnd"), m_matrix->yEnd())));

	// format
	ui.cbFormat->setCurrentIndex(ui.cbFormat->findData(group.readEntry(QStringLiteral("NumericFormat"), (int)(m_matrix->numericFormat()))));
	ui.sbPrecision->setValue(group.readEntry(QStringLiteral("Precision"), m_matrix->precision()));
	ui.cbHeader->setCurrentIndex(group.readEntry(QStringLiteral("HeaderFormat"), (int)m_matrix->headerFormat()));
}

/*!
	saves matrix properties to \c config.
 */
void MatrixDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Matrix"));

	// matrix dimensions
	group.writeEntry(QStringLiteral("RowCount"), ui.sbRowCount->value());
	group.writeEntry(QStringLiteral("ColumnCount"), ui.sbColumnCount->value());

	// mapping to the logical coordinates
	const auto numberLocale = QLocale();
	group.writeEntry(QStringLiteral("XStart"), numberLocale.toDouble(ui.leXStart->text()));
	group.writeEntry(QStringLiteral("XEnd"), numberLocale.toDouble(ui.leXEnd->text()));
	group.writeEntry(QStringLiteral("YStart"), numberLocale.toDouble(ui.leYStart->text()));
	group.writeEntry(QStringLiteral("YEnd"), numberLocale.toDouble(ui.leYEnd->text()));

	// format
	group.writeEntry(QStringLiteral("NumericFormat"), ui.cbFormat->itemData(ui.cbFormat->currentIndex()));
	group.writeEntry(QStringLiteral("Precision"), ui.sbPrecision->value());
	group.writeEntry(QStringLiteral("HeaderFormat"), ui.cbHeader->currentIndex());

	config.sync();
}
