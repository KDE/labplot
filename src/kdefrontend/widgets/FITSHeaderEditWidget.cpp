/*
    File                 : FITSHeaderEditWidget.cpp
    Project              : LabPlot
    Description          : Widget for listing/editing FITS header keywords
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Fabian Kristof <fkristofszabolcs@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FITSHeaderEditWidget.h"
#include "ui_fitsheadereditwidget.h"
#include "backend/datasources/filters/FITSFilter.h"
#include "backend/lib/macros.h"
#include "FITSHeaderEditNewKeywordDialog.h"
#include "FITSHeaderEditAddUnitDialog.h"

#include <QMenu>
#include <QTableWidget>
#include <QFileDialog>
#include <QContextMenuEvent>
#include <QPushButton>
#include <KConfigGroup>
#include <KMessageBox>
#include <KSharedConfig>

/*! \class FITSHeaderEditWidget
 * \brief Widget for listing/editing FITS header keywords
 * \since 2.4.0
 * \ingroup kdefrontend/widgets
 */
FITSHeaderEditWidget::FITSHeaderEditWidget(QWidget* parent) : QWidget(parent),
	ui(new Ui::FITSHeaderEditWidget()),
	m_fitsFilter(new FITSFilter()) {

	ui->setupUi(this);
	initActions();
	connectActions();
	initContextMenus();

	ui->bOpen->setIcon(QIcon::fromTheme("document-open"));

	ui->bAddKey->setIcon(QIcon::fromTheme("list-add"));
	ui->bAddKey->setEnabled(false);
	ui->bAddKey->setToolTip(i18n("Add new keyword"));

	ui->bRemoveKey->setIcon(QIcon::fromTheme("list-remove"));
	ui->bRemoveKey->setEnabled(false);
	ui->bRemoveKey->setToolTip(i18n("Remove selected keyword"));

	ui->bAddUnit->setIcon(QIcon::fromTheme("document-new"));
	ui->bAddUnit->setEnabled(false);
	ui->bAddUnit->setToolTip(i18n("Add unit to keyword"));

	ui->bClose->setIcon(QIcon::fromTheme("document-close"));
	ui->bClose->setEnabled(false);
	ui->bClose->setToolTip(i18n("Close file"));

	ui->twKeywordsTable->setColumnCount(3);
	ui->twExtensions->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->twExtensions->headerItem()->setText(0, i18n("Content"));
	ui->twKeywordsTable->setHorizontalHeaderItem(0, new QTableWidgetItem(i18n("Key")));
	ui->twKeywordsTable->setHorizontalHeaderItem(1, new QTableWidgetItem(i18n("Value")));
	ui->twKeywordsTable->setHorizontalHeaderItem(2, new QTableWidgetItem(i18n("Comment")));
	ui->twKeywordsTable->setAlternatingRowColors(true);
	ui->twKeywordsTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	ui->twKeywordsTable->horizontalHeader()->setStretchLastSection(true);
	ui->twKeywordsTable->installEventFilter(this);
	ui->twExtensions->installEventFilter(this);

	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui->bAddUnit, &QPushButton::clicked, m_actionAddmodifyUnit, &QAction::triggered);
	connect(ui->bClose, &QPushButton::clicked, this, &FITSHeaderEditWidget::closeFile);
	connect(ui->bOpen, &QPushButton::clicked, this, &FITSHeaderEditWidget::openFile);
	connect(ui->bAddKey, &QPushButton::clicked, this, &FITSHeaderEditWidget::addKeyword);
	connect(ui->bRemoveKey, &QPushButton::clicked, this, &FITSHeaderEditWidget::removeKeyword);
	connect(ui->twKeywordsTable, &QTableWidget::itemClicked, this, &FITSHeaderEditWidget::enableButtonAddUnit);
	connect(ui->twKeywordsTable, &QTableWidget::itemChanged, this, &FITSHeaderEditWidget::updateKeyword);
	connect(ui->twExtensions, &QTreeWidget::itemClicked, this, &FITSHeaderEditWidget::fillTableSlot);
	connect(ui->twExtensions, &QTreeWidget::itemClicked, this, &FITSHeaderEditWidget::enableButtonCloseFile);
}

/*!
 * \brief Destructor
 */
