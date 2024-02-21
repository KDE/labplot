/*
	File                 : Project.cpp
	Project              : LabPlot
	Description          : Represents a LabPlot project.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2011-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007-2008 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2007 Knut Franke <knut.franke@gmx.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/ErrorBar.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/KDEPlot.h"
#include "backend/worksheet/plots/cartesian/LollipopPlot.h"
#include "backend/worksheet/plots/cartesian/QQPlot.h"
#include "backend/worksheet/plots/cartesian/Value.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif

#ifndef SDK
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datasources/LiveDataSource.h"
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif
#endif

#include <KCompressionDevice>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <kwidgetsaddons_version.h>

#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMimeData>
#include <QThreadPool>
#include <QUndoStack>

namespace {
// xmlVersion of this labplot version
// the project version will compared with this.
// if you make any compatibilty changes to the xmlfile
// or the function in labplot, increase this number
int buildXmlVersion = 12;
}

/**
 * \class Project
 * \ingroup core
 * \brief Represents a project.
 *
 * Project represents the root node of all objects created during the runtime of the program.
 * Manages also the undo stack.
 */

/**
 * \enum Project::MdiWindowVisibility
 * \brief MDI subwindow visibility setting
 */
/**
 * \var Project::folderOnly
 * \brief only show MDI windows corresponding to Parts in the current folder
 */
/**
 * \var Project::foldAndSubfolders
 * \brief show MDI windows corresponding to Parts in the current folder and its subfolders
 */
/**
 * \var Project::allMdiWindows
 * \brief show MDI windows for all Parts in the project simultaneously
 */

class ProjectPrivate {
public:
	explicit ProjectPrivate(Project* owner)
		: modificationTime(QDateTime::currentDateTime())
		, q(owner) {
		setVersion(QStringLiteral(LVERSION));
	}
	QString name() const {
		return q->name();
	}

	bool setVersion(const QString& v) const {
		versionString = v;
		auto l = v.split(QLatin1Char('.'));
		const int count = l.count();
		int major = 0;
		int minor = 0;
		int patch = 0;
		bool ok;

		if (count > 0) {
			major = l.at(0).toInt(&ok);
			if (!ok)
				return false;
		}

		if (count > 1) {
			minor = l.at(1).toInt(&ok);
			if (!ok)
				return false;
		}

		if (count > 2) {
			patch = l.at(2).toInt(&ok);
			if (!ok)
				return false;
		}

		m_versionNumber = QT_VERSION_CHECK(major, minor, patch);
		return true;
	}

	static QString version() {
		return versionString;
	}

	static int versionNumber() {
		return m_versionNumber;
	}

	static int xmlVersion() {
		return mXmlVersion;
	}

	Project::DockVisibility dockVisibility{Project::DockVisibility::folderOnly};
	bool changed{false};
	bool aspectAddedSignalSuppressed{false};

	static int m_versionNumber;
	static int mXmlVersion;
	static QString versionString;

	QDateTime modificationTime;
	Project* const q;
	QString fileName;
	QString windowState;
	QString author;
	bool saveCalculations{true};
	QUndoStack undo_stack;
};

int ProjectPrivate::m_versionNumber = 0;
QString ProjectPrivate::versionString = QString();
int ProjectPrivate::mXmlVersion = buildXmlVersion;

Project::Project()
	: Folder(i18n("Project"), AspectType::Project)
	, d_ptr(new ProjectPrivate(this)) {
	Q_D(Project);
	// load default values for name, comment and author from config
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("Project"));

	QString user = qEnvironmentVariable("USER"); // !Windows
	if (user.isEmpty())
		user = qEnvironmentVariable("USERNAME"); // Windows
	d->author = group.readEntry(QStringLiteral("Author"), user);

	// we don't have direct access to the members name and comment
	//->temporary disable the undo stack and call the setters
	setUndoAware(false);
	setIsLoading(true);
	setName(group.readEntry(QStringLiteral("Name"), i18n("Project")));
	setComment(group.readEntry(QStringLiteral("Comment"), QString()));
	setUndoAware(true);
	setIsLoading(false);
	d->changed = false;

	connect(this, &Project::aspectDescriptionChanged, this, &Project::descriptionChanged);
	connect(this, &Project::childAspectAdded, this, &Project::aspectAddedSlot);
}

Project::~Project() {
	Q_D(Project);
	Q_EMIT aboutToClose();
#ifndef SDK
	// if the project is being closed and the live data sources still continue reading the data,
	// the dependent objects (columns, etc.), which are already deleted maybe here,  are still being notified about the changes.
	//->stop reading the live data sources prior to deleting all objects.
	for (auto* lds : children<LiveDataSource>())
		lds->pauseReading();

#ifdef HAVE_MQTT
	for (auto* client : children<MQTTClient>())
		client->pauseReading();
#endif
#endif
	// if the project is being closed, in Worksheet the scene items are being removed and the selection in the view can change.
	// don't react on these changes since this can lead crashes (worksheet object is already in the destructor).
	//->notify all worksheets about the project being closed.
	for (auto* w : children<Worksheet>(ChildIndexFlag::Recursive))
		w->setIsClosing();

	d->undo_stack.clear();
	delete d;
}

