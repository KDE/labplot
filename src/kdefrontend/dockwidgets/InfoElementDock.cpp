/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InfoElementDock.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "ui_infoelementdock.h"

InfoElementDock::InfoElementDock(QWidget* parent)
	: BaseDock(parent)
	, ui(new Ui::InfoElementDock) {
	ui->setupUi(this);
	m_leName = ui->leName;
	m_teComment = ui->teComment;
	m_teComment->setFixedHeight(m_leName->height());

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui->tabTitle);
	m_labelWidget = new LabelWidget(ui->tabTitle);
	hboxLayout->addWidget(m_labelWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// set the current locale
	SET_NUMBER_LOCALE
	ui->sbPosition->setLocale(numberLocale);
	m_labelWidget->updateLocale();

	GuiTools::updatePenStyles(ui->cbConnectionLineStyle, Qt::black);
	GuiTools::updatePenStyles(ui->cbVerticalLineStyle, Qt::black);

	//**********************************  Slots **********************************************
	// general
	connect(ui->leName, &QLineEdit::textChanged, this, &InfoElementDock::nameChanged);
	connect(ui->teComment, &QTextEdit::textChanged, this, &InfoElementDock::commentChanged);
	connect(ui->sbPosition, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &InfoElementDock::positionChanged);
	connect(ui->dateTimeEditPosition, &QDateTimeEdit::dateTimeChanged, this, &InfoElementDock::positionDateTimeChanged);
	connect(ui->cbConnectToCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::curveChanged);
	connect(ui->cbConnectToAnchor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::gluePointChanged);
	connect(ui->cbPlotRanges, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::plotRangeChanged);
	connect(ui->chbVisible, &QCheckBox::toggled, this, &InfoElementDock::visibilityChanged);

	// vertical line
	connect(ui->cbVerticalLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::verticalLineStyleChanged);
	connect(ui->kcbVerticalLineColor, &KColorButton::changed, this, &InfoElementDock::verticalLineColorChanged);
	connect(ui->sbVerticalLineWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &InfoElementDock::verticalLineWidthChanged);
	connect(ui->sbVerticalLineOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &InfoElementDock::verticalLineOpacityChanged);

	// connection line
	connect(ui->cbConnectionLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::connectionLineStyleChanged);
	connect(ui->sbConnectionLineWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &InfoElementDock::connectionLineWidthChanged);
	connect(ui->kcbConnectionLineColor, &KColorButton::changed, this, &InfoElementDock::connectionLineColorChanged);
	connect(ui->sbConnectionLineOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &InfoElementDock::connectionLineOpacityChanged);
}