FITSHeaderEditWidget::~FITSHeaderEditWidget() {
	delete m_fitsFilter;
}

/*!
 * \brief Fills the keywords tablewidget.
 * If the selected extension was not yet selected before, then the keywords are read from the file
 * and then the table is filled, otherwise the table is filled using the already existing keywords.
 */
void FITSHeaderEditWidget::fillTable() {
	m_initializingTable = true;
	if (!m_extensionData.contains(m_seletedExtension)) {
		m_extensionData[m_seletedExtension].keywords = m_fitsFilter->chduKeywords(m_seletedExtension);
		m_extensionData[m_seletedExtension].updates.updatedKeywords.reserve(m_extensionData[m_seletedExtension].keywords.size());
		m_extensionData[m_seletedExtension].updates.updatedKeywords.resize(m_extensionData[m_seletedExtension].keywords.size());

		m_fitsFilter->parseHeader(m_seletedExtension, ui->twKeywordsTable);
	} else {
		QList<FITSFilter::Keyword> keywords = m_extensionData[m_seletedExtension].keywords;
		for (int i = 0; i < m_extensionData[m_seletedExtension].updates.updatedKeywords.size(); ++i) {
			FITSFilter::Keyword keyword = m_extensionData[m_seletedExtension].updates.updatedKeywords.at(i);
			if (!keyword.key.isEmpty())
				keywords.operator [](i).key = keyword.key;
			if (!keyword.value.isEmpty())
				keywords.operator [](i).value = keyword.value;
			if (!keyword.comment.isEmpty())
				keywords.operator [](i).comment = keyword.comment;
		}
		for (const FITSFilter::Keyword& key : m_extensionData[m_seletedExtension].updates.newKeywords)
			keywords.append(key);
		m_fitsFilter->parseHeader(QString(), ui->twKeywordsTable, false, keywords);
	}
	m_initializingTable = false;
}

/*!
 * \brief Fills the tablewidget with the keywords of extension \a item
 * \param item the extension selected
 * \param col the column of the selected item
 */
void FITSHeaderEditWidget::fillTableSlot(QTreeWidgetItem *item, int col) {
	WAIT_CURSOR;
	const QString& itemText = item->text(col);
	QString selectedExtension;
	int extType = 0;
	if (itemText.contains(QLatin1String("IMAGE #")) ||
	        itemText.contains(QLatin1String("ASCII_TBL #")) ||
	        itemText.contains(QLatin1String("BINARY_TBL #")))
		extType = 1;
	else if (!itemText.compare(QLatin1String("Primary header")))
		extType = 2;
	if (extType == 0) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr)
				selectedExtension = item->parent()->parent()->text(0) + '[' + item->text(col) + ']';
		}
	} else if (extType == 1) {
		if (item->parent() != nullptr) {
			if (item->parent()->parent() != nullptr) {
				bool ok;
				int hduNum = itemText.rightRef(1).toInt(&ok);
				selectedExtension = item->parent()->parent()->text(0) + '[' + QString::number(hduNum-1) + ']';
			}
		}
	} else {
		if (item->parent()->parent() != nullptr)
			selectedExtension = item->parent()->parent()->text(col);
	}

	if (!selectedExtension.isEmpty()) {
		if (!(m_seletedExtension == selectedExtension)) {
			m_seletedExtension = selectedExtension;
			fillTable();
		}
	}
	RESET_CURSOR;
}

/*!
 * \brief Shows a dialog for opening a FITS file
 * If the returned file name is not empty (so a FITS file was selected) and it's not opened yet
 * then the file is parsed, so the treeview for the extensions is built and the table is filled.
 */
