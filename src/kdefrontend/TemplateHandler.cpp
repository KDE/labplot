/*
    File                 : TemplateHandler.cpp
    Project              : LabPlot
    Description          : Widget for handling saving and loading of templates
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2012 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-FileCopyrightText: 2012-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "TemplateHandler.h"

#include <QFileInfo>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSpacerItem>
#include <QToolButton>
#include <QWidgetAction>


#include <QApplication>
#include <QMouseEvent>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>

static QVector<TemplateHandler*> templateHandlers;

 /*!
  \class TemplateHandler
  \brief Provides a widget with buttons for saving and loading of templates.

  Emits \c loadConfig() and \c saveConfig() signals that have to be connected
  to the appropriate slots in the ui (mostly in the dock widgets)

  \ingroup kdefrontend
*/

TemplateHandler::TemplateHandler(QWidget* parent, ClassName name) : QWidget(parent) {
	auto* horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(0);
	horizontalLayout->setMargin(0);

	auto* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	horizontalLayout->addItem(horizontalSpacer);

	int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);

	m_tbLoad = new QToolButton(this);
	m_tbLoad->setIconSize(QSize(size, size));
	horizontalLayout->addWidget(m_tbLoad);

	m_tbSave = new QToolButton(this);
	m_tbSave->setIconSize(QSize(size, size));
	horizontalLayout->addWidget(m_tbSave);

	m_tbSaveDefault = new QToolButton(this);
	m_tbSaveDefault->setIconSize(QSize(size, size));
	horizontalLayout->addWidget(m_tbSaveDefault);

// 	QSpacerItem* horizontalSpacer2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
// 	horizontalLayout->addItem(horizontalSpacer2);

// 	m_tbCopy = new QToolButton(this);
// 	m_tbCopy->setIconSize(QSize(size, size));
// 	m_tbCopy->setEnabled(false);
// 	horizontalLayout->addWidget(m_tbCopy);
//
// 	m_tbPaste = new QToolButton(this);
// 	m_tbPaste->setIconSize(QSize(size, size));
// 	m_tbPaste->setEnabled(false);
// 	horizontalLayout->addWidget(m_tbPaste);

	m_tbLoad->setIcon(QIcon::fromTheme("document-open"));
	m_tbSave->setIcon(QIcon::fromTheme("document-save"));
	m_tbSaveDefault->setIcon(QIcon::fromTheme("document-save-as"));
// 	m_tbCopy->setIcon(QIcon::fromTheme("edit-copy"));
// 	m_tbPaste->setIcon(QIcon::fromTheme("edit-paste"));

	connect(m_tbLoad, &QToolButton::clicked, this, &TemplateHandler::loadMenu);
	connect(m_tbSave, &QToolButton::clicked, this, &TemplateHandler::saveMenu);
	connect(m_tbSaveDefault, &QToolButton::clicked, this, &TemplateHandler::saveDefaults);


	KConfig config;
	KConfigGroup group = config.group(QLatin1String("TemplateHandler"));
	auto style = (Qt::ToolButtonStyle)group.readEntry(QLatin1String("TextPosition"), (int)Qt::ToolButtonTextBesideIcon);
	m_tbLoad->setToolButtonStyle(style);
	m_tbSave->setToolButtonStyle(style);
	m_tbSaveDefault->setToolButtonStyle(style);

	m_tbLoad->installEventFilter(this);
	m_tbSave->installEventFilter(this);
	m_tbSaveDefault->installEventFilter(this);

	m_className = name;

	//folder where config files will be stored in object specific sub-folders:
	//Linux    - ~/.local/share/labplot2/templates/
	//Mac      - //TODO
	//Windows  - C:/Users/<USER>/AppData/Roaming/labplot2/templates/
	m_dirName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/templates/");

	//synchronize this with the ordering in TemplateHandler::ClassName
	enum ClassName {Spreadsheet, Matrix, Worksheet, CartesianPlot, CartesianPlotLegend, Histogram, XYCurve, Axis, CustomPoint};
	m_subDirNames <<"spreadsheet"<<"matrix"<<"worksheet"<<"cartesianplot"
				<<"cartesianplotlegend"<<"histogram"<<"xycurve"<<"axis"<<"custompoint";

	this->retranslateUi();

	//disable the load-button if no templates are available yet
	QStringList list = QDir(m_dirName + m_subDirNames.at(static_cast<int>(m_className))).entryList();
	list.removeAll(QLatin1String("."));
	list.removeAll(QLatin1String(".."));
	m_tbLoad->setEnabled(list.size());

	//TODO: implement copy&paste of properties and activate copy- and paste-buttons again
