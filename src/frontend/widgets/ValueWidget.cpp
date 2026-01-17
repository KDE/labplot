/*
	File                 : ValueWidget.cpp
	Project              : LabPlot
	Description          : values settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ValueWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "frontend/widgets/TreeViewComboBox.h"
#include "frontend/dockwidgets/BaseDock.h"

/*!
	\class ValueWidget
	\brief Widget for editing the properties of a
   Value object, mostly used in an appropriate dock widget.

	\ingroup frontend
 */
ValueWidget::ValueWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	auto* gridLayout = static_cast<QGridLayout*>(layout());
	cbColumn = new TreeViewComboBox(this);
	gridLayout->addWidget(cbColumn, 2, 2, 1, 1);

	ui.cbType->addItem(i18n("No Values"));
	ui.cbType->addItem(i18n("Frequency"));
	ui.cbType->addItem(i18n("Custom Column"));

	ui.cbPosition->addItem(i18n("Above"));
	ui.cbPosition->addItem(i18n("Below"));
	ui.cbPosition->addItem(i18n("Left"));
	ui.cbPosition->addItem(i18n("Right"));

	// add formats for numeric values
	ui.cbNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	// add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateFormats())
		ui.cbDateTimeFormat->addItem(s, QVariant(s));

	for (const auto& s : AbstractColumn::timeFormats())
		ui.cbDateTimeFormat->addItem(s, QVariant(s));

	for (const auto& s1 : AbstractColumn::dateFormats()) {
		for (const auto& s2 : AbstractColumn::timeFormats())
			ui.cbDateTimeFormat->addItem(s1 + QStringLiteral(" ") + s2, QVariant(s1 + QStringLiteral(" ") + s2));
	}

	ui.cbDateTimeFormat->setEditable(true);

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValueWidget::typeChanged);
	connect(cbColumn, &TreeViewComboBox::currentModelIndexChanged, this, &ValueWidget::columnChanged);
	connect(ui.cbPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValueWidget::positionChanged);
	connect(ui.sbDistance, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &ValueWidget::distanceChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &ValueWidget::rotationChanged);
	connect(ui.sbOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &ValueWidget::opacityChanged);
	connect(ui.cbNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ValueWidget::numericFormatChanged);
	connect(ui.sbPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &ValueWidget::precisionChanged);
	connect(ui.cbDateTimeFormat, &QComboBox::currentTextChanged, this, &ValueWidget::dateTimeFormatChanged);
	connect(ui.lePrefix, &QLineEdit::textChanged, this, &ValueWidget::prefixChanged);
	connect(ui.leSuffix, &QLineEdit::textChanged, this, &ValueWidget::suffixChanged);
	connect(ui.kfrFont, &KFontRequester::fontSelected, this, &ValueWidget::fontChanged);
	connect(ui.kcbColor, &KColorButton::changed, this, &ValueWidget::colorChanged);
}

ValueWidget::~ValueWidget() {
	delete m_aspectModel;
}

void ValueWidget::setValues(const QList<Value*>& values) {
	m_values = values;
	m_value = m_values.first();

	ui.sbDistance->setLocale(QLocale());

	if (!m_aspectModel) {
		m_aspectModel = new AspectTreeModel(m_value->project());
		m_aspectModel->enablePlottableColumnsOnly(true);
		m_aspectModel->enableShowPlotDesignation(true);
	}

	// add center value if position is available
	if (m_value->centerPositionAvailable())
		if (!ui.cbPosition->contains(i18n("Center")))
			ui.cbPosition->addItem(i18n("Center"));

	QList<AspectType> list{AspectType::Folder,
						   AspectType::Workbook,
						   AspectType::Datapicker,
						   AspectType::DatapickerCurve,
						   AspectType::Spreadsheet,
						   AspectType::LiveDataSource,
						   AspectType::Column,
						   AspectType::Worksheet,
						   AspectType::CartesianPlot,
						   AspectType::XYFitCurve,
						   AspectType::XYSmoothCurve,
						   AspectType::Notebook};

	cbColumn->setTopLevelClasses(list);
	m_aspectModel->setSelectableAspects({AspectType::Column});

	cbColumn->setModel(m_aspectModel);

	load();

	connect(m_value, &Value::typeChanged, this, &ValueWidget::valueTypeChanged);
	connect(m_value, &Value::columnChanged, this, &ValueWidget::valueColumnChanged);
	connect(m_value, &Value::positionChanged, this, &ValueWidget::valuePositionChanged);
	connect(m_value, &Value::distanceChanged, this, &ValueWidget::valueDistanceChanged);
	connect(m_value, &Value::opacityChanged, this, &ValueWidget::valueOpacityChanged);
	connect(m_value, &Value::rotationAngleChanged, this, &ValueWidget::valueRotationAngleChanged);
	connect(m_value, &Value::numericFormatChanged, this, &ValueWidget::valueNumericFormatChanged);
	connect(m_value, &Value::precisionChanged, this, &ValueWidget::valuePrecisionChanged);
	connect(m_value, &Value::dateTimeFormatChanged, this, &ValueWidget::valueDateTimeFormatChanged);
	connect(m_value, &Value::prefixChanged, this, &ValueWidget::valuePrefixChanged);
	connect(m_value, &Value::suffixChanged, this, &ValueWidget::valueSuffixChanged);
	connect(m_value, &Value::fontChanged, this, &ValueWidget::valueFontChanged);
	connect(m_value, &Value::colorChanged, this, &ValueWidget::valueColorChanged);
}

