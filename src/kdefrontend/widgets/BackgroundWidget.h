/*
	File                 : BackgroundWidget.h
	Project              : LabPlot
	Description          : background settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include "backend/worksheet/Background.h"
#include "ui_backgroundwidget.h"

class QShowEvent;
#include <KConfigGroup>

class BackgroundWidget : public QWidget {
	Q_OBJECT

public:
	explicit BackgroundWidget(QWidget*);

	void setBackgrounds(const QList<Background*>&);
	void setEnabled(bool);

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::BackgroundWidget ui;
	Background* m_background{nullptr};
	QList<Background*> m_backgrounds;
	bool m_initializing{false};
	QString m_prefix;

	void retranslateUi();
	void showEvent(QShowEvent*) override;
	void adjustLayout();

Q_SIGNALS:
	void dataChanged(bool);

private Q_SLOTS:
	// SLOTs for changes triggered in BackgroundWidget
	void enabledChanged(bool);
	void positionChanged(int);
	void typeChanged(int);
	void colorStyleChanged(int);
	void imageStyleChanged(int);
	void brushStyleChanged(int);
	void firstColorChanged(const QColor&);
	void secondColorChanged(const QColor&);
	void opacityChanged(int);
	void selectFile();
	void fileNameChanged();

	// SLOTs for changes triggered in Background
	void backgroundEnabledChanged(bool);
	void backgroundPositionChanged(Background::Position);
	void backgroundTypeChanged(Background::Type);
	void backgroundColorStyleChanged(Background::ColorStyle);
	void backgroundImageStyleChanged(Background::ImageStyle);
	void backgroundBrushStyleChanged(Qt::BrushStyle);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void backgroundFileNameChanged(const QString&);
	void backgroundOpacityChanged(float);
};

#endif // LABELWIDGET_H