QString Project::version() {
	return ProjectPrivate::version();
}

int Project::versionNumber() {
	return ProjectPrivate::versionNumber();
}

int Project::xmlVersion() {
	return ProjectPrivate::xmlVersion();
}

void Project::setXmlVersion(int version) {
	ProjectPrivate::mXmlVersion = version;
}

int Project::currentBuildXmlVersion() {
	return buildXmlVersion;
}

QUndoStack* Project::undoStack() const {
	// Q_D(const Project);
	return &d_ptr->undo_stack;
}

QMenu* Project::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();

	// add close action
	menu->addSeparator();
	menu->addAction(QIcon::fromTheme(QLatin1String("document-close")), i18n("Close"), this, SIGNAL(closeRequested()));

	// add the actions from MainWin
	Q_EMIT requestProjectContextMenu(menu);

	return menu;
}

QMenu* Project::createFolderContextMenu(const Folder* folder) {
	QMenu* menu = const_cast<Folder*>(folder)->AbstractAspect::createContextMenu();
	Q_EMIT requestFolderContextMenu(folder, menu);
	return menu;
}

void Project::setDockVisibility(DockVisibility visibility) {
	Q_D(Project);
	d->dockVisibility = visibility;
	Q_EMIT mdiWindowVisibilityChanged();
}

Project::DockVisibility Project::dockVisibility() const {
	Q_D(const Project);
	return d->dockVisibility;
}

CLASS_D_ACCESSOR_IMPL(Project, QString, fileName, FileName, fileName)
CLASS_D_ACCESSOR_IMPL(Project, QString, windowState, WindowState, windowState)
BASIC_D_READER_IMPL(Project, QString, author, author)
CLASS_D_ACCESSOR_IMPL(Project, QDateTime, modificationTime, ModificationTime, modificationTime)
BASIC_D_READER_IMPL(Project, bool, saveCalculations, saveCalculations)

STD_SETTER_CMD_IMPL_S(Project, SetAuthor, QString, author)
void Project::setAuthor(const QString& author) {
	Q_D(Project);
	if (author != d->author)
		exec(new ProjectSetAuthorCmd(d, author, ki18n("%1: set author")));
}

STD_SETTER_CMD_IMPL_S(Project, SetSaveCalculations, bool, saveCalculations)
void Project::setSaveCalculations(bool save) {
	Q_D(Project);
	if (save != d->saveCalculations)
		exec(new ProjectSetSaveCalculationsCmd(d, save, ki18n("%1: save calculation changed")));
}

void Project::setChanged(const bool value) {
	if (isLoading())
		return;

	Q_D(Project);

	d->changed = value;

	if (value)
		Q_EMIT changed();
}

void Project::setSuppressAspectAddedSignal(bool value) {
	Q_D(Project);
	d->aspectAddedSignalSuppressed = value;
}

bool Project::aspectAddedSignalSuppressed() const {
	Q_D(const Project);
	return d->aspectAddedSignalSuppressed;
}

bool Project::hasChanged() const {
	Q_D(const Project);
	return d->changed;
}

/*!
 * \brief Project::descriptionChanged
 * This function is called, when an object changes its name. When a column changed its name and wasn't connected
 * before to the curve/column(formula), this is updated in this function.
 * \param aspect
 */
void Project::descriptionChanged(const AbstractAspect* aspect) {
	if (isLoading())
		return;

	// when the name of a column is being changed, it can match again the names being used in the plots, etc.
	// and we need to update the dependencies
	const auto* column = dynamic_cast<const AbstractColumn*>(aspect);
	if (column)
		updateColumnDependencies(column);

	Q_D(Project);
	d->changed = true;
	Q_EMIT changed();
}

/*!
 * \brief Project::aspectAddedSlot
 * When adding new columns, these should be connected to the corresponding curves
 * \param aspect
 */
