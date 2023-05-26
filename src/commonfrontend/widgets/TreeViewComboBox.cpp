/*
	File                 : TreeViewComboBox.cpp
	Project              : LabPlot
	Description          : Provides a QTreeView in a QComboBox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/lib/macros.h"

#include <QEvent>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QStylePainter>
#include <QTreeView>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <cstring> // strcmp()

/*!
	\class TreeViewComboBox
	\brief Provides a QTreeView in a QComboBox.

	\ingroup backend/widgets
*/
TreeViewComboBox::TreeViewComboBox(QWidget* parent)
	: QComboBox(parent)
	, m_treeView(new QTreeView)
	, m_groupBox(new QGroupBox)
	, m_lineEdit(new QLineEdit) {
	auto* layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	layout->addWidget(m_lineEdit);
	layout->addWidget(m_treeView);

	m_groupBox->setLayout(layout);
	m_groupBox->setParent(parent, Qt::Popup);
	m_groupBox->hide();
	m_groupBox->installEventFilter(this);

	m_treeView->header()->hide();
	m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_treeView->setUniformRowHeights(true);

	m_lineEdit->setPlaceholderText(i18n("Search/Filter text"));
	m_lineEdit->setClearButtonEnabled(true);
	m_lineEdit->setFocus();

	addItem(QString());
	setCurrentIndex(0);
	setEditText(m_lineEditText);

	// signal activated() is platform dependent
	connect(m_treeView, &QTreeView::pressed, this, &TreeViewComboBox::treeViewIndexActivated);
	connect(m_lineEdit, &QLineEdit::textChanged, this, &TreeViewComboBox::filterChanged);
}

void TreeViewComboBox::setTopLevelClasses(const QList<AspectType>& list) {
	m_topLevelClasses = list;
}

void TreeViewComboBox::setHiddenAspects(const QList<const AbstractAspect*>& list) {
	m_hiddenAspects = list;
}

/*!
	Sets the \a model for the view to present.
*/
void TreeViewComboBox::setModel(AspectTreeModel* model) {
	m_model = model;
	m_treeView->setModel(model);

	// show only the first column in the combo box
	for (int i = 1; i < model->columnCount(); i++)
		m_treeView->hideColumn(i);

	// Expand the complete tree in order to see everything in the first popup.
	m_treeView->expandAll();

	setEditText(m_lineEditText);
}

/*!
	Sets the current item to be the item at \a index and selects it.
	\sa currentIndex()
*/
void TreeViewComboBox::setCurrentModelIndex(const QModelIndex& index) {
	m_treeView->setCurrentIndex(index);
	QComboBox::setItemText(0, index.data().toString());
}

/*!
	Returns the model index of the current item.
	\sa setCurrentModelIndex()
*/
QModelIndex TreeViewComboBox::currentModelIndex() const {
	return m_treeView->currentIndex();
}

/*!
	Displays the tree view of items in the combobox.
	Triggers showTopLevelOnly() to show toplevel items only.
*/
void TreeViewComboBox::showPopup() {
	if (!m_treeView->model() || !m_treeView->model()->hasChildren())
		return;

	QModelIndex root = m_treeView->model()->index(0, 0);
	showTopLevelOnly(root);
	m_groupBox->show();
	m_groupBox->resize(this->width(), 250);
	m_groupBox->move(mapToGlobal(this->rect().topLeft()));

	setEditText(m_lineEditText);
	m_lineEdit->setText(QString()); // delete the previous search string
	m_lineEdit->setFocus();
}

/*!
	\reimp
	TODO: why do I have to reimplement paintEvent. It should work
	also without
*/
void TreeViewComboBox::paintEvent(QPaintEvent*) {
	QStylePainter painter(this);
	painter.setPen(palette().color(QPalette::Text));
	// draw the combobox frame, focusrect and selected etc.
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	opt.currentText = currentText(); // TODO: why it's not working when letting this away?
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);
	// draw the icon and text
	painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void TreeViewComboBox::hidePopup() {
	m_groupBox->hide();
}

void TreeViewComboBox::useCurrentIndexText(const bool set) {
	m_useCurrentIndexText = set;
}

/*!
	\property QComboBox::currentText
	\brief the current text
	If the combo box is editable, the current text is the value displayed
	by the line edit. Otherwise, it is the value of the current item or
	an empty string if the combo box is empty or no current item is set.
	The setter setCurrentText() simply calls setEditText() if the combo box is editable.
	Otherwise, if there is a matching text in the list, currentIndex is set to the
	corresponding index.
	If m_useCurrentIndexText is false, the Text set with setText is used. The intention of displaying
	this text is to show a text in the case of removed element.
	\sa editable, setEditText()
*/
QString TreeViewComboBox::currentText() const {
	if (lineEdit())
		return lineEdit()->text();
	else if (currentModelIndex().isValid() && m_useCurrentIndexText)
		return itemText(currentIndex());
	else if (!m_useCurrentIndexText)
		return m_lineEditText;

	return {};
}

void TreeViewComboBox::setText(const QString& text) {
	m_lineEditText = text;
}

void TreeViewComboBox::setInvalid(bool invalid, const QString& tooltip) {
	if (invalid) {
		SET_WARNING_PALETTE

		setToolTip(tooltip);
	} else {
		setPalette(qApp->palette());
		setToolTip(QString());
	}
}

