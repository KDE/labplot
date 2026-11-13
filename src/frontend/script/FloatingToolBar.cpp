/*
	File                 : FloatingToolBar.cpp
	Project              : LabPlot
	Description          : Toolbar floating on top of another widget, movable horizontally
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FloatingToolBar.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QToolBar>

namespace {
// grip on the left side of the toolbar to drag it
class Grip : public QWidget {
public:
	explicit Grip(QWidget* parent)
		: QWidget(parent) {
		setFixedWidth(10);
		setCursor(Qt::OpenHandCursor);
		setToolTip(i18n("Drag to move the toolbar"));
	}

protected:
	void paintEvent(QPaintEvent*) override {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(Qt::NoPen);
		painter.setBrush(palette().color(QPalette::Mid));
		// two columns of three dots
		const double r = 1.2;
		const double cy = height() / 2.;
		for (int col = 0; col < 2; ++col)
			for (int row = -1; row <= 1; ++row)
				painter.drawEllipse(QPointF(3 + col * 4, cy + row * 4), r, r);
	}
	// mouse events are not handled here and are propagated to the FloatingToolBar
};

const QString configGroup = QStringLiteral("ScriptEditor");
}

/*!
 * \class FloatingToolBar
 * \brief Toolbar floating on top of the parent widget (not part of its layout), similar to the debug toolbar of VS Code.
 * The toolbar is placed at the top of the parent and can be moved horizontally by dragging it at the grip.
 * The position is saved in the config under \c configKey.
 */
FloatingToolBar::FloatingToolBar(QWidget* parent, const QString& configKey)
	: QFrame(parent)
	, m_configKey(configKey) {
	setFrameShape(QFrame::StyledPanel);
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Window);

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(2, 1, 2, 1);
	layout->setSpacing(0);

	layout->addWidget(new Grip(this));

	m_toolBar = new QToolBar(this);
	m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
	const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
	m_toolBar->setIconSize(QSize(iconSize, iconSize));
	layout->addWidget(m_toolBar);

	// the size changes when actions are added or changed (e.g. Debug <-> Continue)
	m_toolBar->installEventFilter(this);
	// keep the position when the parent is resized
	parent->installEventFilter(this);

	KConfig config;
	m_position = std::clamp(config.group(configGroup).readEntry(m_configKey, 0.5), 0., 1.);

	updatePlacement();
	raise();
}

void FloatingToolBar::addAction(QAction* action) {
	m_toolBar->addAction(action);
}

void FloatingToolBar::addSeparator() {
	m_toolBar->addSeparator();
}

double FloatingToolBar::position() const {
	return m_position;
}

void FloatingToolBar::setPosition(double position) {
	m_position = std::clamp(position, 0., 1.);
	updatePlacement();
}

/*!
 * places the toolbar at its relative horizontal position at the top of the parent
 */
void FloatingToolBar::updatePlacement() {
	adjustSize();
	const int freeSpace = std::max(0, parentWidget()->width() - width());
	move(qRound(m_position * freeSpace), 0);
	raise(); // stay on top of the widgets added to the parent later
}

bool FloatingToolBar::eventFilter(QObject* watched, QEvent* event) {
	if ((watched == parentWidget() && event->type() == QEvent::Resize) || (watched == m_toolBar && event->type() == QEvent::LayoutRequest))
		updatePlacement();
	return QFrame::eventFilter(watched, event);
}

void FloatingToolBar::mousePressEvent(QMouseEvent* event) {
	if (event->button() != Qt::LeftButton)
		return QFrame::mousePressEvent(event);

	m_dragOffset = qRound(event->position().x());
	setCursor(Qt::ClosedHandCursor);
	event->accept();
}

void FloatingToolBar::mouseMoveEvent(QMouseEvent* event) {
	if (m_dragOffset < 0)
		return QFrame::mouseMoveEvent(event);

	const int freeSpace = parentWidget()->width() - width();
	if (freeSpace <= 0)
		return;

	const int x = std::clamp(qRound(mapToParent(event->position()).x()) - m_dragOffset, 0, freeSpace);
	m_position = double(x) / freeSpace;
	move(x, 0);
	event->accept();
}

void FloatingToolBar::mouseReleaseEvent(QMouseEvent* event) {
	if (m_dragOffset < 0)
		return QFrame::mouseReleaseEvent(event);

	m_dragOffset = -1;
	unsetCursor();

	KConfig config;
	config.group(configGroup).writeEntry(m_configKey, m_position);
	event->accept();
}