void Project::aspectAddedSlot(const AbstractAspect* aspect) {
	if (isLoading())
		return;

	if (aspect->inherits(AspectType::AbstractColumn)) {
		// check whether new columns were added and if yes,
		// update the dependencies in the project
		QVector<const AbstractColumn*> columns;
		const auto* column = static_cast<const AbstractColumn*>(aspect);
		if (column)
			columns.append(column);
		else {
			for (auto* child : aspect->children<Column>(ChildIndexFlag::Recursive))
				columns.append(static_cast<const AbstractColumn*>(child));
		}

		if (!columns.isEmpty()) {
			// if a new column was addded, check whether the column name matches the missing
			// names in the plots, etc. and update the dependencies
			for (auto column : columns)
				updateColumnDependencies(column);
		}
	} else if (aspect->inherits(AspectType::Spreadsheet)) {
		// if a new spreadsheet was addded, check whether the spreadsheet name matches the missing
		// name in a linked spreadsheet, etc. and update the dependencies
		const auto* newSpreadsheet = static_cast<const Spreadsheet*>(aspect);
		updateSpreadsheetDependencies(newSpreadsheet);

		connect(static_cast<const Spreadsheet*>(aspect), &Spreadsheet::aboutToResize, [this]() {
			const auto& wes = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* we : wes)
				we->setSuppressRetransform(true);
		});
		connect(static_cast<const Spreadsheet*>(aspect), &Spreadsheet::resizeFinished, [this]() {
			const auto& wes = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive);
			for (auto* we : wes)
				we->setSuppressRetransform(false);
		});
	}
}

void Project::updateSpreadsheetDependencies(const Spreadsheet* spreadsheet) const {
	const QString& spreadsheetPath = spreadsheet->path();
	const auto& spreadsheets = children<Spreadsheet>(ChildIndexFlag::Recursive);

	for (auto* sh : spreadsheets) {
		sh->setUndoAware(false);
		if (sh->linkedSpreadsheetPath() == spreadsheetPath)
			sh->setLinkedSpreadsheet(spreadsheet);
		sh->setUndoAware(true);
	}
}

/*!
 * in case the column \c column was added or renamed, update all dependent objects in the project accordingly.
 */
void Project::updateColumnDependencies(const AbstractColumn* column) const {
	// update the dependencies in the plots
	const auto& plots = children<Plot>(ChildIndexFlag::Recursive);
	for (auto* plot : plots)
		plot->updateColumnDependencies(column);

	// update the dependencies in the column formulas
	const QString& columnPath = column->path();
	const auto& columns = children<Column>(ChildIndexFlag::Recursive);
	for (auto* tempColumn : columns) {
		for (int i = 0; i < tempColumn->formulaData().count(); i++) {
			auto path = tempColumn->formulaData().at(i).columnName();
			if (path == columnPath)
				tempColumn->setFormulVariableColumn(i, const_cast<Column*>(static_cast<const Column*>(column)));
		}
	}
}

void Project::navigateTo(const QString& path) {
	Q_EMIT requestNavigateTo(path);
}

/*!
 * returns \c true if the project file \fileName has a supported format and can be openned in LabPlot directly,
 * returns \c false otherwise.
 */
bool Project::isSupportedProject(const QString& fileName) {
	bool open = Project::isLabPlotProject(fileName);
#ifdef HAVE_LIBORIGIN
	if (!open)
		open = OriginProjectParser::isOriginProject(fileName);
#endif

#ifdef HAVE_CANTOR_LIBS
	if (!open) {
		QFileInfo fi(fileName);
		open = (fi.completeSuffix() == QLatin1String("cws")) || (fi.completeSuffix() == QLatin1String("ipynb"));
	}
#endif

	return open;
}

bool Project::isLabPlotProject(const QString& fileName) {
	return fileName.endsWith(QStringLiteral(".lml"), Qt::CaseInsensitive) || fileName.endsWith(QStringLiteral(".lml.gz"), Qt::CaseInsensitive)
		|| fileName.endsWith(QStringLiteral(".lml.bz2"), Qt::CaseInsensitive) || fileName.endsWith(QStringLiteral(".lml.xz"), Qt::CaseInsensitive);
}

QString Project::supportedExtensions() {
	static const QString extensions = QStringLiteral("*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ");
	return extensions;
}

