/*
	File                 : CantorWorksheet.cpp
	Project              : LabPlot
	Description          : Aspect providing a Cantor Worksheets for Multiple backends
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CantorWorksheet.h"
#include "VariableParser.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/lib/XmlStreamReader.h"
#ifndef SDK
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"
#endif

#include "3rdparty/cantor/cantor_part.h"
#include <cantor/cantorlibs_version.h>
#include <cantor/worksheetaccess.h>

#ifdef HAVE_NEW_CANTOR_LIBS
#include <cantor/panelplugin.h>
#include <cantor/panelpluginhandler.h>
#else
#include "3rdparty/cantor/panelplugin.h"
#include "3rdparty/cantor/panelpluginhandler.h"
#endif

#include <KConfigGroup>
#include <KLocalizedString>
#include <KParts/ReadWritePart>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <kcoreaddons_version.h>

#include <QAction>
#include <QFileInfo>
#include <QModelIndex>

CantorWorksheet::CantorWorksheet(const QString& name, bool loading)
	: AbstractPart(name, AspectType::CantorWorksheet)
	, m_backendName(name) {
	if (!loading)
		init();
}

/*!
	initializes Cantor's part and plugins
*/
bool CantorWorksheet::init(QByteArray* content) {
	DEBUG(Q_FUNC_INFO)

	const auto result = KPluginFactory::instantiatePlugin<KParts::ReadWritePart>(KPluginMetaData(QStringLiteral("kf5/parts/cantorpart")),
																				 this,
																				 QVariantList() << m_backendName << QLatin1String("--noprogress"));

	if (!result) {
		WARN("Could not find cantorpart part");
		return false;
	} else {
		m_part = result.plugin;
		if (!m_part) {
			WARN("Could not create the Cantor Part for backend " << STDSTRING(m_backendName))
			m_error = i18n("Couldn't find the plugin for %1. Please check your installation.", m_backendName);
			return false;
		}

		m_worksheetAccess = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
		if (!m_worksheetAccess)
			return false;

		// load worksheet content if available
		if (content)
			m_worksheetAccess->loadWorksheetFromByteArray(content);

		connect(m_worksheetAccess, SIGNAL(modified()), this, SLOT(modified()));

		// Cantor's session
#ifdef HAVE_CANTOR_LIBS
		m_session = m_worksheetAccess->session();
		if (!m_session) {
			WARN("Could not create the session for backend " << STDSTRING(m_backendName))
			m_error = i18n("Couldn't create the sessions for %1. Please check your installation.", m_backendName);
			return false;
		}

		connect(m_session, &Cantor::Session::statusChanged, this, &CantorWorksheet::statusChanged);

		// variable model
		m_variableModel = m_session->variableDataModel();
		connect(m_variableModel, &QAbstractItemModel::dataChanged, this, &CantorWorksheet::dataChanged);
		connect(m_variableModel, &QAbstractItemModel::rowsInserted, this, &CantorWorksheet::rowsInserted);
		connect(m_variableModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &CantorWorksheet::rowsAboutToBeRemoved);
		connect(m_variableModel, &QAbstractItemModel::modelReset, this, &CantorWorksheet::modelReset);

		// default settings
		const KConfigGroup group = Settings::group(QStringLiteral("Settings_Notebook"));

		// TODO: right now we don't have the direct accces to Cantor's worksheet and to all its public methods
		// and we need to go through the actions provided in cantor_part.
		//-> redesign this! expose Cantor's Worksheet directly and add more settings here.
		auto* action = m_part->action(QStringLiteral("enable_highlighting"));
		if (action) {
			bool value = group.readEntry(QLatin1String("SyntaxHighlighting"), false);
			action->setChecked(value);
		}

		action = m_part->action(QStringLiteral("enable_completion"));
		if (action) {
			bool value = group.readEntry(QLatin1String("SyntaxCompletion"), false);
			action->setChecked(value);
		}

		action = m_part->action(QStringLiteral("enable_expression_numbers"));
		if (action) {
			bool value = group.readEntry(QLatin1String("LineNumbers"), false);
			action->setChecked(value);
		}

		action = m_part->action(QStringLiteral("enable_typesetting"));
		if (action) {
			bool value = group.readEntry(QLatin1String("LatexTypesetting"), false);
			action->setChecked(value);
		}

		action = m_part->action(QStringLiteral("enable_animations"));
		if (action) {
			bool value = group.readEntry(QLatin1String("Animations"), false);
			action->setChecked(value);
		}

		// bool value = group.readEntry(QLatin1String("ReevaluateEntries"), false);
		// value = group.readEntry(QLatin1String("AskConfirmation"), true);
#endif
	}

	return true;
}

