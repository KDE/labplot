/***************************************************************************
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Dock widget for InfoElemnt
	--------------------------------------------------------------------
	Copyright            : (C) 2020 Martin Marmsoler (martin.marmsoler@gmail.com)
	Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#include "InfoElementDock.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "ui_infoelementdock.h"

InfoElementDock::InfoElementDock(QWidget* parent) : BaseDock(parent), ui(new Ui::InfoElementDock) {
	ui->setupUi(this);
	m_leName = ui->leName;
	m_leComment = ui->leComment;

	ui->lePosition->setValidator( new QDoubleValidator(ui->lePosition) );

	//**********************************  Slots **********************************************
	connect(ui->leName, &QLineEdit::textChanged, this, &InfoElementDock::nameChanged);
	connect(ui->leComment, &QLineEdit::textChanged, this, &InfoElementDock::commentChanged);
	connect(ui->chbVisible, &QCheckBox::toggled, this, &InfoElementDock::visibilityChanged);

	connect(ui->sbVerticalLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::xposLineWidthChanged);
	connect(ui->sbConnectionLineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InfoElementDock::connectionLineWidthChanged);
	connect(ui->kcbVerticalLineColor, &KColorButton::changed, this, &InfoElementDock::xposLineColorChanged);
	connect(ui->kcbConnectionLineColor, &KColorButton::changed, this, &InfoElementDock::connectionLineColorChanged);
	connect(ui->chbVerticalLineEnabled, &QCheckBox::toggled, this, &InfoElementDock::xposLineVisibilityChanged);
	connect(ui->cbConnnectionLineEnabled, &QCheckBox::toggled, this, &InfoElementDock::connectionLineVisibilityChanged);
	connect(ui->cbConnectToAnchor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::gluePointChanged);
	connect(ui->cbConnectToCurve, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &InfoElementDock::curveChanged);
	connect(ui->lePosition, &QLineEdit::textChanged, this, &InfoElementDock::positionChanged);
}

void InfoElementDock::setInfoElements(QList<InfoElement*>& list, bool sameParent) {
	const Lock lock(m_initializing);

	m_elements = list;
	m_element = list.first();
	m_aspect = m_element;
	m_sameParent = sameParent;

	ui->lwCurves->clear();
	ui->cbConnectToCurve->clear();

	//if there are more then one info element in the list, disable the name and comment fields
	if (list.size() == 1) {
		ui->lName->setEnabled(true);
		ui->leName->setEnabled(true);
		ui->lComment->setEnabled(true);
		ui->leComment->setEnabled(true);
		ui->leName->setText(m_element->name());
		ui->leComment->setText(m_element->comment());
	} else {
		ui->lName->setEnabled(false);
		ui->leName->setEnabled(false);
		ui->lComment->setEnabled(false);
		ui->leComment->setEnabled(false);
		ui->leName->setText(QString());
		ui->leComment->setText(QString());
	}

	ui->chbVisible->setChecked(m_element->isVisible());

	// disable if not all worksheetelements do not have the same parent (different CartesianPlots),
	// because then the available curves are different
	if (sameParent) {
		QVector<XYCurve*> curves = m_element->plot()->children<XYCurve>();
		for (int i = 0; i < curves.length(); ++i) {
			auto* curve = curves.at(i);
			auto* item = new QListWidgetItem();
			auto* checkBox = new QCheckBox(curve->name());
			connect(checkBox, &QCheckBox::stateChanged, this, &InfoElementDock::curveSelectionChanged);
			ui->lwCurves->addItem(item);
			ui->lwCurves->setItemWidget(item, checkBox);

			for (int i=0; i<m_element->markerPointsCount(); i++) {
				auto* markerCurve = m_element->markerPointAt(i).curve;
				if (markerCurve && markerCurve->name() == curve->name())
					checkBox->setChecked(true);
			}
		}

		for (int i=0; i<m_element->markerPointsCount(); i++) {
			auto* markerCurve = m_element->markerPointAt(i).curve;
			if (markerCurve)
				ui->cbConnectToCurve->addItem(markerCurve->name());
		}
	} else
		ui->lwCurves->setEnabled(false);

	const QString& curveName = m_element->connectionLineCurveName();
	for (int i=0; i< ui->cbConnectToCurve->count(); i++) {
		if (ui->cbConnectToCurve->itemData(i, Qt::DisplayRole).toString().compare(curveName) == 0) {
			ui->cbConnectToCurve->setCurrentIndex(i);
			break;
		}
	}

	if (m_element->isTextLabel()) {
		elementLabelBorderShapeChanged();
		ui->cbConnectToAnchor->setCurrentIndex(m_element->gluePointIndex()+1);
	}

	ui->chbVerticalLineEnabled->setChecked(m_element->xposLineVisible());
	ui->cbConnnectionLineEnabled->setChecked(m_element->connectionLineVisible());
	ui->kcbVerticalLineColor->setColor(m_element->xposLineColor());
	ui->kcbConnectionLineColor->setColor(m_element->connectionLineColor());
	ui->sbVerticalLineWidth->setValue(m_element->xposLineWidth());
	ui->sbConnectionLineWidth->setValue(m_element->connectionLineWidth());

	SET_NUMBER_LOCALE
	ui->lePosition->setText( numberLocale.toString(m_element->position()) );

	initConnections();
}

void InfoElementDock::initConnections() {
	connect( m_element, &InfoElement::visibleChanged,
			 this, &InfoElementDock::elementVisibilityChanged );
	connect( m_element, &InfoElement::connectionLineWidthChanged,
			 this, &InfoElementDock::elementConnectionLineWidthChanged );
	connect( m_element, &InfoElement::connectionLineColorChanged,
			 this, &InfoElementDock::elementConnectionLineColorChanged );
	connect( m_element, &InfoElement::xposLineWidthChanged,
			 this, &InfoElementDock::elementXPosLineWidthChanged );
	connect( m_element, &InfoElement::xposLineColorChanged,
			 this, &InfoElementDock::elementXposLineColorChanged );
	connect( m_element, &InfoElement::xposLineVisibleChanged,
			 this, &InfoElementDock::elementXPosLineVisibleChanged );
	connect (m_element, &InfoElement::gluePointIndexChanged,
			 this, &InfoElementDock::elementGluePointIndexChanged );
	connect (m_element, &InfoElement::connectionLineCurveNameChanged,
			 this, &InfoElementDock::elementConnectionLineCurveChanged );
	connect (m_element, &InfoElement::labelBorderShapeChangedSignal,
			 this, &InfoElementDock::elementLabelBorderShapeChanged );
	connect (m_element, &InfoElement::curveRemoved,
			 this, &InfoElementDock::elementCurveRemoved);
	connect (m_element, &InfoElement::positionChanged,
				this, &InfoElementDock::elementPositionChanged);
}

//*************************************************************
//******* SLOTs for changes triggered in InfoElementDock ******
//*************************************************************
InfoElementDock::~InfoElementDock() {
	delete ui;
}

void InfoElementDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* infoElement : m_elements)
		infoElement->setVisible(state);
}

void InfoElementDock::connectionLineWidthChanged(double width) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineWidth(width);
}

void InfoElementDock::connectionLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineColor(color);
}

void InfoElementDock::xposLineWidthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineWidth(value);
}

void InfoElementDock::xposLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineColor(color);
}

void InfoElementDock::xposLineVisibilityChanged(bool visible) {
	ui->lVerticalLineWidth->setVisible(visible);
	ui->sbVerticalLineWidth->setVisible(visible);
	ui->lVerticalLineColor->setVisible(visible);
	ui->kcbVerticalLineColor->setVisible(visible);

	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setXPosLineVisible(visible);
}

void InfoElementDock::connectionLineVisibilityChanged(bool visible) {
	ui->lConnectionLineWidth->setVisible(visible);
	ui->sbConnectionLineWidth->setVisible(visible);
	ui->lConnectionLineColor->setVisible(visible);
	ui->kcbConnectionLineColor->setVisible(visible);
	ui->lConnectTo->setVisible(visible);
	ui->lConnectToAnchor->setVisible(visible);
	ui->cbConnectToAnchor->setVisible(visible);
	ui->lConnectToCurve->setVisible(visible);
	ui->cbConnectToCurve->setVisible(visible);

	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineVisible(visible);
}

void InfoElementDock::gluePointChanged(int index) {
	if (m_initializing)
		return;

	for (auto* infoElement: m_elements)
		infoElement->setGluePointIndex(index - 1); // index 0 means automatic, which is defined as -1
}

void InfoElementDock::curveChanged() {
	if (m_initializing)
		return;

	QString name = ui->cbConnectToCurve->currentText();
	for (auto* infoElement: m_elements)
		infoElement->setConnectionLineCurveName(name);
}

void InfoElementDock::positionChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double pos = numberLocale.toDouble(value, &ok);
	if (ok) {
		for (auto* element : m_elements)
			element->setPosition(pos);
	}
}

void InfoElementDock::curveSelectionChanged(int state) {
	if (m_initializing || !m_sameParent)
		return;

	//determine the curve for which the selection was changed
	auto* checkBox = static_cast<QCheckBox*>(QObject::sender());
	QString curveName = checkBox->text().remove(QLatin1Char('&'));
	XYCurve* curve = nullptr;
	for (auto* c : m_elements[0]->plot()->children<XYCurve>())
		if (c->name() == curveName) {
			curve = c;
			break;
		}

	//add/remove the changed curve
	if (state == Qt::Checked && curve) {
		for (auto* element : m_elements)
			element->addCurve(curve);
	} else {
		for (auto* element : m_elements)
			element->removeCurve(curve);
	}
}

//***********************************************************
//******* SLOTs for changes triggered in InfoElement ********
//***********************************************************
void InfoElementDock::elementDescriptionChanged(const AbstractAspect* aspect) {
	if (m_element != aspect)
		return;

	const Lock lock(m_initializing);
	if (aspect->name() != ui->leName->text())
		ui->leName->setText(aspect->name());
	else if (aspect->comment() != ui->leComment->text())
		ui->leComment->setText(aspect->comment());
}

void InfoElementDock::elementConnectionLineWidthChanged(const double width) {
	const Lock lock(m_initializing);
	ui->sbConnectionLineWidth->setValue(width);
}

void InfoElementDock::elementConnectionLineColorChanged(const QColor& color) {
	const Lock lock(m_initializing);
	ui->kcbConnectionLineColor->setColor(color);
}

void InfoElementDock::elementXPosLineWidthChanged(const double width) {
	const Lock lock(m_initializing);
	ui->sbVerticalLineWidth->setValue(width);
}

void InfoElementDock::elementXposLineColorChanged(const QColor& color) {
	const Lock lock(m_initializing);
	ui->kcbVerticalLineColor->setColor(color);
}

void InfoElementDock::elementXPosLineVisibleChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->chbVerticalLineEnabled->setChecked(visible);
}

void InfoElementDock::elementConnectionLineVisibleChanged(const bool visible) {
	const Lock lock(m_initializing);
	ui->cbConnnectionLineEnabled->setChecked(visible);
}

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

void InfoElementDock::elementConnectionLineCurveChanged(const QString name) {
	const Lock lock(m_initializing);
	for (int i=0; i< ui->cbConnectToCurve->count(); i++) {
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
	for (int i=0; i < m_element->gluePointsCount(); i++)
		ui->cbConnectToAnchor->addItem(m_element->gluePoint(i).name);
}

void InfoElementDock::elementPositionChanged(double pos) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui->lePosition->setText(numberLocale.toString(pos));
}

void InfoElementDock::elementCurveRemoved(QString name) {
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