QVector<quintptr> Project::droppedAspects(const QMimeData* mimeData) {
	auto data = mimeData->data(QLatin1String("labplot-dnd"));
	QDataStream stream(&data, QIODevice::ReadOnly);

	// read the project pointer first
	quintptr project = 0;
	stream >> project;

	// read the pointers of the dragged aspects
	QVector<quintptr> vec;
	stream >> vec;

	return vec;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

void Project::save(const QPixmap& thumbnail, QXmlStreamWriter* writer) {
	Q_D(Project);
	// set the version and the modification time to the current values
	d->setVersion(QStringLiteral(LVERSION));
	d->modificationTime = QDateTime::currentDateTime();

	writer->setAutoFormatting(true);
	writer->writeStartDocument();
	writer->writeDTD(QStringLiteral("<!DOCTYPE LabPlotXML>"));

	writer->writeStartElement(QStringLiteral("project"));
	writer->writeAttribute(QStringLiteral("version"), version());
	writer->writeAttribute(QStringLiteral("xmlVersion"), QString::number(buildXmlVersion));
	writer->writeAttribute(QStringLiteral("modificationTime"), modificationTime().toString(QStringLiteral("yyyy-dd-MM hh:mm:ss:zzz")));
	writer->writeAttribute(QStringLiteral("author"), author());
	writer->writeAttribute(QStringLiteral("saveCalculations"), QString::number(d->saveCalculations));
	writer->writeAttribute(QStringLiteral("windowState"), d->windowState);

	QString image;
	if (!thumbnail.isNull()) {
		QByteArray bArray;
		QBuffer buffer(&bArray);
		buffer.open(QIODevice::WriteOnly);
		QPixmap scaledThumbnail = thumbnail.scaled(512, 512, Qt::KeepAspectRatio);
		scaledThumbnail.save(&buffer, "JPEG");
		image = QString::fromLatin1(bArray.toBase64().data());
	}

	writer->writeAttribute(QStringLiteral("thumbnail"), image);
	writeBasicAttributes(writer);
	writeCommentElement(writer);
	save(writer);
}

/**
 * \brief Save as XML
 */
void Project::save(QXmlStreamWriter* writer) const {
	// save all children
	const auto& children = this->children<AbstractAspect>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children) {
		writer->writeStartElement(QStringLiteral("child_aspect"));
		child->save(writer);
		writer->writeEndElement();
	}

	// save the state of the views (visible, maximized/minimized/geometry)
	// and the state of the project explorer (expanded items, currently selected item)
	Q_EMIT requestSaveState(writer);

	writer->writeEndElement();
	writer->writeEndDocument();
	Q_EMIT saved();
}

bool Project::load(const QString& filename, bool preview) {
	setFileName(filename);
	DEBUG(Q_FUNC_INFO << ", LOADING file " << STDSTRING(filename))
	QIODevice* file;
	if (filename.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive)) {
		DEBUG(Q_FUNC_INFO << ", filename ends with .lml")

		// check compression
		file = new QFile(filename);
		if (!file->open(QIODevice::ReadOnly)) {
			KMessageBox::error(nullptr, i18n("Sorry. Could not open file for reading."));
			delete file;
			return false;
		}
		QDataStream in(file);
		quint16 magic;
		in >> magic;
		file->close();
		delete file;

		if (!magic) {
			KMessageBox::error(nullptr, i18n("The project file is empty."), i18n("Error opening project"));
			return false;
		}
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		QDEBUG(Q_FUNC_INFO << ", got magic: " << magic << Qt::hex << "0x" << magic)
#else
		QDEBUG(Q_FUNC_INFO << ", got magic: " << magic << hex << "0x" << magic)
#endif

		if (magic == 0xfd37) // XZ compressed data
			file = new KCompressionDevice(filename, KCompressionDevice::Xz);
		else // gzip or not compressed data
			file = new KCompressionDevice(filename, KCompressionDevice::GZip);
	} else { // opens filename using file ending
		// DEBUG(Q_FUNC_INFO << ", filename does not end with .lml. Guessing by extension")
		file = new KCompressionDevice(filename);
		DEBUG(Q_FUNC_INFO << ", found compression type " << ((KCompressionDevice*)file)->compressionType())
	}

	if (!file)
		file = new QFile(filename);

	if (!file->open(QIODevice::ReadOnly)) {
		KMessageBox::error(nullptr, i18n("Sorry. Could not open file for reading."));
		return false;
	}

	char c;
	bool rc = file->getChar(&c);
	if (!rc) {
		KMessageBox::error(nullptr, i18n("The project file is empty."), i18n("Error opening project"));
		file->close();
		delete file;
		return false;
	}
	file->seek(0);

	// parse XML
	XmlStreamReader reader(file);
	setIsLoading(true);
	ProjectPrivate::mXmlVersion =
		0; // set the version temporarily to 0, the actual project version will be read in the file, if available, and used in load() functions
	rc = this->load(&reader, preview);
	ProjectPrivate::mXmlVersion = buildXmlVersion; // set the version back to the current XML version
	setIsLoading(false);
	if (rc == false) {
		RESET_CURSOR;
		QString msg = reader.errorString();
		if (msg.isEmpty())
			msg = i18n("Unknown error when opening the project %1.", filename);
		KMessageBox::error(nullptr, msg, i18n("Error when opening the project"));
		file->close();
		delete file;
		return false;
	}

	if (reader.hasWarnings()) {
		qWarning("The following problems occurred when loading the project file:");
		const QStringList& warnings = reader.warningStrings();
		for (const auto& str : warnings)
			qWarning() << qUtf8Printable(str);

		// TODO: show warnings in a kind of "log window" but not in message box
		//  		KMessageBox::error(this, msg, i18n("Project loading partly failed"));
	}

	if (reader.hasMissingCASWarnings()) {
		RESET_CURSOR;

		const QString& msg = i18n(
			"The project has content written with %1. "
			"Your installation of LabPlot lacks the support for it.\n\n "
			"You won't be able to see this part of the project. "
			"If you modify and save the project, the CAS content will be lost.\n\n"
			"Do you want to continue?",
			reader.missingCASWarning());
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
		auto status = KMessageBox::warningTwoActions(nullptr, msg, i18n("Missing Support for CAS"), KStandardGuiItem::cont(), KStandardGuiItem::cancel());
		if (status == KMessageBox::SecondaryAction) {
#else
		auto status = KMessageBox::warningYesNo(nullptr, msg, i18n("Missing Support for CAS"));
		if (status == KMessageBox::No) {
#endif
			file->close();
			delete file;
			return false;
		}
	}

	file->close();
	delete file;

	return true;
}

