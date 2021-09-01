/*
    File                 : ThemesComboBox.h
    Project              : LabPlot
    Description          : Preview of all themes in a QComboBox
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef THEMESCOMBOBOX_H
#define THEMESCOMBOBOX_H

#include <QComboBox>

class QGroupBox;
class ThemesWidget;

class ThemesComboBox : public QComboBox {
	Q_OBJECT

public:
	explicit ThemesComboBox(QWidget* parent = nullptr);
	void setCurrentTheme(const QString&);

	void showPopup() override;
	void hidePopup() override;

private:
	QGroupBox* m_groupBox;
	ThemesWidget* m_view;
	bool eventFilter(QObject*, QEvent*) override;

private slots:
	void handleThemeChanged(const QString&);

signals:
	void currentThemeChanged(const QString&);
};

#endif
