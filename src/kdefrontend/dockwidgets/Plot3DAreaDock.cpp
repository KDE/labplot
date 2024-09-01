#include "Plot3DAreaDock.h"
#include "qwidget.h"

#include <NumberSpinBox.h>
#include <QCheckBox>
Plot3DAreaDock::Plot3DAreaDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);
	this->retranslateUi();

	connect(ui.sbLeft, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()));
	connect(ui.sbTop, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()));
	connect(ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()));
	connect(ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()));
}

void Plot3DAreaDock::setPlots(const QList<Plot3DArea*>& m_plots) {
	this->m_plots = m_plots;
	m_plot = m_plots.first();
	Q_ASSERT(m_plot);

	Worksheet* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w) {
		bool b = (w->layout() == Worksheet::Layout::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, &Worksheet::layoutChanged, this, &Plot3DAreaDock::onGeometryChanged);
	}
	connect(m_plot, SIGNAL(rectChanged(const QRectF&)), this, SLOT(rectChanged(const QRectF&)));
}

void Plot3DAreaDock::updateUiVisibility() {
}

void Plot3DAreaDock::geometryChanged() {
	const float x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), Worksheet::Unit::Centimeter);
	const float y = Worksheet::convertToSceneUnits(ui.sbTop->value(), Worksheet::Unit::Centimeter);
	const float w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Unit::Centimeter);
	const float h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Unit::Centimeter);

	const QRectF rect(x, y, w, h);
	m_plot->setRect(rect);
}

void Plot3DAreaDock::onGeometryChanged(Worksheet::Layout layout) {
	bool b = (layout == Worksheet::Layout::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
	if (!b)
		rectChanged(m_plot->rect());
}
void Plot3DAreaDock::rectChanged(const QRectF& rect) {
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), Worksheet::Unit::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), Worksheet::Unit::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Unit::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Unit::Centimeter));
}

void Plot3DAreaDock::retranslateUi() {
}
//*************************************************************
//************************* Settings **************************
//*************************************************************
void Plot3DAreaDock::load() {
}

void Plot3DAreaDock::loadConfig(KConfig& config) {
}