//*************************************************************
//******** SLOTs for changes triggered in ValueWidget *********
//*************************************************************
/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void ValueWidget::typeChanged(int index) {
	this->updateWidgets();

	CONDITIONAL_LOCK_RETURN;

	auto valuesType = Value::Type(index);
	for (auto* value : m_values)
		value->setType(valuesType);
}

/*!
  depending on the currently selected values column type (column mode) updates
  the widgets for the values column format, shows/hides the allowed widgets,
  fills the corresponding combobox with the possible entries. Called when the
  values column was changed.
*/
void ValueWidget::updateWidgets() {
	const auto type = Value::Type(ui.cbType->currentIndex());
	bool showValues = (type != Value::Type::NoValues);

	ui.cbPosition->setEnabled(showValues);
	ui.sbDistance->setEnabled(showValues);
	ui.sbRotation->setEnabled(showValues);
	ui.sbOpacity->setEnabled(showValues);
	ui.kfrFont->setEnabled(showValues);
	ui.kcbColor->setEnabled(showValues);

	bool hasInteger = false;
	bool hasNumeric = false;
	bool hasDateTime = false;

	if (type == Value::Type::CustomColumn) {
		ui.lColumn->show();
		cbColumn->show();

		auto* column = static_cast<Column*>(cbColumn->currentModelIndex().internalPointer());
		if (column) {
			if (column->columnMode() == AbstractColumn::ColumnMode::Double)
				hasNumeric = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::Integer || column->columnMode() == AbstractColumn::ColumnMode::BigInt)
				hasInteger = true;
			else if (column->columnMode() == AbstractColumn::ColumnMode::DateTime)
				hasDateTime = true;
		}
	} else {
		ui.lColumn->hide();
		cbColumn->hide();

		if (type == Value::Type::BinEntries)
			hasInteger = true;
	}

	// hide all the format related widgets first and
	// then show only what is required depending of the column mode(s)
	ui.lFormat->hide();
	ui.lNumericFormat->hide();
	ui.cbNumericFormat->hide();
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
	ui.lDateTimeFormat->hide();
	ui.cbDateTimeFormat->hide();

	if (hasNumeric || hasInteger) {
		ui.lFormat->show();
		ui.lNumericFormat->show();
		ui.cbNumericFormat->show();
	}

	// precision is only available for Numeric
	if (hasNumeric) {
		ui.lPrecision->show();
		ui.sbPrecision->show();
	}

	if (hasDateTime) {
		ui.lFormat->show();
		ui.lDateTimeFormat->show();
		ui.cbDateTimeFormat->show();
	}
}

/*!
  called when the custom column for the values was changed.
*/
void ValueWidget::columnChanged(const QModelIndex& index) {
	CONDITIONAL_LOCK_RETURN;

	this->updateWidgets();

	auto* column = static_cast<Column*>(index.internalPointer());
	for (auto* value : m_values)
		value->setColumn(column);
}

void ValueWidget::positionChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* value : m_values)
		value->setPosition(Value::Position(index));
}

void ValueWidget::distanceChanged(double distance) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* value : m_values)
		value->setDistance(Worksheet::convertToSceneUnits(distance, Worksheet::Unit::Point));
}

void ValueWidget::rotationChanged(int angle) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* value : m_values)
		value->setRotationAngle(angle);
}

void ValueWidget::opacityChanged(int v) {
	CONDITIONAL_LOCK_RETURN;

	qreal opacity = static_cast<qreal>(v) / 100.;
	for (auto* value : m_values)
		value->setOpacity(opacity);
}

void ValueWidget::numericFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	char format = ui.cbNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* value : m_values)
		value->setNumericFormat(format);
}

void ValueWidget::precisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* value : m_values)
		value->setPrecision(precision);
}

void ValueWidget::dateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* value : m_values)
		value->setDateTimeFormat(format);
}

void ValueWidget::prefixChanged() {
	CONDITIONAL_LOCK_RETURN;

	QString prefix = ui.lePrefix->text();
	for (auto* value : m_values)
		value->setPrefix(prefix);
}

void ValueWidget::suffixChanged() {
	CONDITIONAL_LOCK_RETURN;

	QString suffix = ui.leSuffix->text();
	for (auto* value : m_values)
		value->setSuffix(suffix);
}

