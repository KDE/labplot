/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/datasources/FileDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/HDFFilter.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/Workbook.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/MainWin.h"

#include <KMessageBox>
#include <KInputDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QDir>
#include <QInputDialog>
#include <KMenu>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeds \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */

ImportFileDialog::ImportFileDialog(MainWin* parent, bool fileDataSource, const QString& fileName) : KDialog(parent), m_mainWin(parent),
	cbAddTo(0), cbPosition(0), m_showOptions(false), m_newDataContainerMenu(0) {

	QWidget* mainWidget = new QWidget(this);
	vLayout = new QVBoxLayout(mainWidget);
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(0,0,0,0);

	importFileWidget = new ImportFileWidget(mainWidget, fileName);
	vLayout->addWidget(importFileWidget);
	setMainWidget(mainWidget);

	setButtons( KDialog::Ok | KDialog::User1 | KDialog::Cancel );

	//hide the data-source related widgets
	if (!fileDataSource) {
		this->setModel(m_mainWin->model());
		//TODO: disable for file data sources
		importFileWidget->hideDataSource();
	}

	connect(this, SIGNAL(user1Clicked()), this, SLOT(toggleOptions()));
	connect(importFileWidget, SIGNAL(fileNameChanged()), this, SLOT(checkOkButton()));
	connect(importFileWidget, SIGNAL(checkedFitsTableToMatrix(bool)), this, SLOT(checkOnFitsTableToMatrix(bool)));

	setCaption(i18n("Import Data to Spreadsheet or Matrix"));
	setWindowIcon(KIcon("document-import-database"));

	//restore saved settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	m_showOptions = conf.readEntry("ShowOptions", false);
	m_showOptions ? setButtonText(KDialog::User1, i18n("Hide Options")) : setButtonText(KDialog::User1, i18n("Show Options"));
	importFileWidget->showOptions(m_showOptions);
	restoreDialogSize(conf);
}

ImportFileDialog::~ImportFileDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	conf.writeEntry("ShowOptions", m_showOptions);
	if (cbPosition)
		conf.writeEntry("Position", cbPosition->currentIndex());

	saveDialogSize(conf);
}

/*!
	creates widgets for the frame "Add-To" and sets the current model in the combobox to \c model.
 */
void ImportFileDialog::setModel(QAbstractItemModel* model) {
	DEBUG("ImportFileDialog::setModel()");
	QDEBUG(" model =" << model);
	//Frame for the "Add To"-Stuff
	frameAddTo = new QGroupBox(this);
	frameAddTo->setTitle(i18n("Import To"));
	QGridLayout *grid = new QGridLayout(frameAddTo);
	grid->addWidget(new QLabel(i18n("Data container"), frameAddTo), 0, 0);

	cbAddTo = new TreeViewComboBox(frameAddTo);
	cbAddTo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QList<const char *> list;
	list << "Folder" << "Spreadsheet" << "Matrix" << "Workbook";
	cbAddTo->setTopLevelClasses(list);
	grid->addWidget(cbAddTo, 0, 1);

	list.clear();
	list << "Spreadsheet" << "Matrix" << "Workbook";
	cbAddTo->setSelectableClasses(list);
	cbAddTo->setModel(model);

	tbNewDataContainer = new QToolButton(frameAddTo);
	tbNewDataContainer->setIcon(KIcon("list-add"));
	grid->addWidget( tbNewDataContainer, 0, 2);

	lPosition = new QLabel(i18n("Position"), frameAddTo);
	lPosition->setEnabled(false);
	grid->addWidget(lPosition, 1, 0);

	cbPosition = new QComboBox(frameAddTo);
	cbPosition->setEnabled(false);
	cbPosition->addItem(i18n("Append"));
	cbPosition->addItem(i18n("Prepend"));
	cbPosition->addItem(i18n("Replace"));
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	cbPosition->setCurrentIndex(conf.readEntry("Position", 0));

	cbPosition->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	grid->addWidget(cbPosition, 1, 1);

	vLayout->addWidget(frameAddTo);

	//menu for new data container
	m_newDataContainerMenu = new KMenu(this);
	m_newDataContainerMenu->addAction( KIcon("labplot-workbook-new"), i18n("new Workbook") );
	m_newDataContainerMenu->addAction( KIcon("labplot-spreadsheet-new"), i18n("new Spreadsheet") );
	m_newDataContainerMenu->addAction( KIcon("labplot-matrix-new"), i18n("new Matrix") );

	//ok is only available if a valid container was selected
	enableButtonOk(false);

	connect(cbAddTo, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(checkOkButton()));
	connect(tbNewDataContainer, SIGNAL(clicked(bool)), this, SLOT(newDataContainerMenu()));
	connect(m_newDataContainerMenu, SIGNAL(triggered(QAction*)), this, SLOT(newDataContainer(QAction*)));
	DEBUG("ImportFileDialog::setModel() DONE");
}