/**
 * \brief Load from XML
 */
bool Project::load(XmlStreamReader* reader, bool preview) {
	Q_D(Project);
	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();

	bool stateAttributeFound = false;
	if (!(reader->atEnd())) {
		if (!reader->skipToNextTag())
			return false;

		if (reader->name() == QLatin1String("project")) {
			QString version = reader->attributes().value(QStringLiteral("version")).toString();
			if (version.isEmpty())
				reader->raiseWarning(i18n("Attribute 'version' is missing."));
			else
				d->setVersion(version);

			QString c = reader->attributes().value(QStringLiteral("xmlVersion")).toString();
			if (c.isEmpty())
				d->mXmlVersion = 0;
			else
				d->mXmlVersion = c.toInt();

			if (!readBasicAttributes(reader))
				return false;
			if (!readProjectAttributes(reader))
				return false;

			while (!reader->atEnd()) {
				reader->readNext();

				if (reader->isEndElement())
					break;

				if (reader->isStartElement()) {
					if (reader->name() == QLatin1String("comment")) {
						if (!readCommentElement(reader))
							return false;
					} else if (reader->name() == QLatin1String("child_aspect")) {
						if (!readChildAspectElement(reader, preview))
							return false;
					} else if (reader->name() == QLatin1String("state")) {
						// load the state of the views (visible, maximized/minimized/geometry)
						// and the state of the project explorer (expanded items, currently selected item).
						//"state" is read at the very end of XML, restore the pointers here so the current index
						// can be properly selected in ProjectExplorer after requestLoadState() is called.
						// Restore pointers and retransform elements before loading the state,
						// otherwise curves don't have column pointers assigned and therefore calculations
						// in the docks might be wrong
						stateAttributeFound = true;
						restorePointers(this);
						if (!preview)
							retransformElements(this);
						Q_EMIT requestLoadState(reader);
					} else {
						if (!preview)
							reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
						if (!reader->skipToEndElement())
							return false;
					}
				}
			}
		} else // no project element
			reader->raiseError(i18n("no project element found"));
	} else // no start document
		reader->raiseError(i18n("no valid XML document found"));

	if (!stateAttributeFound) {
		// No state attribute available, means no project explorer reacted on the signal
		restorePointers(this);
		if (!preview)
			retransformElements(this);
	}

	Q_EMIT loaded();

	return !reader->hasError();
}