// 	m_tbCopy->hide();
// 	m_tbPaste->hide();

	templateHandlers << this;
}

void TemplateHandler::retranslateUi() {
	m_tbLoad->setText(i18n("Load"));
	m_tbLoad->setToolTip(i18n("Load properties from a template"));
	m_tbSave->setText(i18n("Save"));
	m_tbSave->setToolTip(i18n("Save current properties as a template"));
	m_tbSaveDefault->setText(i18n("Save Default"));
	m_tbSaveDefault->setToolTip(i18n("Save current properties as default"));
// 	m_tbCopy->setToolTip(i18n("Copy properties"));
// 	m_tbPaste->setToolTip(i18n("Paste properties"));
}

bool TemplateHandler::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::MouseButtonPress) {
		auto* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::RightButton) {
			if (!m_textPositionMenu) {
				auto style = m_tbLoad->toolButtonStyle();

				m_textPositionMenu = new QMenu(this);
				m_textPositionMenu->addSection(i18n("Toolbar Settings"));

				auto* actionGroup = new QActionGroup(this);
				actionGroup->setExclusive(true);
				connect(actionGroup, &QActionGroup::triggered, this, &TemplateHandler::updateTextPosition);

				auto* subMenu = new QMenu(i18n("Text position"), m_textPositionMenu);

				QAction* action = new QAction(i18n("Icons only"), actionGroup);
				action->setCheckable(true);
				action->setData((int)Qt::ToolButtonIconOnly);
				if (style == Qt::ToolButtonIconOnly)
					action->setChecked(true);
				subMenu->addAction(action);

				action = new QAction(i18n("Text only"), actionGroup);
				action->setCheckable(true);
				action->setData((int)Qt::ToolButtonTextOnly);
				if (style == Qt::ToolButtonTextOnly)
					action->setChecked(true);
				subMenu->addAction(action);

				action = new QAction(i18n("Text Alongside Icons"), actionGroup);
				action->setCheckable(true);
				action->setData((int)Qt::ToolButtonTextBesideIcon);
				if (style == Qt::ToolButtonTextBesideIcon)
					action->setChecked(true);
				subMenu->addAction(action);

				action = new QAction(i18n("Text Under Icons"), actionGroup);
				action->setCheckable(true);
				action->setData((int)Qt::ToolButtonTextUnderIcon);
				if (style == Qt::ToolButtonTextUnderIcon)
					action->setChecked(true);
				subMenu->addAction(action);
				m_textPositionMenu->addMenu(subMenu);

			}

			auto* widget = static_cast<QWidget*>(obj);
			m_textPositionMenu->exec(widget->mapToGlobal(mouseEvent->pos()));
		}
	}
	return QWidget::eventFilter(obj, event);
}

void TemplateHandler::updateTextPosition(QAction* action) {
	auto style = static_cast<Qt::ToolButtonStyle>(action->data().toInt());

	//save the current style
	KConfig config;
	KConfigGroup group = config.group(QLatin1String("TemplateHandler"));
	group.writeEntry(QLatin1String("TextPosition"), (int)style);

	//update all available template handlers
	for (auto* handler : templateHandlers)
		handler->setToolButtonStyle(style);
}

void TemplateHandler::setToolButtonStyle(Qt::ToolButtonStyle style) {
	m_tbLoad->setToolButtonStyle(style);
	m_tbSave->setToolButtonStyle(style);
	m_tbSaveDefault->setToolButtonStyle(style);
}