void InfoElementDock::setInfoElements(QList<InfoElement*> list) {
	const Lock lock(m_initializing);

	m_elements = list;
	m_element = list.first();
	setAspects(list);

	// check if all InfoElements have the same CartesianPlot as Parent
	m_sameParent = true;
	if (!m_elements.isEmpty()) {
		const auto* parent = m_elements.constFirst()->parentAspect();
		for (auto* element : m_elements) {
			if (element->parentAspect() != parent) {
				m_sameParent = false;
				break;
			}
		}
	}

	QList<TextLabel*> labels;
	for (auto* element : list)
		labels.append(element->title());

	m_labelWidget->setLabels(labels);

	ui->lwCurves->clear();
	ui->cbConnectToCurve->clear();

	// if there are more then one info element in the list, disable the name and comment fields
	if (list.size() == 1) {
		ui->lName->setEnabled(true);
		ui->leName->setEnabled(true);
		ui->lComment->setEnabled(true);
		ui->teComment->setEnabled(true);
		ui->leName->setText(m_element->name());
		ui->teComment->setText(m_element->comment());
	} else {
		ui->lName->setEnabled(false);
		ui->leName->setEnabled(false);
		ui->lComment->setEnabled(false);
		ui->teComment->setEnabled(false);
		ui->leName->setText(QString());
		ui->teComment->setText(QString());
	}

	ui->chbVisible->setChecked(m_element->isVisible());

	// disable if not all worksheetelements do not have the same parent (different CartesianPlots),
	// because then the available curves are different
	if (m_sameParent) {
		ui->lwCurves->setEnabled(true);
		ui->cbConnectToCurve->setEnabled(true);

		const auto& curves = m_element->plot()->children<XYCurve>();
		for (const auto* curve : curves) {
			auto* item = new QListWidgetItem();
			auto* checkBox = new QCheckBox(curve->name());
			connect(checkBox, &QCheckBox::toggled, this, &InfoElementDock::curveSelectionChanged);
			ui->lwCurves->addItem(item);
			ui->lwCurves->setItemWidget(item, checkBox);

			for (int i = 0; i < m_element->markerPointsCount(); i++) {
				auto* markerCurve = m_element->markerPointAt(i).curve;
				if (markerCurve && markerCurve->name() == curve->name()) {
					checkBox->setChecked(true);
					ui->cbConnectToCurve->addItem(markerCurve->name());
				}
			}
		}
	} else {
		ui->lwCurves->setEnabled(false);
		ui->cbConnectToCurve->setEnabled(false);
	}

	const QString& curveName = m_element->connectionLineCurveName();
	for (int i = 0; i < ui->cbConnectToCurve->count(); i++) {
		if (ui->cbConnectToCurve->itemData(i, Qt::DisplayRole).toString().compare(curveName) == 0) {
			ui->cbConnectToCurve->setCurrentIndex(i);
			break;
		}
	}

	// possible anchor points
	ui->cbConnectToAnchor->clear();
	ui->cbConnectToAnchor->addItem(i18n("Auto"));
	for (int i = 0; i < m_element->gluePointsCount(); i++)
		ui->cbConnectToAnchor->addItem(m_element->gluePoint(i).name);
	ui->cbConnectToAnchor->setCurrentIndex(m_element->gluePointIndex() + 1);

	ui->cbVerticalLineStyle->setCurrentIndex((int)m_element->verticalLinePen().style());
	ui->kcbVerticalLineColor->setColor(m_element->verticalLinePen().color());
	ui->sbVerticalLineWidth->setValue(Worksheet::convertFromSceneUnits(m_element->verticalLinePen().widthF(), Worksheet::Unit::Point));
	ui->sbVerticalLineOpacity->setValue(round(m_element->verticalLineOpacity() * 100.0));
	GuiTools::updatePenStyles(ui->cbVerticalLineStyle, ui->kcbVerticalLineColor->color());

	ui->cbConnectionLineStyle->setCurrentIndex((int)m_element->connectionLinePen().style());
	ui->kcbConnectionLineColor->setColor(m_element->connectionLinePen().color());
	ui->sbConnectionLineWidth->setValue(Worksheet::convertFromSceneUnits(m_element->connectionLinePen().widthF(), Worksheet::Unit::Point));
	ui->sbConnectionLineOpacity->setValue(round(m_element->connectionLineOpacity() * 100.0));
	GuiTools::updatePenStyles(ui->cbConnectionLineStyle, ui->kcbConnectionLineColor->color());

	SET_NUMBER_LOCALE
	if (m_element->plot()->xRangeFormatDefault() == RangeT::Format::Numeric) {
		ui->sbPosition->setValue(m_element->positionLogical());
		ui->lPosition->show();
		ui->sbPosition->show();
		ui->lPositionDateTime->hide();
		ui->dateTimeEditPosition->hide();
	} else {
		ui->dateTimeEditPosition->setDisplayFormat(m_element->plot()->rangeDateTimeFormat(Dimension::X));
		ui->dateTimeEditPosition->setDateTime(QDateTime::fromMSecsSinceEpoch(m_element->positionLogical()));
		ui->lPosition->hide();
		ui->sbPosition->hide();
		ui->lPositionDateTime->show();
		ui->dateTimeEditPosition->show();
	}

	// connections

	updatePlotRanges(); // needed when loading project

	// general
	connect(m_element, &InfoElement::aspectDescriptionChanged, this, &InfoElementDock::aspectDescriptionChanged);
	connect(m_element, &InfoElement::positionLogicalChanged, this, &InfoElementDock::elementPositionChanged);
	connect(m_element, &InfoElement::gluePointIndexChanged, this, &InfoElementDock::elementGluePointIndexChanged);
	connect(m_element, &InfoElement::connectionLineCurveNameChanged, this, &InfoElementDock::elementConnectionLineCurveChanged);
	connect(m_element, &InfoElement::labelBorderShapeChangedSignal, this, &InfoElementDock::elementLabelBorderShapeChanged);
	connect(m_element, &InfoElement::curveRemoved, this, &InfoElementDock::elementCurveRemoved);
	connect(m_element, &WorksheetElement::plotRangeListChanged, this, &InfoElementDock::updatePlotRanges);
	connect(m_element, &InfoElement::visibleChanged, this, &InfoElementDock::elementVisibilityChanged);

	// vertical line
	connect(m_element, &InfoElement::verticalLinePenChanged, this, &InfoElementDock::elementVerticalLinePenChanged);
	connect(m_element, &InfoElement::verticalLineOpacityChanged, this, &InfoElementDock::elementVerticalLineOpacityChanged);

	// connection line
	connect(m_element, &InfoElement::connectionLinePenChanged, this, &InfoElementDock::elementConnectionLinePenChanged);
	connect(m_element, &InfoElement::connectionLineOpacityChanged, this, &InfoElementDock::elementConnectionLineOpacityChanged);
}