void FITSHeaderEditWidget::openFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "FITSHeaderEditWidget");
	QString dir = conf.readEntry("LastDir", "");
	QString fileName = QFileDialog::getOpenFileName(this, i18n("Open FITS file"), dir,
	                   i18n("FITS files (*.fits *.fit *.fts)"));
	if (fileName.isEmpty())
		return;

	int pos = fileName.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString newDir = fileName.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastDir", newDir);
	}

	WAIT_CURSOR;
	QTreeWidgetItem* root = ui->twExtensions->invisibleRootItem();
	const int childCount = root->childCount();
	bool opened = false;
	for (int i = 0; i < childCount; ++i) {
		if (root->child(i)->text(0) == fileName) {
			opened = true;
			break;
		}
	}
	if (!opened) {
		for (QTreeWidgetItem* item : ui->twExtensions->selectedItems())
			item->setSelected(false);
		m_fitsFilter->parseExtensions(fileName, ui->twExtensions);
		ui->twExtensions->resizeColumnToContents(0);
		if (ui->twExtensions->selectedItems().size() > 0)
			fillTableSlot(ui->twExtensions->selectedItems().at(0), 0);

		ui->bAddKey->setEnabled(true);
		ui->bRemoveKey->setEnabled(true);
		ui->bAddUnit->setEnabled(true);
		ui->bClose->setEnabled(false);

	} else {
		KMessageBox::information(this, i18n("Cannot open file, file already opened."),
		                         i18n("File already opened"));
	}
	enableButtonAddUnit();
	RESET_CURSOR;
}

/*!
 * \brief Triggered when clicking the Save button
 * Saves the modifications (new keywords, new keyword units, keyword modifications,
 * deleted keywords, deleted extensions) to the FITS files.
 * \return \c true if there was something saved, otherwise false
 */
bool FITSHeaderEditWidget::save() {
	bool saved = false;

	QMap<QString, ExtensionData>::const_iterator it = m_extensionData.constBegin();
	while (it != m_extensionData.constEnd()) {
		const QString& fileName = it.key();
		const auto& data = it.value();
		if (data.updates.newKeywords.size() > 0) {
			m_fitsFilter->addNewKeyword(fileName, data.updates.newKeywords);
			if (!saved)
				saved = true;
		}
		if (data.updates.removedKeywords.size() > 0) {
			m_fitsFilter->deleteKeyword(fileName, data.updates.removedKeywords);
			if (!saved)
				saved = true;
		}
		if (!saved) {
			for (const FITSFilter::Keyword& key : data.updates.updatedKeywords) {
				if (!key.isEmpty()) {
					saved = true;
					break;
				}
			}
		}

		m_fitsFilter->updateKeywords(fileName, data.keywords, data.updates.updatedKeywords);
		m_fitsFilter->addKeywordUnit(fileName, data.keywords);
		m_fitsFilter->addKeywordUnit(fileName, data.updates.newKeywords);

		++it;
	}

	if (m_removedExtensions.size() > 0) {
		m_fitsFilter->removeExtensions(m_removedExtensions);
		if (!saved)
			saved = true;
	}
	if (saved) {
		//to reset the window title
		emit changed(false);
	}

	return saved;
}

/*!
 * \brief Initializes the context menu's actions.
 */
void FITSHeaderEditWidget::initActions() {
	m_actionAddKeyword = new QAction(QIcon::fromTheme("list-add"), i18n("Add New Keyword"), this);
	m_actionRemoveKeyword = new QAction(QIcon::fromTheme("list-remove"), i18n("Remove Keyword"), this);
	m_actionRemoveExtension = new QAction(i18n("Delete"), this);
	m_actionAddmodifyUnit = new QAction(i18n("Add Unit"), this);
}

/*!
 * \brief Connects signals of the actions to the appropriate slots.
 */
void FITSHeaderEditWidget::connectActions() {
	connect(m_actionAddKeyword, &QAction::triggered, this, [=](){addKeyword();});
	connect(m_actionRemoveKeyword, &QAction::triggered, this, [=](){removeKeyword();});
	connect(m_actionRemoveExtension, &QAction::triggered, this, [=](){removeExtension();});
	connect(m_actionAddmodifyUnit, &QAction::triggered, this, [=](){addModifyKeywordUnit();});
}

/*!
 * \brief Initializes the context menus.
 */
void FITSHeaderEditWidget::initContextMenus() {
	m_keywordActionsMenu = new QMenu(this);
	m_keywordActionsMenu->addAction(m_actionAddKeyword);
	m_keywordActionsMenu->addAction(m_actionRemoveKeyword);
	m_keywordActionsMenu->addSeparator();
	m_keywordActionsMenu->addAction(m_actionAddmodifyUnit);

	m_extensionActionsMenu = new QMenu(this);
	m_extensionActionsMenu->addAction(m_actionRemoveExtension);
}