//##############################################################################
//##################################  Slots ####################################
//##############################################################################
void TemplateHandler::loadMenu() {
	QMenu menu;
	menu.addSection(i18n("Load From Template"));

	QStringList list = QDir(m_dirName + m_subDirNames.at(static_cast<int>(m_className))).entryList();
	list.removeAll(QLatin1String("."));
	list.removeAll(QLatin1String(".."));
	for (int i = 0; i < list.size(); ++i) {
		QFileInfo fileinfo(list.at(i));
		QAction* action = menu.addAction(QIcon::fromTheme(QLatin1String("document-edit")), fileinfo.fileName());
		action->setData(fileinfo.fileName());
	}
	connect(&menu, &QMenu::triggered, this, &TemplateHandler::loadMenuSelected);

	QPoint pos(-menu.sizeHint().width()+m_tbLoad->width(),-menu.sizeHint().height());
	menu.exec(m_tbLoad->mapToGlobal(pos));
}

void TemplateHandler::loadMenuSelected(QAction* action) {
	QString configFile = m_dirName + m_subDirNames.at(static_cast<int>(m_className)) + '/' + action->data().toString();
	KConfig config(configFile, KConfig::SimpleConfig);
	Q_EMIT loadConfigRequested(config);
	Q_EMIT info( i18n("Template \"%1\" was loaded.", action->text().remove('&')) );
}

void TemplateHandler::saveMenu() {
	QMenu menu;
	menu.addSection(i18n("Save As Template"));

	QStringList list = QDir(m_dirName + m_subDirNames.at(static_cast<int>(m_className))).entryList();
	list.removeAll(QLatin1String("."));
	list.removeAll(QLatin1String(".."));
	for (int i = 0; i < list.size(); ++i) {
			QFileInfo fileinfo(list.at(i));
			QAction* action = menu.addAction(QIcon::fromTheme(QLatin1String("document-edit")), fileinfo.fileName());
			menu.addAction(action);
			action->setShortcut(QKeySequence());
	}
	connect(&menu, &QMenu::triggered, this, &TemplateHandler::saveMenuSelected);

	// add editable action
	auto* widgetAction = new QWidgetAction(this);
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);

	QLabel* label = new QLabel(i18n("New:"), frame);
	layout->addWidget(label);

	auto* leFilename = new QLineEdit(QString(), frame);
	layout->addWidget(leFilename);
	connect(leFilename, &QLineEdit::returnPressed, this, [=]() {saveNewSelected(leFilename->text());} );
	connect(leFilename, &QLineEdit::returnPressed, &menu, &QMenu::close);

	widgetAction->setDefaultWidget(frame);
	if (menu.actions().size() > 1)
		menu.addSeparator();
	menu.addAction(widgetAction);
	leFilename->setFocus();

	QPoint pos(-menu.sizeHint().width()+m_tbSave->width(),-menu.sizeHint().height());
	menu.exec(m_tbSave->mapToGlobal(pos));
}

/*!
 * Is called when the current properties are going to be saved as a new template.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveNewSelected(const QString& filename) {
	QString path = m_dirName + m_subDirNames.at(static_cast<int>(m_className)) + '/' + filename;
	KConfig config(path, KConfig::SimpleConfig);
	Q_EMIT saveConfigRequested(config);
	Q_EMIT info( i18n("New template \"%1\" was saved.", filename) );

	//we have at least one saved template now -> enable the load button
	m_tbLoad->setEnabled(true);
}

/*!
 * Is called when the current properties are going to be saved in an already available template.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveMenuSelected(QAction* action) {
	KConfig config(action->data().toString()+'/'+action->text(), KConfig::SimpleConfig);
	Q_EMIT saveConfigRequested(config);
	Q_EMIT info( i18n("Template \"%1\" was saved.", action->text()) );
}

/*!
 * Is called when the current properties are going to be saved as new default properties.
 * Emits \c saveConfigRequested, the receiver of the signal has to config.sync().
 */
void TemplateHandler::saveDefaults() {
	KConfig config;
	Q_EMIT saveConfigRequested(config);
	Q_EMIT info( i18n("New default template was saved.") );
}
