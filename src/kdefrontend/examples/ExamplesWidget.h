/***************************************************************************
	File                 : ExamplesWidget.h
	Project              : LabPlot
	Description          : widget showing the available example projects
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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


#ifndef EXAMPLESWIDGET_H
#define EXAMPLESWIDGET_H

#include "ui_exampleswidget.h"

class QCompleter;
class QStandardItemModel;
class ExamplesManager;

class ExamplesWidget : public QWidget {
	Q_OBJECT

public:
	explicit ExamplesWidget(QWidget*);
	~ExamplesWidget() override;

	QString path() const;

private:
	Ui::ExamplesWidget ui;
	QCompleter* m_completer{nullptr};
	QStandardItemModel* m_model{nullptr};
	ExamplesManager* m_manager{nullptr};

	void loadCollections();
	void activateIconViewItem(const QString& name);
	void activateListViewItem(const QString& name);

private slots:
	void collectionChanged(int);
	void colorMapChanged();
	void showInfo();
	void toggleIconView();
	void viewModeChanged(int);
	void activated(const QString&);

signals:
	void doubleClicked();
};

#endif // EXAMPLESWIDGET_H