/*!
 * \brief Shows a FITSHeaderEditNewKeywordDialog and decides whether the new keyword provided in the dialog
 * can be added to the new keywords or not. Updates the tablewidget if it's needed.
 */
void FITSHeaderEditWidget::addKeyword() {
	auto* newKeywordDialog = new FITSHeaderEditNewKeywordDialog;
	m_initializingTable = true;
	if (newKeywordDialog->exec() == QDialog::Accepted) {
		FITSFilter::Keyword newKeyWord = newKeywordDialog->newKeyword();
		QList<FITSFilter::Keyword> currentKeywords = m_extensionData[m_seletedExtension].keywords;

		for (const FITSFilter::Keyword& keyword : currentKeywords) {
			if (keyword.operator == (newKeyWord)) {
				KMessageBox::information(this, i18n("Cannot add keyword, keyword already added"), i18n("Cannot Add Keyword"));
				return;
			}
		}

		for (const FITSFilter::Keyword& keyword : m_extensionData[m_seletedExtension].updates.newKeywords) {
			if (keyword.operator == (newKeyWord)) {
				KMessageBox::information(this, i18n("Cannot add keyword, keyword already added"), i18n("Cannot Add Keyword"));
				return;
			}
		}

		for (const QString& keyword : mandatoryKeywords()) {
			if (!keyword.compare(newKeyWord.key)) {
				KMessageBox::information(this, i18n("Cannot add mandatory keyword, they are already present"),
				                         i18n("Cannot Add Keyword"));
				return;
			}
		}

		/*
		- Column related keyword (TFIELDS, TTYPEn,TFORMn, etc.) in an image
		- SIMPLE, EXTEND, or BLOCKED keyword in any extension
		- BSCALE, BZERO, BUNIT, BLANK, DATAMAX, DATAMIN keywords in a table
		- Keyword name contains illegal character
		*/

		m_extensionData[m_seletedExtension].updates.newKeywords.append(newKeyWord);

		const int lastRow = ui->twKeywordsTable->rowCount();
		ui->twKeywordsTable->setRowCount(lastRow + 1);
		auto* newKeyWordItem = new QTableWidgetItem(newKeyWord.key);
		newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		ui->twKeywordsTable->setItem(lastRow, 0, newKeyWordItem);

		newKeyWordItem = new QTableWidgetItem(newKeyWord.value);
		newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		ui->twKeywordsTable->setItem(lastRow, 1, newKeyWordItem);

		newKeyWordItem = new QTableWidgetItem(newKeyWord.comment);
		newKeyWordItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		ui->twKeywordsTable->setItem(lastRow, 2, newKeyWordItem);
		emit changed(true);
	}
	m_initializingTable = false;
	delete newKeywordDialog;
}

/*!
 * \brief Shows a messagebox whether we want to remove the keyword or not.
 * Mandatory keywords cannot be deleted.
 */
void FITSHeaderEditWidget::removeKeyword() {
	const int row = ui->twKeywordsTable->currentRow();
	if (row == -1)
		return;

	QString key = ui->twKeywordsTable->item(row, 0)->text();
	const int rc = KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete the keyword '%1'?", key),
	               i18n("Confirm Deletion"));
	if (rc == KMessageBox::Yes) {
		bool remove = true;
		for (const QString& k : mandatoryKeywords()) {
			if (!k.compare(key)) {
				remove = false;
				break;
			}
		}

		if (remove) {
			FITSFilter::Keyword toRemove = FITSFilter::Keyword(key,
			                               ui->twKeywordsTable->item(row, 1)->text(),
			                               ui->twKeywordsTable->item(row, 2)->text());
			ui->twKeywordsTable->removeRow(row);

			m_extensionData[m_seletedExtension].keywords.removeAt(row);
			m_extensionData[m_seletedExtension].updates.removedKeywords.append(toRemove);
			emit changed(true);
		} else
			KMessageBox::information(this, i18n("Cannot remove mandatory keyword."), i18n("Removing Keyword"));
	}

	enableButtonAddUnit();
}

/*!
 * \brief Trigggered when an item was updated by the user in the tablewidget
 * \param item the item which was updated
 */
