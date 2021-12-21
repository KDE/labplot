/*
    File                 : ThemesWidget.h
    Project              : LabPlot
    Description          : widget for selecting themes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Prakriti Bhardwaj <p_bhardwaj14@informatik.uni-kl.de>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef THEMESWIDGET_H
#define THEMESWIDGET_H

#include <QListView>

class ThemesWidget : public QListView {
	Q_OBJECT

public:
	explicit ThemesWidget(QWidget*);
	void setFixedMode();

Q_SIGNALS:
	void themeSelected(const QString&);
	void canceled();

private Q_SLOTS:
	void applyClicked(const QModelIndex&);
// 	void downloadThemes();
};

#endif //THEMESWIDGET_H