void ValueWidget::fontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	QFont valuesFont = font;
	valuesFont.setPointSizeF(Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Unit::Point));
	for (auto* value : m_values)
		value->setFont(valuesFont);
}

void ValueWidget::colorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* value : m_values)
		value->setColor(color);
}

//*************************************************************
//********* SLOTs for changes triggered in Value *********
//*************************************************************
void ValueWidget::valueTypeChanged(Value::Type type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbType->setCurrentIndex((int)type);
}
void ValueWidget::valueColumnChanged(const AbstractColumn* column) {
	CONDITIONAL_LOCK_RETURN;
	cbColumn->setAspect(column, m_value->columnPath());
}
void ValueWidget::valuePositionChanged(Value::Position position) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPosition->setCurrentIndex((int)position);
}
void ValueWidget::valueDistanceChanged(qreal distance) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbDistance->setValue(Worksheet::convertFromSceneUnits(distance, Worksheet::Unit::Point));
}
void ValueWidget::valueRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}
void ValueWidget::valueOpacityChanged(qreal opacity) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOpacity->setValue(round(opacity * 100.0));
}
void ValueWidget::valueNumericFormatChanged(char format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbNumericFormat->setCurrentIndex(ui.cbNumericFormat->findData(format));
}
void ValueWidget::valuePrecisionChanged(int precision) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPrecision->setValue(precision);
}
void ValueWidget::valueDateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbDateTimeFormat->setCurrentText(format);
}
void ValueWidget::valuePrefixChanged(const QString& prefix) {
	CONDITIONAL_LOCK_RETURN;
	ui.lePrefix->setText(prefix);
}
void ValueWidget::valueSuffixChanged(const QString& suffix) {
	CONDITIONAL_LOCK_RETURN;
	ui.leSuffix->setText(suffix);
}
void ValueWidget::valueFontChanged(QFont font) {
	CONDITIONAL_LOCK_RETURN;
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrFont->setFont(font);
}
void ValueWidget::valueColorChanged(QColor color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbColor->setColor(color);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void ValueWidget::load() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbType->setCurrentIndex((int)m_value->type());
	ui.cbPosition->setCurrentIndex((int)m_value->position());
	ui.sbDistance->setValue(Worksheet::convertFromSceneUnits(m_value->distance(), Worksheet::Unit::Point));
	ui.sbRotation->setValue(m_value->rotationAngle());
	ui.sbOpacity->setValue(round(m_value->opacity()) * 100.0);
	cbColumn->setAspect(m_value->column(), m_value->columnPath());
	this->updateWidgets();
	ui.lePrefix->setText(m_value->prefix());
	ui.leSuffix->setText(m_value->suffix());
	QFont font = m_value->font();
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrFont->setFont(font);
	ui.kcbColor->setColor(m_value->color());
}

void ValueWidget::loadConfig(const KConfigGroup& group) {
	ui.cbType->setCurrentIndex(group.readEntry("ValuesType", (int)m_value->type()));
	ui.cbPosition->setCurrentIndex(group.readEntry("ValuesPosition", (int)m_value->position()));
	ui.sbDistance->setValue(Worksheet::convertFromSceneUnits(group.readEntry("ValuesDistance", m_value->distance()), Worksheet::Unit::Point));
	ui.sbRotation->setValue(group.readEntry("ValuesRotation", m_value->rotationAngle()));
	ui.sbOpacity->setValue(round(group.readEntry("ValuesOpacity", m_value->opacity()) * 100.0));
	this->updateWidgets();
	ui.lePrefix->setText(group.readEntry("ValuesPrefix", m_value->prefix()));
	ui.leSuffix->setText(group.readEntry("ValuesSuffix", m_value->suffix()));
	QFont font = m_value->font();
	font.setPointSizeF(round(Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Unit::Point)));
	ui.kfrFont->setFont(group.readEntry("ValuesFont", font));
	ui.kcbColor->setColor(group.readEntry("ValuesColor", m_value->color()));
}

void ValueWidget::saveConfig(KConfigGroup& group) const {
	group.writeEntry("ValuesType", ui.cbType->currentIndex());
	group.writeEntry("ValuesPosition", ui.cbPosition->currentIndex());
	group.writeEntry("ValuesDistance", Worksheet::convertToSceneUnits(ui.sbDistance->value(), Worksheet::Unit::Point));
	group.writeEntry("ValuesRotation", ui.sbRotation->value());
	group.writeEntry("ValuesOpacity", ui.sbOpacity->value() / 100.0);
	group.writeEntry("ValuesPrefix", ui.lePrefix->text());
	group.writeEntry("ValuesSuffix", ui.leSuffix->text());
	group.writeEntry("ValuesFont", ui.kfrFont->font());
	group.writeEntry("ValuesColor", ui.kcbColor->color());
}
