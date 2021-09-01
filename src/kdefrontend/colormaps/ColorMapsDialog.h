/*
	File                 : ColorMapsDialog.h
	Project              : LabPlot
	Description          : dialog showing the available color maps
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLORMAPSDIALOG_H
#define COLORMAPSDIALOG_H

#include <QDialog>

class ColorMapsWidget;

class ColorMapsDialog : public QDialog {
	Q_OBJECT

public:
	explicit ColorMapsDialog(QWidget*);
	~ColorMapsDialog() override;

	QPixmap previewPixmap() const;
	QString name() const;
	QVector<QColor> colors() const;

private:
	ColorMapsWidget* m_colorMapsWidget;
};

#endif // COLORMAPSDIALOG_H