const QString& CantorWorksheet::error() const {
	return m_error;
}

// SLots
void CantorWorksheet::dataChanged(const QModelIndex& index) {
	parseData(index.row());
}

void CantorWorksheet::rowsInserted(const QModelIndex& /*parent*/, int first, int last) {
	for (int i = first; i <= last; ++i)
		parseData(i);

	setProjectChanged(true);
}

void CantorWorksheet::parseData(int row) {
	const QString& name = m_variableModel->data(m_variableModel->index(row, 0)).toString();
	QVariant dataValue = m_variableModel->data(m_variableModel->index(row, 1), 257);
	if (dataValue.isNull())
		dataValue = m_variableModel->data(m_variableModel->index(row, 1));

	const QString& value = dataValue.toString();
	VariableParser parser(m_backendName, value);

	if (parser.isParsed()) {
		auto* col = child<Column>(name);
		if (col) {
			switch (parser.dataType()) {
			case AbstractColumn::ColumnMode::Integer:
				col->setColumnMode(AbstractColumn::ColumnMode::Integer);
				col->setIntegers(parser.integers());
				break;
			case AbstractColumn::ColumnMode::BigInt:
				col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
				col->setBigInts(parser.bigInt());
				break;
			case AbstractColumn::ColumnMode::Double:
				col->setColumnMode(AbstractColumn::ColumnMode::Double);
				col->setValues(parser.doublePrecision());
				break;
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime:
				col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
				col->setDateTimes(parser.dateTime());
				break;
			case AbstractColumn::ColumnMode::Text:
				col->setColumnMode(AbstractColumn::ColumnMode::Text);
				col->setText(parser.text());
				break;
			}
		} else {
			// Column doesn't exist for this variable yet either because it was not defined yet or
			// because its values was changed now to an array-like structure after the initial definition.
			// -> create a new column for the current variable
			switch (parser.dataType()) {
			case AbstractColumn::ColumnMode::Integer:
				col = new Column(name, parser.integers());
				break;
			case AbstractColumn::ColumnMode::BigInt:
				col = new Column(name, parser.bigInt());
				break;
			case AbstractColumn::ColumnMode::Double:
				col = new Column(name, parser.doublePrecision());
				break;
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime:
				col = new Column(name, parser.dateTime(), parser.dataType());
				break;
			case AbstractColumn::ColumnMode::Text:
				col = new Column(name, parser.text());
				break;
			}
			col->setUndoAware(false);
			col->setFixed(true);
			addChild(col);

			// TODO: Cantor currently ignores the order of variables in the worksheets
			// and adds new variables at the last position in the model.
			// Fix this in Cantor and switch to insertChildBefore here later.
			// insertChildBefore(col, child<Column>(i));
		}
	} else {
		// the already existing variable doesn't contain any numerical values -> remove it
		Column* col = child<Column>(name);
		if (col)
			removeChild(col);
	}
}

void CantorWorksheet::modified() {
	setProjectChanged(true);
}

void CantorWorksheet::modelReset() {
	for (auto* column : children<Column>())
		column->remove();
}

void CantorWorksheet::rowsAboutToBeRemoved(const QModelIndex& /*parent*/, int first, int last) {
	for (int i = first; i <= last; ++i) {
		const QString& name = m_variableModel->data(m_variableModel->index(first, 0)).toString();
		Column* column = child<Column>(name);
		if (column)
			column->remove();
	}
}

QList<Cantor::PanelPlugin*> CantorWorksheet::getPlugins() {
	if (!m_pluginsLoaded) {
#ifdef HAVE_NEW_CANTOR_LIBS
		auto* handler = new Cantor::PanelPluginHandler(this);
		handler->loadPlugins();
		m_plugins = handler->activePluginsForSession(m_session, Cantor::PanelPluginHandler::PanelStates());
		for (auto* plugin : m_plugins)
			plugin->connectToShell(m_part);
#else
		auto* handler = m_part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
		if (!handler) {
			m_error = i18n("Couldn't find panel plugins. Please check your installation.");
			return false;
		}
		m_plugins = handler->plugins();
#endif

		m_pluginsLoaded = true;
	}

	return m_plugins;
}

