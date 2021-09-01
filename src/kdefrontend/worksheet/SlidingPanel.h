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

#include <QFrame>

class QLabel;
class QPushButton;

class SlidingPanel : public QFrame {
	Q_OBJECT
public:
	explicit SlidingPanel(QWidget* parent, const QString& worksheetName);
	~SlidingPanel() override;
	QPushButton* quitButton() const;
private:

	QLabel* m_worksheetName;
	QPushButton* m_quitPresentingMode;
	QSize sizeHint() const override;

public slots:
	void movePanel(qreal);
};

#endif // SLIDINGPANEL_H
