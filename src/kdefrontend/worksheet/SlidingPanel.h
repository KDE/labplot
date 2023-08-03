/*
	File                 : SlidingPanel.h
	Project              : LabPlot
	Description          : Sliding panel shown in the presenter widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Fabian Kristof <fkristofszabolcs@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SLIDINGPANEL_H
#define SLIDINGPANEL_H

#include "src/backend/worksheet/plots/cartesian/CartesianPlot.h"

#include <QFrame>

class QLabel;
class QPushButton;
class QTimeLine;
class WorksheetView;
class QBoxLayout;
class QHBoxLayout;

class SlidingPanel : public QFrame {
	Q_OBJECT

public:
	enum class Position {
		Top,
		Bottom,
	};

	SlidingPanel(const QRect& screenRect, Position, QWidget* parent);
	~SlidingPanel() override;
	void slideShow();
	void slideHide();
	virtual bool insideRect(QPoint screenPos) = 0;

public Q_SLOTS:
	virtual void movePanel(qreal);

private:
	void startTimeline();

protected:
	QRect m_screenRect;

private:
	Position m_pos;
	QTimeLine* m_timeLine;
};

class SlidingPanelTop : public SlidingPanel {
	Q_OBJECT
public:
	explicit SlidingPanelTop(const QRect& screenRect, const QString& worksheetName, QWidget* parent);
	virtual bool insideRect(QPoint screenPos) override;

public:
	QPushButton* quitButton() const;

private:
	QLabel* m_worksheetName;
	QPushButton* m_quitPresentingMode;
	QSize sizeHint() const override;
};

class SlidingPanelBottom : public SlidingPanel {
	Q_OBJECT
public:
	explicit SlidingPanelBottom(const QRect& screenRect, WorksheetView* view, QWidget* parent);
	virtual bool insideRect(QPoint screenPos) override;

private:
	void addButtonToLayout(CartesianPlot::NavigationOperation op, QBoxLayout* layout, WorksheetView* view);
	QSize sizeHint() const override;

	int m_sizeHintHeight{0};

	QHBoxLayout* m_hlayout;
};

#endif // SLIDINGPANEL_H