void FITSHeaderEditWidget::updateKeyword(QTableWidgetItem *item) {
	if (!m_initializingTable) {
		const int row = item->row();
		if (row < 0)
			return;

		int idx;
		bool fromNewKeyword = false;
		if (row > m_extensionData[m_seletedExtension].keywords.size()-1) {
			idx = row - m_extensionData[m_seletedExtension].keywords.size();
			fromNewKeyword = true;
		} else
			idx = row;

		if (item->column() == 0) {
			if (!fromNewKeyword) {
				m_extensionData[m_seletedExtension].updates.updatedKeywords.operator [](idx).key = item->text();
				m_extensionData[m_seletedExtension].keywords.operator [](idx).updates.keyUpdated = true;
			} else {
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).key = item->text();
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).updates.keyUpdated = true;
			}

		} else if (item->column() == 1) {
			if (!fromNewKeyword) {
				m_extensionData[m_seletedExtension].updates.updatedKeywords.operator [](idx).value = item->text();
				m_extensionData[m_seletedExtension].keywords.operator [](idx).updates.valueUpdated = true;
			} else {
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).value = item->text();
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).updates.valueUpdated = true;
			}
		} else {
			if (!fromNewKeyword) {
				m_extensionData[m_seletedExtension].updates.updatedKeywords.operator [](idx).comment = item->text();
				m_extensionData[m_seletedExtension].keywords.operator [](idx).updates.commentUpdated = true;
			} else {
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).comment = item->text();
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).updates.commentUpdated = true;
			}
		}
		emit changed(true);
	}
}

/*!
 * \brief Shows a FITSHeaderEditAddUnitDialog on the selected keyword (provides the keyword's unit to the
 * dialog if it had one) and if the dialog was accepted then the new keyword unit is set and the tablewidget
 * is updated (filled with the modifications).
 */

void FITSHeaderEditWidget::addModifyKeywordUnit() {
	FITSHeaderEditAddUnitDialog* addUnitDialog;

	const int selectedRow = ui->twKeywordsTable->currentRow();
	int idx;
	bool fromNewKeyword = false;
	if (selectedRow > m_extensionData[m_seletedExtension].keywords.size()-1) {
		idx = selectedRow - m_extensionData[m_seletedExtension].keywords.size();
		fromNewKeyword = true;
	} else
		idx = selectedRow;

	QString unit;
	if (fromNewKeyword) {
		if (!m_extensionData[m_seletedExtension].updates.newKeywords.at(idx).unit.isEmpty())
			unit = m_extensionData[m_seletedExtension].updates.newKeywords.at(idx).unit;
	} else {
		if (!m_extensionData[m_seletedExtension].keywords.at(idx).unit.isEmpty())
			unit = m_extensionData[m_seletedExtension].keywords.at(idx).unit;
	}

	addUnitDialog = new FITSHeaderEditAddUnitDialog(unit);
	if (addUnitDialog->exec() == QDialog::Accepted) {
		if (fromNewKeyword) {
			m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).unit = addUnitDialog->unit();
			if (!m_extensionData[m_seletedExtension].updates.newKeywords.at(idx).unit.isEmpty()) {
				m_extensionData[m_seletedExtension].updates.newKeywords.operator [](idx).updates.unitUpdated = true;
			}
		} else {
			m_extensionData[m_seletedExtension].keywords.operator [](idx).unit = addUnitDialog->unit();
			if (!m_extensionData[m_seletedExtension].keywords.at(idx).unit.isEmpty())
				m_extensionData[m_seletedExtension].keywords.operator [](idx).updates.unitUpdated = true;
		}
		emit changed(true);
		fillTable();
	}

	delete addUnitDialog;
}

/*!
 * \brief Removes the selected extension from the extensions treeview
 * If the last extension is removed from the tree, then the extension and the file will be removed too.
 */
void FITSHeaderEditWidget::removeExtension() {
	QTreeWidgetItem* current = ui->twExtensions->currentItem();
	QTreeWidgetItem* newCurrent = ui->twExtensions->itemBelow(current);
	if (current->parent()) {
		if (current->parent()->childCount() < 2)
			delete current->parent();
		else
			delete current;
	}
	const QStringList keys = m_extensionData.keys();
	const int selectedidx = keys.indexOf(m_seletedExtension);

	if (selectedidx > 0) {
		const QString& ext = m_seletedExtension;
		m_extensionData.remove(ext);
		m_removedExtensions.append(ext);
		m_seletedExtension = keys.at(selectedidx-1);

		fillTable();
	}
	ui->twExtensions->setCurrentItem(newCurrent);
	emit changed(true);
}