void Project::retransformElements(AbstractAspect* aspect) {
	bool hasChildren = aspect->childCount<AbstractAspect>();

	// recalculate all analysis curves if the results of the calculations were not saved in the project
	if (!aspect->project()->saveCalculations()) {
		for (auto* curve : aspect->children<XYAnalysisCurve>(ChildIndexFlag::Recursive))
			curve->recalculate();
	}

	// set "isLoading" to false for all worksheet elements
	for (auto* child : aspect->children<WorksheetElement>(ChildIndexFlag::Recursive | ChildIndexFlag::IncludeHidden))
		child->setIsLoading(false);

	for (auto& column : aspect->project()->children<Column>(ChildIndexFlag::Recursive))
		column->setIsLoading(false);

	// all data was read:
	// call retransform() to every element
	if (hasChildren && aspect->type() == AspectType::Worksheet) {
		const auto& elements = aspect->children<WorksheetElement>(ChildIndexFlag::Recursive | ChildIndexFlag::IncludeHidden);
		for (auto* e : elements)
			e->retransform();
	} else if (hasChildren && aspect->type() != AspectType::CartesianPlot) {
		for (const auto* w : aspect->children<Worksheet>(ChildIndexFlag::Recursive | ChildIndexFlag::IncludeHidden)) {
			// retransform all elements in the worksheet (labels, images, plots)
			// the plots will then recursive retransform the childs of them
			const auto& elements = w->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
			for (auto* e : elements)
				e->retransform();
		}
	} else {
		QVector<CartesianPlot*> plots;
		if (aspect->type() == AspectType::CartesianPlot)
			plots << static_cast<CartesianPlot*>(aspect);
		else if (dynamic_cast<Plot*>(aspect))
			plots << static_cast<CartesianPlot*>(aspect->parentAspect());

		if (!plots.isEmpty()) {
			for (auto* plot : plots)
				plot->retransform();
		} else {
			// worksheet element is being copied alone without its parent plot object
			// so the plot retransform is not called above. we need to call it for the aspect.
			auto* e = dynamic_cast<WorksheetElement*>(aspect);
			if (e)
				e->retransform();
		}
	}

#ifndef SDK
	QVector<XYCurve*> curves;
	if (hasChildren)
		curves = aspect->children<XYCurve>(ChildIndexFlag::Recursive);
	// all data was read in live-data sources:
	// call CartesianPlot::dataChanged() to notify affected plots about the new data.
	// this needs to be done here since in LiveDataSource::finalizeImport() called above
	// where the data is read the column pointers are not restored yes in curves.
	QVector<CartesianPlot*> plots;
	for (auto* source : aspect->children<LiveDataSource>(ChildIndexFlag::Recursive)) {
		for (int n = 0; n < source->columnCount(); ++n) {
			Column* column = source->column(n);

			// determine the plots where the column is consumed
			for (const auto* curve : curves) {
				if (curve->xColumn() == column || curve->yColumn() == column) {
					auto* plot = static_cast<CartesianPlot*>(curve->parentAspect());
					if (plots.indexOf(plot) == -1) {
						plots << plot;
						plot->setSuppressRetransform(true);
					}
				}
			}

			column->setChanged();
		}
	}
#endif

	// loop over all affected plots and retransform them
	for (auto* plot : plots) {
		plot->setSuppressRetransform(false);
		plot->dataChanged(-1, -1);
	}
}

/*!
 * this function is used to restore the pointers to the columns in xy-curves etc.
 * from the stored column paths. This function is called after the project was loaded
 * and when an aspect is being pasted. In both cases we deserialized from XML and need
 * to restore the pointers.
 */