void ImportFileDialog::setCurrentIndex(const QModelIndex& index) {
	DEBUG("ImportFileDialog::setCurrentIndex()");
	QDEBUG(" index =" << index);
	cbAddTo->setCurrentModelIndex(index);
	QDEBUG("cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	checkOkButton();
}

/*!
  triggers data import to the file data source \c source
*/
void ImportFileDialog::importToFileDataSource(FileDataSource* source, QStatusBar* statusBar) const {
	importFileWidget->saveSettings(source);

	//show a progress bar in the status bar
	QProgressBar* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(source->filter(), SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	WAIT_CURSOR;

	QTime timer;
	timer.start();
	source->read();
	statusBar->showMessage( i18n("File data source created in %1 seconds.", (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}
/*!
  triggers data import to the currently selected data container
*/
void ImportFileDialog::importTo(QStatusBar* statusBar) const {
	DEBUG("ImportFileDialog::importTo()");
	QDEBUG("cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG("ERROR in importTo(): No aspect available");
		return;
	}

	QString fileName = importFileWidget->fileName();
	AbstractFileFilter* filter = importFileWidget->currentFileFilter();
	AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	//show a progress bar in the status bar
	QProgressBar* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	connect(filter, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QTime timer;
	timer.start();
	if (aspect->inherits("Matrix")) {
		Matrix* matrix = qobject_cast<Matrix*>(aspect);
		filter->read(fileName, matrix, mode);
	} else if (aspect->inherits("Spreadsheet")) {
		Spreadsheet* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		filter->read(fileName, spreadsheet, mode);
	} else if (aspect->inherits("Workbook")) {
		Workbook* workbook = qobject_cast<Workbook*>(aspect);
		QList<AbstractAspect*> sheets = workbook->children<AbstractAspect>();

		QStringList names;
		FileDataSource::FileType fileType = importFileWidget->currentFileType();
		if (fileType == FileDataSource::HDF)
			names = importFileWidget->selectedHDFNames();
		else if (fileType == FileDataSource::NETCDF)
			names = importFileWidget->selectedNetCDFNames();
		//TODO
		//multiple extensions selected

		// multiple data sets/variables for HDF/NetCDF
		if (fileType == FileDataSource::HDF || fileType == FileDataSource::NETCDF) {
			int nrNames = names.size(), offset = sheets.size();

			int start=0;
			if (mode == AbstractFileFilter::Replace)
				start=offset;

			// add additional sheets
			for (int i = start; i < nrNames; i++) {
				Spreadsheet *spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
				if (mode == AbstractFileFilter::Prepend)
					workbook->insertChildBefore(spreadsheet,sheets[0]);
				else
					workbook->addChild(spreadsheet);
			}

			if (mode != AbstractFileFilter::Append)
				offset = 0;

			// import to sheets
			sheets = workbook->children<AbstractAspect>();
			for (int i = 0; i < nrNames; i++) {
				if (fileType == FileDataSource::HDF)
					((HDFFilter*) filter)->setCurrentDataSetName(names[i]);
				else
					((NetCDFFilter*) filter)->setCurrentVarName(names[i]);

				if (sheets[i+offset]->inherits("Matrix"))
					filter->read(fileName, qobject_cast<Matrix*>(sheets[i+offset]), AbstractFileFilter::Replace);
				else if (sheets[i+offset]->inherits("Spreadsheet"))
					filter->read(fileName, qobject_cast<Spreadsheet*>(sheets[i+offset]), AbstractFileFilter::Replace);
			}
		} else { // single import file types
			// use active spreadsheet/matrix if present, else new spreadsheet
			Spreadsheet* spreadsheet = workbook->currentSpreadsheet();
			Matrix* matrix = workbook->currentMatrix();
			if (spreadsheet != NULL)
				filter->read(fileName, spreadsheet, mode);
			else if (matrix != NULL)
				filter->read(fileName, matrix, mode);
			else {
				spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
				workbook->addChild(spreadsheet);
				filter->read(fileName, spreadsheet, mode);
			}
		}

	}
	statusBar->showMessage( i18n("File %1 imported in %2 seconds.", fileName, (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
	delete filter;
}

void ImportFileDialog::toggleOptions() {
	importFileWidget->showOptions(!m_showOptions);
	m_showOptions = !m_showOptions;
	m_showOptions ? setButtonText(KDialog::User1,i18n("Hide Options")) : setButtonText(KDialog::User1,i18n("Show Options"));

	//resize the dialog
	mainWidget()->resize(layout()->minimumSize());
	layout()->activate();
	resize( QSize(this->width(), 0).expandedTo(minimumSize()) );
}

void ImportFileDialog::newDataContainer(QAction* action) {
	DEBUG("ImportFileDialog::newDataContainer()");
	QString path = importFileWidget->fileName();
	QString name = path.right( path.length()-path.lastIndexOf(QDir::separator())-1 );

	QString type = action->iconText().split(' ')[1];
	if (name.isEmpty())
		name = action->iconText();

	bool ok;
	// child widgets can't have own icons
	QInputDialog* dlg = new QInputDialog(this);
	name = dlg->getText(this, i18n("Add %1", action->iconText()), i18n("%1 name:", type), QLineEdit::Normal, name, &ok);
	if (ok) {
		AbstractAspect* aspect;
		int actionIndex = m_newDataContainerMenu->actions().indexOf(action);
		if (actionIndex == 0)
			aspect = new Workbook(0, name);
		else if (actionIndex == 1)
			aspect = new Spreadsheet(0, name);
		else
			aspect = new Matrix(0, name);

		m_mainWin->addAspectToProject(aspect);
		QDEBUG("cbAddTo->setCurrentModelIndex() to " << m_mainWin->model()->modelIndexOfAspect(aspect));
		cbAddTo->setCurrentModelIndex(m_mainWin->model()->modelIndexOfAspect(aspect));
		checkOkButton();
	}

	delete dlg;
}

void ImportFileDialog::newDataContainerMenu() {
	m_newDataContainerMenu->exec( tbNewDataContainer->mapToGlobal(tbNewDataContainer->rect().bottomLeft()));
}

void ImportFileDialog::checkOnFitsTableToMatrix(const bool enable) {
	if (cbAddTo) {
		QDEBUG("cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			DEBUG("ERROR: no aspect available.");
			return;
		}

		if(aspect->inherits("Matrix"))
			enableButtonOk(enable);
	}
}

void ImportFileDialog::checkOkButton() {
	DEBUG("ImportFileDialog::checkOkButton()");
	if (cbAddTo) { //only check for the target container when no file data source is being added
		QDEBUG(" cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			enableButtonOk(false);
			lPosition->setEnabled(false);
			cbPosition->setEnabled(false);
			DEBUG("WARNING: no aspect available.");
			return;
		} else {
			DEBUG("Aspect available.");
			lPosition->setEnabled(true);
			cbPosition->setEnabled(true);

			//when doing ASCII import to a matrix, hide the options for using the file header (first line)
			//to name the columns since the column names are fixed in a matrix
			const Matrix* matrix = dynamic_cast<const Matrix*>(aspect);
			importFileWidget->showAsciiHeaderOptions(matrix == NULL);
		}
	}

	QString fileName = importFileWidget->fileName();
	if (importFileWidget->currentFileType() != FileDataSource::FITS) {
#ifndef _WIN32
		if (!fileName.isEmpty() && fileName.left(1) != QDir::separator())
			fileName = QDir::homePath() + QDir::separator() + fileName;
#endif
	} else {
		int extensionBraceletPos = -1;
		if (!fileName.isEmpty()) {
			if(fileName.right(1) == QLatin1String("]")) {
				for (int i = fileName.size() - 1; i >= 5; --i) {
					if (fileName.at(i) == QLatin1Char('[')) {
						extensionBraceletPos = i;
						break;
					}
				}
			}
		}

		if (fileName.left(1) != QDir::separator())
			fileName = QDir::homePath() + QDir::separator() + fileName.mid(0, extensionBraceletPos);
		else
			fileName = fileName.mid(0, extensionBraceletPos);
	}
	DEBUG(" fileName = " << fileName.toUtf8().constData());

	enableButtonOk( QFile::exists(fileName) ) ;
}