KParts::ReadWritePart* CantorWorksheet::part() {
	return m_part;
}

QIcon CantorWorksheet::icon() const {
#ifdef HAVE_CANTOR_LIBS
	if (m_session)
		return QIcon::fromTheme(m_session->backend()->icon());
#endif
	return {};
}

QWidget* CantorWorksheet::view() const {
#ifndef SDK
	if (!m_partView) {
		m_view = new CantorWorksheetView(const_cast<CantorWorksheet*>(this));
		m_view->setBaseSize(1500, 1500);
		m_partView = m_view;
		connect(this, &CantorWorksheet::viewAboutToBeDeleted, [this]() {
			m_view = nullptr;
		});
		// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));

		// set the current path in the session to the path of the project file
#ifdef HAVE_CANTOR_LIBS
		if (m_session) {
			const Project* project = const_cast<CantorWorksheet*>(this)->project();
			const QString& fileName = project->fileName();
			if (!fileName.isEmpty()) {
				QFileInfo fi(fileName);
				m_session->setWorksheetPath(fi.filePath());
			}
		}
#endif
	}
#endif
	return m_partView;
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu* CantorWorksheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

void CantorWorksheet::fillColumnContextMenu(QMenu* menu, Column* column) {
#ifndef SDK
	if (m_view)
		m_view->fillColumnContextMenu(menu, column);
#else
	Q_UNUSED(menu)
	Q_UNUSED(column)
#endif
}

QString CantorWorksheet::backendName() {
	return this->m_backendName;
}

bool CantorWorksheet::exportView() const {
	// TODO: file_export_pdf exists starting with Cantor 23.12,
	// remove this check later once 23.12 is the minimal
	// supported version of Cantor.
	auto* action = m_part->action(QStringLiteral("file_export_pdf"));
	if (action) {
		action->trigger();
		return true;
	} else
		return false;
}

bool CantorWorksheet::printView() {
	m_part->action(QStringLiteral("file_print"))->trigger();
	return true;
}

bool CantorWorksheet::printPreview() const {
	m_part->action(QStringLiteral("file_print_preview"))->trigger();
	return true;
}

void CantorWorksheet::evaluate() {
	m_part->action(QStringLiteral("evaluate_worksheet"))->trigger();
}

void CantorWorksheet::restart() {
	m_part->action(QStringLiteral("restart_backend"))->trigger();
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void CantorWorksheet::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("cantorWorksheet"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("backend_name"), m_backendName);
	// TODO: save worksheet settings
	writer->writeEndElement();

	// save the content of Cantor's worksheet
	QByteArray content = m_worksheetAccess->saveWorksheetToByteArray();
	writer->writeStartElement(QStringLiteral("worksheet"));
	writer->writeAttribute(QStringLiteral("content"), QLatin1String(content.toBase64()));
	writer->writeEndElement();

	// save columns(variables)
	for (auto* col : children<Column>(ChildIndexFlag::IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); // close "cantorWorksheet" section
}

//! Load from XML
bool CantorWorksheet::load(XmlStreamReader* reader, bool preview) {
	// reset the status of the reader differentiating between
	//"failed because of the missing CAS" and "failed because of the broken XML"
	reader->setFailedCASMissing(false);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	bool rc = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("cantorWorksheet"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			m_backendName = attribs.value(QStringLiteral("backend_name")).toString().trimmed();
			if (m_backendName.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("backend_name"));
		} else if (!preview && reader->name() == QLatin1String("worksheet")) {
			attribs = reader->attributes();

			QString str = attribs.value(QStringLiteral("content")).toString().trimmed();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("content"));

			QByteArray content = QByteArray::fromBase64(str.toLatin1());
			rc = init(&content);
			if (!rc) {
				reader->raiseMissingCASWarning(m_backendName);

				// failed to load this object because of the missing CAS plugin
				// and not because of the broken project XML. Set this flag to
				// handle this case correctly.
				// TODO: we also can fail in the limit in cases where Cantor's content is broken
				// and not because of the missing CAS plugin. This also needs to be treated accrodingly...
				reader->setFailedCASMissing(true);
				return false;
			}
		} else if (!preview && reader->name() == QLatin1String("column")) {
			Column* column = new Column(QString());
			column->setUndoAware(false);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			column->setFixed(true);
			addChild(column);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}