void Project::restorePointers(AbstractAspect* aspect) {
	// wait until all columns are decoded from base64-encoded data
	QThreadPool::globalInstance()->waitForDone();

	bool hasChildren = aspect->childCount<AbstractAspect>();
	const auto& columns = aspect->project()->children<Column>(ChildIndexFlag::Recursive);
	const auto& histograms = aspect->project()->children<Histogram>(ChildIndexFlag::Recursive); // needed for fit curves only. why a better implementation?

#ifndef SDK
	// LiveDataSource:
	// call finalizeLoad() to replace relative with absolute paths if required
	// and to create columns during the initial read
	for (auto* source : aspect->children<LiveDataSource>(ChildIndexFlag::Recursive)) {
		if (!source)
			continue;
		source->finalizeLoad();
	}
#endif

	// xy-curves
	//  cannot be removed by the column observer, because it does not react
	//  on curve changes
	QVector<XYCurve*> curves;
	if (hasChildren)
		curves = aspect->children<XYCurve>(ChildIndexFlag::Recursive);
	else if (aspect->inherits(AspectType::XYCurve) || aspect->inherits(AspectType::XYAnalysisCurve))
		// the object doesn't have any children -> one single aspect is being pasted.
		// check whether the object being pasted is a XYCurve and add it to the
		// list of curves to be retransformed
		curves << static_cast<XYCurve*>(aspect);

	for (auto* curve : qAsConst(curves)) {
		if (!curve)
			continue;
		curve->setSuppressRetransform(true);

		auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(curve);
		if (analysisCurve) {
			RESTORE_COLUMN_POINTER(analysisCurve, xDataColumn, XDataColumn);
			RESTORE_COLUMN_POINTER(analysisCurve, yDataColumn, YDataColumn);
			RESTORE_COLUMN_POINTER(analysisCurve, y2DataColumn, Y2DataColumn);
			auto* fitCurve = dynamic_cast<XYFitCurve*>(curve);
			if (fitCurve) {
				RESTORE_COLUMN_POINTER(fitCurve, xErrorColumn, XErrorColumn);
				RESTORE_COLUMN_POINTER(fitCurve, yErrorColumn, YErrorColumn);
				RESTORE_POINTER(fitCurve, dataSourceHistogram, DataSourceHistogram, Histogram, histograms);
			}
		} else {
			RESTORE_COLUMN_POINTER(curve, xColumn, XColumn);
			RESTORE_COLUMN_POINTER(curve, yColumn, YColumn);
			RESTORE_COLUMN_POINTER(curve, valuesColumn, ValuesColumn);
			RESTORE_COLUMN_POINTER(curve->errorBar(), xPlusColumn, XPlusColumn);
			RESTORE_COLUMN_POINTER(curve->errorBar(), xMinusColumn, XMinusColumn);
			RESTORE_COLUMN_POINTER(curve->errorBar(), yPlusColumn, YPlusColumn);
			RESTORE_COLUMN_POINTER(curve->errorBar(), yMinusColumn, YMinusColumn);
		}

		if (analysisCurve)
			RESTORE_POINTER(analysisCurve, dataSourceCurve, DataSourceCurve, XYCurve, curves);

		curve->setSuppressRetransform(false);
	}

	// assign to all markers the curves they need
	QVector<InfoElement*> elements;
	if (aspect->type() == AspectType::InfoElement) // check for the type first. InfoElement has children, but they are not relevant here
		elements << static_cast<InfoElement*>(aspect);
	else if (hasChildren)
		elements = aspect->children<InfoElement>(ChildIndexFlag::Recursive);

	for (auto* element : elements)
		element->assignCurve(curves);

	// axes
	QVector<Axis*> axes;
	if (hasChildren)
		axes = aspect->children<Axis>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::Axis)
		axes << static_cast<Axis*>(aspect);

	for (auto* axis : axes) {
		if (!axis)
			continue;
		RESTORE_COLUMN_POINTER(axis, majorTicksColumn, MajorTicksColumn);
		RESTORE_COLUMN_POINTER(axis, minorTicksColumn, MinorTicksColumn);
		RESTORE_COLUMN_POINTER(axis, labelsTextColumn, LabelsTextColumn);
	}

	// histograms
	QVector<Histogram*> hists;
	if (hasChildren)
		hists = aspect->children<Histogram>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::Histogram)
		hists << static_cast<Histogram*>(aspect);

	for (auto* hist : hists) {
		if (!hist)
			continue;
		RESTORE_COLUMN_POINTER(hist, dataColumn, DataColumn);
		auto* value = hist->value();
		RESTORE_COLUMN_POINTER(value, column, Column);
		RESTORE_COLUMN_POINTER(hist->errorBar(), yPlusColumn, YPlusColumn);
		RESTORE_COLUMN_POINTER(hist->errorBar(), yMinusColumn, YMinusColumn);
	}

	// QQ-plots
	QVector<QQPlot*> qqPlots;
	if (hasChildren)
		qqPlots = aspect->children<QQPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::QQPlot)
		qqPlots << static_cast<QQPlot*>(aspect);

	for (auto* plot : qqPlots) {
		if (!plot)
			continue;
		RESTORE_COLUMN_POINTER(plot, dataColumn, DataColumn);
	}

	// KDE-plots
	QVector<KDEPlot*> kdePlots;
	if (hasChildren)
		kdePlots = aspect->children<KDEPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::KDEPlot)
		kdePlots << static_cast<KDEPlot*>(aspect);

	for (auto* plot : kdePlots) {
		if (!plot)
			continue;
		RESTORE_COLUMN_POINTER(plot, dataColumn, DataColumn);
	}

	// box plots
	QVector<BoxPlot*> boxPlots;
	if (hasChildren)
		boxPlots = aspect->children<BoxPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::BoxPlot)
		boxPlots << static_cast<BoxPlot*>(aspect);

	for (auto* boxPlot : boxPlots) {
		if (!boxPlot)
			continue;

		// initialize the array for the column pointers
		int count = boxPlot->dataColumnPaths().count();
		QVector<const AbstractColumn*> dataColumns;
		dataColumns.resize(count);

		// restore the pointers
		for (int i = 0; i < count; ++i) {
			dataColumns[i] = nullptr;
			const auto& path = boxPlot->dataColumnPaths().at(i);
			for (Column* column : columns) {
				if (!column)
					continue;
				if (column->path() == path) {
					dataColumns[i] = column;
					break;
				}
			}
		}

		boxPlot->setDataColumns(dataColumns);
	}

	// bar plots
	QVector<BarPlot*> barPlots;
	if (hasChildren)
		barPlots = aspect->children<BarPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::BarPlot)
		barPlots << static_cast<BarPlot*>(aspect);

	for (auto* barPlot : barPlots) {
		if (!barPlot)
			continue;

		// initialize the array for the column pointers
		int count = barPlot->dataColumnPaths().count();
		QVector<const AbstractColumn*> dataColumns;
		dataColumns.resize(count);

		// restore the pointers
		for (int i = 0; i < count; ++i) {
			// data columns
			dataColumns[i] = nullptr;
			const auto& path = barPlot->dataColumnPaths().at(i);
			for (Column* column : columns) {
				if (!column)
					continue;
				if (column->path() == path) {
					dataColumns[i] = column;
					break;
				}
			}

			// error bars
			RESTORE_COLUMN_POINTER(barPlot->errorBarAt(i), yPlusColumn, YPlusColumn);
			RESTORE_COLUMN_POINTER(barPlot->errorBarAt(i), yMinusColumn, YMinusColumn);
		}

		barPlot->setDataColumns(dataColumns);

		RESTORE_COLUMN_POINTER(barPlot, xColumn, XColumn);
	}

	// lollipop plots
	QVector<LollipopPlot*> lollipopPlots;
	if (hasChildren)
		lollipopPlots = aspect->children<LollipopPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::BoxPlot)
		lollipopPlots << static_cast<LollipopPlot*>(aspect);

	for (auto* lollipopPlot : lollipopPlots) {
		if (!lollipopPlot)
			continue;

		// initialize the array for the column pointers
		int count = lollipopPlot->dataColumnPaths().count();
		QVector<const AbstractColumn*> dataColumns;
		dataColumns.resize(count);

		// restore the pointers
		for (int i = 0; i < count; ++i) {
			dataColumns[i] = nullptr;
			const auto& path = lollipopPlot->dataColumnPaths().at(i);
			for (Column* column : columns) {
				if (!column)
					continue;
				if (column->path() == path) {
					dataColumns[i] = column;
					break;
				}
			}
		}

		lollipopPlot->setDataColumns(dataColumns);

		RESTORE_COLUMN_POINTER(lollipopPlot, xColumn, XColumn);
	}

	// data picker curves