void InfoElementDock::updatePlotRanges() {
	updatePlotRangeList(ui->cbPlotRanges);
}

//*************************************************************
//******* SLOTs for changes triggered in InfoElementDock ******
//*************************************************************
InfoElementDock::~InfoElementDock() {
	delete ui;
}

// general tab
void InfoElementDock::positionChanged(double pos) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	for (auto* element : m_elements)
		element->setPositionLogical(pos);
}

void InfoElementDock::positionDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	for (auto* element : m_elements)
		element->setPositionLogical(value);
}

void InfoElementDock::curveSelectionChanged(bool state) {
	if (m_initializing || !m_sameParent)
		return;

	// determine the curve for which the selection was changed
	auto* checkBox = static_cast<QCheckBox*>(QObject::sender());
	QString curveName = checkBox->text().remove(QLatin1Char('&'));
	XYCurve* curve = nullptr;
	for (auto* c : m_elements[0]->plot()->children<XYCurve>()) {
		if (c->name() == curveName) {
			curve = c;
			break;
		}
	}

	// add/remove the changed curve
	if (state && curve) {
		for (auto* element : m_elements)
			element->addCurve(curve);

		// TODO: add the new curve at the proper index via insertItem();
		ui->cbConnectToCurve->addItem(curveName);
	} else {
		bool macroStarted = false;

		// update the "connect to" combobox
		for (int i = 0; ui->cbConnectToCurve->count(); ++i) {
			if (ui->cbConnectToCurve->itemText(i) == curveName) {
				// removing an entry from combo box automatically triggers
				// the selection of a new time which leads to a selection of
				// a new "connect to"-curve in the InfoElement. To make only
				// one single entry on the undo-stak we need to start a macro here:
				macroStarted = true;
				int size = m_elements.size();
				if (size > 1)
					m_element->beginMacro(i18n("%1 info elements: curve \"%2\" removed", size, curveName));
				else
					m_element->beginMacro(i18n("%1: curve \"%2\" removed", m_element->name(), curveName));

				ui->cbConnectToCurve->removeItem(i);
				break;
			}
		}

		for (auto* element : m_elements)
			element->removeCurve(curve);

		if (macroStarted)
			m_element->endMacro();
	}
}

void InfoElementDock::curveChanged() {
	if (m_initializing)
		return;

	const QString& name = ui->cbConnectToCurve->currentText();
	for (auto* infoElement : m_elements)
		infoElement->setConnectionLineCurveName(name);
}

void InfoElementDock::gluePointChanged(int index) {
	if (m_initializing)
		return;

	for (auto* infoElement : m_elements)
		infoElement->setGluePointIndex(index - 1); // index 0 means automatic, which is defined as -1
}

void InfoElementDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* infoElement : m_elements)
		infoElement->setVisible(state);
}

// vertical line tab
void InfoElementDock::verticalLineStyleChanged(int index) {
	if (index == -1 || m_initializing)
		return;

	const auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* element : m_elements) {
		pen = element->verticalLinePen();
		pen.setStyle(penStyle);
		element->setVerticalLinePen(pen);
	}
}

void InfoElementDock::verticalLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* element : m_elements) {
		pen = element->verticalLinePen();
		pen.setColor(color);
		element->setVerticalLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui->cbVerticalLineStyle, color);
	m_initializing = false;
}