/*!
	Hides the non-toplevel items of the model used in the tree view.
*/
void TreeViewComboBox::showTopLevelOnly(const QModelIndex& index) {
	int rows = index.model()->rowCount(index);
	for (int i = 0; i < rows; i++) {
		QModelIndex child = index.model()->index(i, 0, index);
		showTopLevelOnly(child);
		const auto* aspect = static_cast<const AbstractAspect*>(child.internalPointer());
		m_treeView->setRowHidden(i, index, !(isTopLevel(aspect) && !isHidden(aspect)));
	}
}

/*!
	catches the MouseButtonPress-event and hides the tree view on mouse clicking.
*/
bool TreeViewComboBox::eventFilter(QObject* object, QEvent* event) {
	if ((object == m_groupBox) && event->type() == QEvent::MouseButtonPress) {
		m_groupBox->hide();
		this->setFocus();
		return true;
	}
	return QComboBox::eventFilter(object, event);
}

// SLOTs
void TreeViewComboBox::treeViewIndexActivated(const QModelIndex& index) {
	if (index.internalPointer()) {
		QComboBox::setCurrentIndex(0);
		QComboBox::setItemText(0, index.data().toString());
		Q_EMIT currentModelIndexChanged(index);
		m_groupBox->hide();
		return;
	}

	m_treeView->setCurrentIndex(QModelIndex());
	setCurrentIndex(0);
	QComboBox::setItemText(0, QString());
	Q_EMIT currentModelIndexChanged(QModelIndex());
	m_groupBox->hide();
}

void TreeViewComboBox::filterChanged(const QString& text) {
	QModelIndex root = m_treeView->model()->index(0, 0);
	filter(root, text);
}

bool TreeViewComboBox::filter(const QModelIndex& index, const QString& text) {
	bool childVisible = false;
	const int rows = index.model()->rowCount(index);
	for (int i = 0; i < rows; i++) {
		QModelIndex child = index.model()->index(i, 0, index);
		auto* aspect = static_cast<AbstractAspect*>(child.internalPointer());
		bool topLevel = isTopLevel(aspect);
		if (!topLevel)
			continue;

		bool visible = aspect->name().contains(text, Qt::CaseInsensitive);

		if (visible) {
			// current item is visible -> make all its children (allowed top level types only and not hidden) visible without applying the filter
			for (int j = 0; j < child.model()->rowCount(child); ++j) {
				AbstractAspect* aspect = static_cast<AbstractAspect*>((child.model()->index(j, 0, child)).internalPointer());
				m_treeView->setRowHidden(j, child, !(isTopLevel(aspect) && !isHidden(aspect)));
			}

			childVisible = true;
		} else {
			// check children items. if one of the children is visible, make the parent (current) item visible too.
			visible = filter(child, text);
			if (visible)
				childVisible = true;
		}

		m_treeView->setRowHidden(i, index, !(visible && !isHidden(aspect)));
	}

	return childVisible;
}

/*!
	checks whether \c aspect is one of the allowed top level types
	and has selectable children
*/
bool TreeViewComboBox::isTopLevel(const AbstractAspect* aspect) const {
	const auto& selectableTypes = m_model->selectableAspects();
	for (AspectType type : m_topLevelClasses) {
		if (aspect->type() == type) {
			// curent aspect is a top level aspect,
			// check whether its selectable
			if (selectableTypes.indexOf(type) != -1)
				return true;

			// check whether the current aspect has selectable children
			bool hasSelectableAspects = false;
			for (auto selectableType : selectableTypes) {
				const auto& children = aspect->children(selectableType, AbstractAspect::ChildIndexFlag::Recursive);
				if (!children.isEmpty()) {
					hasSelectableAspects = true;
					break;
				}
			}

			return hasSelectableAspects;
		}

		if (type == AspectType::XYAnalysisCurve)
			if (aspect->inherits(AspectType::XYAnalysisCurve))
				return true;
	}
	return false;
}

bool TreeViewComboBox::isHidden(const AbstractAspect* aspect) const {
	return (m_hiddenAspects.indexOf(aspect) != -1);
}

void TreeViewComboBox::setAspect(const AbstractAspect* aspect) {
	if (aspect)
		setCurrentModelIndex(m_model->modelIndexOfAspect(aspect));
	else
		setCurrentModelIndex(QModelIndex());
}

AbstractAspect* TreeViewComboBox::currentAspect() const {
	return static_cast<AbstractAspect*>(currentModelIndex().internalPointer());
}

void TreeViewComboBox::setColumn(const AbstractColumn* column, const QString& path) {
	DEBUG(Q_FUNC_INFO)
	setAspect(column);

	// don't make the combobox red for initially created curves
	if (!column && path.isEmpty()) {
		setText(QString());
		setInvalid(false);
		return;
	}

	if (column) {
		useCurrentIndexText(true);
		setInvalid(false);
	} else {
		useCurrentIndexText(false);
		setInvalid(true, i18n("The column \"%1\"\nis not available anymore. It will be automatically used once it is created again.", path));
	}
	setText(path.split(QLatin1Char('/')).last());
}