#ifndef SDK
	QVector<DatapickerCurve*> dataPickerCurves;
	if (hasChildren)
		dataPickerCurves = aspect->children<DatapickerCurve>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::DatapickerCurve)
		dataPickerCurves << static_cast<DatapickerCurve*>(aspect);

	for (auto* dataPickerCurve : dataPickerCurves) {
		if (!dataPickerCurve)
			continue;
		RESTORE_COLUMN_POINTER(dataPickerCurve, posXColumn, PosXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, posYColumn, PosYColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaXColumn, PlusDeltaXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaXColumn, MinusDeltaXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaYColumn, PlusDeltaYColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaYColumn, MinusDeltaYColumn);
	}
#endif

	// spreadsheet
	QVector<Spreadsheet*> spreadsheets;
	if (hasChildren)
		spreadsheets = aspect->children<Spreadsheet>(ChildIndexFlag::Recursive);
	for (auto* linkingSpreadsheet : spreadsheets) {
		if (!linkingSpreadsheet->linking())
			continue;
		for (const auto* toLinkedSpreadsheet : spreadsheets) {
			if (linkingSpreadsheet->linkedSpreadsheetPath() == toLinkedSpreadsheet->path()) {
				linkingSpreadsheet->setLinkedSpreadsheet(toLinkedSpreadsheet, true);
			}
		}
	}

	// if a column was calculated via a formula, restore the pointers to the variable columns defining the formula
	for (auto* col : columns) {
		for (Column* c : columns)
			col->setFormulaVariableColumn(c);
		col->finalizeLoad();
	}

	if (hasChildren && Project::xmlVersion() < 9) {
		const auto& plots = aspect->children<CartesianPlot>(ChildIndexFlag::Recursive);
		for (const auto* plot : plots) {
			const auto& axes = plot->children<Axis>(ChildIndexFlag::Recursive);
			for (auto* axis : axes) {
				const auto cSystem = plot->coordinateSystem(axis->coordinateSystemIndex());
				RangeT::Scale scale{RangeT::Scale::Linear};
				switch (axis->orientation()) {
				case Axis::Orientation::Horizontal:
					scale = plot->range(Dimension::X, cSystem->index(Dimension::X)).scale();
					break;
				case Axis::Orientation::Vertical:
					scale = plot->range(Dimension::Y, cSystem->index(Dimension::Y)).scale();
					break;
				case Axis::Orientation::Both:
					continue;
				}
				if (axis->scale() == scale) {
					axis->setUndoAware(false);
					axis->setRangeScale(true);
					axis->setUndoAware(true);
				}
			}
		}
	}
}

bool Project::readProjectAttributes(XmlStreamReader* reader) {
	Q_D(Project);
	const auto& attribs = reader->attributes();
	auto str = attribs.value(QStringLiteral("modificationTime")).toString();
	auto modificationTime = QDateTime::fromString(str, QStringLiteral("yyyy-dd-MM hh:mm:ss:zzz"));
	if (str.isEmpty() || !modificationTime.isValid()) {
		reader->raiseWarning(i18n("Invalid project modification time. Using current time."));
		d->modificationTime = QDateTime::currentDateTime();
	} else
		d->modificationTime = modificationTime;

	d->author = attribs.value(QStringLiteral("author")).toString();
	d->saveCalculations = attribs.value(QStringLiteral("saveCalculations")).toInt();
	d->windowState = attribs.value(QStringLiteral("windowState")).toString();

	return true;
}