void InfoElementDock::verticalLineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* element : m_elements) {
		pen = element->verticalLinePen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		element->setVerticalLinePen(pen);
	}
}

void InfoElementDock::verticalLineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value / 100.;
	for (auto* element : m_elements)
		element->setVerticalLineOpacity(opacity);
}

// connection line tab
void InfoElementDock::connectionLineStyleChanged(int index) {
	if (index == -1 || m_initializing)
		return;

	const auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* element : m_elements) {
		pen = element->connectionLinePen();
		pen.setStyle(penStyle);
		element->setConnectionLinePen(pen);
	}
}

void InfoElementDock::connectionLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* element : m_elements) {
		pen = element->connectionLinePen();
		pen.setColor(color);
		element->setConnectionLinePen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui->cbConnectionLineStyle, color);
	m_initializing = false;
}

void InfoElementDock::connectionLineWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* element : m_elements) {
		pen = element->connectionLinePen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		element->setConnectionLinePen(pen);
	}
}

void InfoElementDock::connectionLineOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value / 100.;
	for (auto* element : m_elements)
		element->setConnectionLineOpacity(opacity);
}

//***********************************************************
//******* SLOTs for changes triggered in InfoElement ********
//***********************************************************
void InfoElementDock::elementVisibilityChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->chbVisible->setChecked(visible);
}

void InfoElementDock::elementGluePointIndexChanged(const int index) {
	const Lock lock(m_initializing);
	if (index < 0)
		ui->cbConnectToAnchor->setCurrentIndex(0);
	else
		ui->cbConnectToAnchor->setCurrentIndex(index + 1); // automatic label is in same combo box
}

void InfoElementDock::elementConnectionLineCurveChanged(const QString& name) {
	const Lock lock(m_initializing);
	for (int i = 0; i < ui->cbConnectToCurve->count(); i++) {
		if (ui->cbConnectToCurve->itemData(i).toString().compare(name) == 0) {
			ui->cbConnectToCurve->setCurrentIndex(i);
			break;
		}
	}
}

void InfoElementDock::elementLabelBorderShapeChanged() {
	const Lock lock(m_initializing);
	ui->cbConnectToAnchor->clear();
	ui->cbConnectToAnchor->addItem(i18n("Auto"));
	for (int i = 0; i < m_element->gluePointsCount(); i++)
		ui->cbConnectToAnchor->addItem(m_element->gluePoint(i).name);
}

void InfoElementDock::elementPositionChanged(double pos) {
	const Lock lock(m_initializing);
	ui->sbPosition->setValue(pos);
	ui->dateTimeEditPosition->setDateTime(QDateTime::fromMSecsSinceEpoch(pos));
}

void InfoElementDock::elementCurveRemoved(const QString& name) {
	const Lock lock(m_initializing);
	for (int i = 0; i < ui->lwCurves->count(); ++i) {
		auto* item = ui->lwCurves->item(i);
		auto* checkBox = static_cast<QCheckBox*>(ui->lwCurves->itemWidget(item));
		if (checkBox->text() == name) {
			ui->lwCurves->takeItem(i);
			break;
		}
	}
}

// vertical line
void InfoElementDock::elementVerticalLinePenChanged(const QPen& pen) {
	const Lock lock(m_initializing);
	ui->cbVerticalLineStyle->setCurrentIndex((int)pen.style());
	ui->kcbVerticalLineColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui->cbVerticalLineStyle, pen.color());
	ui->sbVerticalLineWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void InfoElementDock::elementVerticalLineOpacityChanged(qreal opacity) {
	const Lock lock(m_initializing);
	ui->sbVerticalLineOpacity->setValue(round(opacity * 100.0));
}

// connection line
void InfoElementDock::elementConnectionLinePenChanged(const QPen& pen) {
	const Lock lock(m_initializing);
	ui->cbConnectionLineStyle->setCurrentIndex((int)pen.style());
	ui->kcbConnectionLineColor->setColor(pen.color());
	GuiTools::updatePenStyles(ui->cbVerticalLineStyle, pen.color());
	ui->sbVerticalLineWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void InfoElementDock::elementConnectionLineOpacityChanged(qreal opacity) {
	const Lock lock(m_initializing);
	ui->sbConnectionLineOpacity->setValue(round(opacity * 100.0));
}