/*!
 * \brief Returns a list of mandatory keywords according to the currently selected extension.
 * If the currently selected extension is an image then it returns the mandatory keywords of an image,
 * otherwise the mandatory keywords of a table
 * \return a list of mandatory keywords
 */
QList<QString> FITSHeaderEditWidget::mandatoryKeywords() const {
	QList<QString> mandatoryKeywords;
	const QTreeWidgetItem* currentItem = ui->twExtensions->currentItem();
	if (currentItem->parent()->text(0).compare(QLatin1String("Images")))
		mandatoryKeywords = FITSFilter::mandatoryImageExtensionKeywords();
	else
		mandatoryKeywords = FITSFilter::mandatoryTableExtensionKeywords();
	return mandatoryKeywords;
}

/*!
 * \brief Manipulates the contextmenu event of the widget
 * \param watched the object on which the event occurred
 * \param event the event watched
 * \return
 */
bool FITSHeaderEditWidget::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::ContextMenu) {
		auto* cm_event = static_cast<QContextMenuEvent*>(event);
		const QPoint& global_pos = cm_event->globalPos();
		if (watched == ui->twKeywordsTable) {
			if (ui->twExtensions->selectedItems().size() != 0)
				m_keywordActionsMenu->exec(global_pos);
		} else if (watched == ui->twExtensions) {
			if (ui->twExtensions->selectedItems().size() != 0) {
				QTreeWidgetItem* current = ui->twExtensions->currentItem();
				int col = ui->twExtensions->currentColumn();
				if (current->parent()) {
					if ((current->text(col) != QLatin1String("Images")) &&
					        (current->text(col) != QLatin1String("Tables")))
						m_extensionActionsMenu->exec(global_pos);
				}
			}
		} else
			return QWidget::eventFilter(watched, event);
		return true;
	} else
		return QWidget::eventFilter(watched, event);
}

void FITSHeaderEditWidget::closeFile() {
	if (ui->twExtensions->currentItem()) {
		QTreeWidgetItem* current = ui->twExtensions->currentItem();

		int idxOfCurrentAsTopLevel = -1;
		for (int i = 0; i < ui->twExtensions->topLevelItemCount(); ++i) {
			if (current == ui->twExtensions->topLevelItem(i)) {
				idxOfCurrentAsTopLevel = i;
				break;
			}
		}

		auto* newCurrent = (QTreeWidgetItem*)nullptr;
		if (idxOfCurrentAsTopLevel == 0) {
			if (ui->twExtensions->topLevelItemCount() == 1) {
				//last file closed, deactivate action buttons, clear keywords table
				ui->twKeywordsTable->setRowCount(0);
				ui->bClose->setEnabled(false);
				ui->bAddUnit->setEnabled(false);
				ui->bAddKey->setEnabled(false);
				ui->bRemoveKey->setEnabled(false);
			} else
				newCurrent = ui->twExtensions->topLevelItem(idxOfCurrentAsTopLevel + 1);
		} else
			newCurrent = ui->twExtensions->topLevelItem(idxOfCurrentAsTopLevel - 1);

		if (newCurrent) {
			m_seletedExtension = newCurrent->text(0);
			fillTable();
		}
		QMap<QString, ExtensionData>::const_iterator it = m_extensionData.constBegin();
		while (it != m_extensionData.constEnd()) {
			const QString& key = it.key();
			if (key.startsWith(current->text(0)))
				m_extensionData.remove(key);
			++it;
		}

		delete current;

		enableButtonAddUnit();
		emit changed(false);
	}
}
void FITSHeaderEditWidget::enableButtonAddUnit() {
	if (ui->twKeywordsTable->currentItem() != nullptr)
		ui->bAddUnit->setEnabled(true);
	else
		ui->bAddUnit->setEnabled(false);
}

void FITSHeaderEditWidget::enableButtonCloseFile(QTreeWidgetItem* item,int col) {
	Q_UNUSED(col)
	ui->bClose->setEnabled(item->parent() ? false : true);
}
