/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InfoElementDock.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/GuiTools.h"
#include "frontend/widgets/LabelWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "ui_infoelementdock.h"

InfoElementDock::InfoElementDock(QWidget* parent)
	: BaseDock(parent)
	, ui(new Ui::InfoElementDock) {
	ui->setupUi(this);
	setPlotRangeCombobox(ui->cbPlotRanges);
	setBaseWidgets(ui->leName, ui->teComment);
	setVisibilityWidgets(ui->chbVisible);

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui->tabTitle);
	m_labelWidget = new LabelWidget(ui->tabTitle);
	hboxLayout->addWidget(m_labelWidget);
	hboxLayout->setContentsMargins(2, 2, 2, 2);
	hboxLayout->setSpacing(2);

	// "Lines"-tab
	auto* layout = static_cast<QGridLayout*>(ui->tabLines->layout());
	m_verticalLineWidget = new LineWidget(ui->tabLines);
	layout->addWidget(m_verticalLineWidget, 1, 0, 1, 3);

	m_connectionLineWidget = new LineWidget(ui->tabLines);
	layout->addWidget(m_connectionLineWidget, 4, 0, 1, 3);

	// set the current locale
	ui->sbPosition->setLocale(QLocale());
	m_labelWidget->updateLocale();
	m_verticalLineWidget->updateLocale();
	m_connectionLineWidget->updateLocale();

	//**********************************  Slots **********************************************
	// general
	connect(ui->sbPosition, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &InfoElementDock::positionChanged);
	connect(ui->dateTimeEditPosition, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &InfoElementDock::positionDateTimeChanged);
	connect(ui->cbConnectToCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::curveChanged);
	connect(ui->cbConnectToAnchor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::gluePointChanged);
}

void InfoElementDock::setInfoElements(QList<InfoElement*> list) {
	CONDITIONAL_LOCK_RETURN;

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
	QList<Line*> verticalLines;
	QList<Line*> connectionLines;
	for (auto* element : list) {
		labels << element->title();
		verticalLines << element->verticalLine();
		connectionLines << element->connectionLine();
	}

	m_labelWidget->setLabels(labels);
	m_verticalLineWidget->setLines(verticalLines);
	m_connectionLineWidget->setLines(connectionLines);

	ui->lwCurves->clear();
	ui->cbConnectToCurve->clear();

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
			for (int i = 0; i < m_element->markerPointsCount(); i++) {
				if (curve->path() == m_element->markerPointAt(i).curvePath) {
					checkBox->setChecked(true);
					ui->cbConnectToCurve->addItem(curve->name());
					break;
				}
			}
			connect(checkBox, &QCheckBox::toggled, this, &InfoElementDock::curveSelectionChanged);
			ui->lwCurves->addItem(item);
			ui->lwCurves->setItemWidget(item, checkBox);
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

	if (m_element->plot()->xRangeFormatDefault() == RangeT::Format::Numeric) {
		ui->sbPosition->setValue(m_element->positionLogical());
		ui->lPosition->show();
		ui->sbPosition->show();
		ui->lPositionDateTime->hide();
		ui->dateTimeEditPosition->hide();
	} else {
		ui->dateTimeEditPosition->setDisplayFormat(m_element->plot()->rangeDateTimeFormat(Dimension::X));
		ui->dateTimeEditPosition->setMSecsSinceEpochUTC(m_element->positionLogical());
		ui->lPosition->hide();
		ui->sbPosition->hide();
		ui->lPositionDateTime->show();
		ui->dateTimeEditPosition->show();
	}

	updatePlotRangeList(); // needed when loading project

	// general
	connect(m_element, &InfoElement::positionLogicalChanged, this, &InfoElementDock::elementPositionChanged);
	connect(m_element, &InfoElement::gluePointIndexChanged, this, &InfoElementDock::elementGluePointIndexChanged);
	connect(m_element, &InfoElement::connectionLineCurveNameChanged, this, &InfoElementDock::elementConnectionLineCurveChanged);
	connect(m_element, &InfoElement::labelBorderShapeChangedSignal, this, &InfoElementDock::elementLabelBorderShapeChanged);
	connect(m_element, &InfoElement::curveRemoved, this, &InfoElementDock::elementCurveRemoved);
}

void InfoElementDock::retranslateUi() {
}

//*************************************************************
//******* SLOTs for changes triggered in InfoElementDock ******
//*************************************************************
InfoElementDock::~InfoElementDock() {
	delete ui;
}

// general tab
void InfoElementDock::positionChanged(double pos) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* element : m_elements)
		element->setPositionLogical(pos);
}

void InfoElementDock::positionDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements)
		element->setPositionLogical(value);
}

void InfoElementDock::curveSelectionChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	if (!m_sameParent)
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
	if (enabled && curve) {
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

		if (curve) {
			for (auto* element : m_elements)
				element->removeCurve(curve);
		}

		if (macroStarted)
			m_element->endMacro();
	}
}

void InfoElementDock::curveChanged() {
	CONDITIONAL_LOCK_RETURN;

	const QString& name = ui->cbConnectToCurve->currentText();
	for (auto* infoElement : m_elements)
		infoElement->setConnectionLineCurveName(name);
}

void InfoElementDock::gluePointChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* infoElement : m_elements)
		infoElement->setGluePointIndex(index - 1); // index 0 means automatic, which is defined as -1
}

//***********************************************************
//******* SLOTs for changes triggered in InfoElement ********
//***********************************************************
void InfoElementDock::elementGluePointIndexChanged(const int index) {
	CONDITIONAL_LOCK_RETURN;
	if (index < 0)
		ui->cbConnectToAnchor->setCurrentIndex(0);
	else
		ui->cbConnectToAnchor->setCurrentIndex(index + 1); // automatic label is in same combo box
}

void InfoElementDock::elementConnectionLineCurveChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	for (int i = 0; i < ui->cbConnectToCurve->count(); i++) {
		if (ui->cbConnectToCurve->itemData(i).toString().compare(name) == 0) {
			ui->cbConnectToCurve->setCurrentIndex(i);
			break;
		}
	}
}

void InfoElementDock::elementLabelBorderShapeChanged() {
	CONDITIONAL_LOCK_RETURN;
	ui->cbConnectToAnchor->clear();
	ui->cbConnectToAnchor->addItem(i18n("Auto"));
	for (int i = 0; i < m_element->gluePointsCount(); i++)
		ui->cbConnectToAnchor->addItem(m_element->gluePoint(i).name);
}

void InfoElementDock::elementPositionChanged(double pos) {
	CONDITIONAL_LOCK_RETURN;
	ui->sbPosition->setValue(pos);
	ui->dateTimeEditPosition->setMSecsSinceEpochUTC(pos);
}

void InfoElementDock::elementCurveRemoved(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	for (int i = 0; i < ui->lwCurves->count(); ++i) {
		auto* item = ui->lwCurves->item(i);
		auto* checkBox = static_cast<QCheckBox*>(ui->lwCurves->itemWidget(item));
		if (checkBox->text() == name) {
			ui->lwCurves->takeItem(i);
			break;
		}
	}
}
