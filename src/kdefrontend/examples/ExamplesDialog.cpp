/***************************************************************************
	File                 : ExamplesDialog.cpp
	Project              : LabPlot
	Description          : dialog showing the available example projects
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

#include "ExamplesDialog.h"
#include "kdefrontend/examples/ExamplesWidget.h"

#include <QDialogButtonBox>
#include <QWindow>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class ExamplesDialog
	\brief Dialog showing the available example projects.

	\ingroup kdefrontend
 */
ExamplesDialog::ExamplesDialog(QWidget* parent) : QDialog(parent),
	m_examplesWidget(new ExamplesWidget(this)){

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_examplesWidget);

	//dialog buttons
	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |QDialogButtonBox::Cancel);
	layout->addWidget(buttonBox);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setWindowTitle(i18nc("@title:window", "Example Projects"));
	setWindowIcon(QIcon::fromTheme("color-management"));
	create();

	QApplication::processEvents(QEventLoop::AllEvents, 0);

	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size());
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ExamplesDialog::~ExamplesDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ExamplesDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

QString ExamplesDialog::name() const {
	return m_examplesWidget->name();
}
