/*
    File                 : Project.cpp
    Project              : LabPlot
    Description          : Represents a LabPlot project.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2021 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2007-2008 Tilman Benkert (thzs@gmx.net)
    SPDX-FileCopyrightText: 2007 Knut Franke (knut.franke@gmx.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/InfoElement.h"
#include "backend/datapicker/DatapickerCurve.h"
#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <QDateTime>
#include <QFile>
#include <QMenu>
#include <QMimeData>
#include <QThreadPool>
#include <QUndoStack>
#include <QBuffer>

#include <KConfig>
#include <KConfigGroup>
#include <KFilterDev>
#include <KLocalizedString>
#include <KMessageBox>

namespace {
	// xmlVersion of this labplot version
	// the project version will compared with this.
	// if you make any compatibilty changes to the xmlfile
	// or the function in labplot, increase this number
	int buildXmlVersion = 5;
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

class Project::Private {
public:
	Private(Project* owner) : modificationTime(QDateTime::currentDateTime()), q(owner) {
		setVersion(LVERSION);
	}
	QString name() const  {
		return q->name();
	}

	bool setVersion(const QString &v) const {
		versionString = v;
		auto l = v.split(".");
		assert(l.length() == 3);
		bool ok;
		int major = l[0].toInt(&ok);
		if (!ok)
			return false;
		int minor = l[1].toInt(&ok);
		if (!ok)
			return false;
		int patch = l[2].toInt(&ok);
		if (!ok)
			return false;
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

	MdiWindowVisibility mdiWindowVisibility{Project::MdiWindowVisibility::folderOnly};
	bool changed{false};
	bool aspectAddedSignalSuppressed{false};

	static int m_versionNumber;
	static int mXmlVersion;
	static QString versionString;

	QDateTime modificationTime;
	Project* const q;
	QString fileName;
	QString author;
	QUndoStack undo_stack;
};

int Project::Private::m_versionNumber = 0;
QString Project::Private::versionString = "";
int Project::Private::mXmlVersion = 0;

Project::Project() : Folder(i18n("Project"), AspectType::Project), d(new Private(this)) {
	//load default values for name, comment and author from config
	KConfig config;
	KConfigGroup group = config.group("Project");

	QString user = qEnvironmentVariable("USER");	// !Windows
	if (user.isEmpty())
		user = qEnvironmentVariable("USERNAME");	// Windows
	d->author = group.readEntry("Author", user);

	//we don't have direct access to the members name and comment
	//->temporary disable the undo stack and call the setters
	setUndoAware(false);
	setIsLoading(true);
	setName(group.readEntry("Name", i18n("Project")));
	setComment(group.readEntry("Comment", QString()));
	setUndoAware(true);
	setIsLoading(false);
	d->changed = false;

	connect(this, &Project::aspectDescriptionChanged,this, &Project::descriptionChanged);
	connect(this, &Project::aspectAdded,this, &Project::aspectAddedSlot);
}

Project::~Project() {
	//if the project is being closed and the live data sources still continue reading the data,
	//the dependent objects (columns, etc.), which are already deleted maybe here,  are still being notified about the changes.
	//->stop reading the live data sources prior to deleting all objects.
	for (auto* lds : children<LiveDataSource>())
		lds->pauseReading();

#ifdef HAVE_MQTT
	for (auto* client : children<MQTTClient>())
		client->pauseReading();
#endif

	//if the project is being closed, in Worksheet the scene items are being removed and the selection in the view can change.
	//don't react on these changes since this can lead crashes (worksheet object is already in the destructor).
	//->notify all worksheets about the project being closed.
	for (auto* w : children<Worksheet>(ChildIndexFlag::Recursive))
		w->setIsClosing();

	d->undo_stack.clear();
	delete d;
}

QString Project::version() {
	return Private::version();
}

int Project::versionNumber() {
	return Private::versionNumber();
}

int Project::xmlVersion() {
	return Private::xmlVersion();
}

QUndoStack* Project::undoStack() const {
	return &d->undo_stack;
}

QMenu* Project::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();

	//add close action
	menu->addSeparator();
	menu->addAction(QIcon::fromTheme(QLatin1String("document-close")), i18n("Close"), this, SIGNAL(closeRequested()));

	//add the actions from MainWin
	emit requestProjectContextMenu(menu);

	return menu;
}

QMenu* Project::createFolderContextMenu(const Folder* folder) {
	QMenu* menu = const_cast<Folder*>(folder)->AbstractAspect::createContextMenu();
	emit requestFolderContextMenu(folder, menu);
	return menu;
}

void Project::setMdiWindowVisibility(MdiWindowVisibility visibility) {
	d->mdiWindowVisibility = visibility;
	emit mdiWindowVisibilityChanged();
}

Project::MdiWindowVisibility Project::mdiWindowVisibility() const {
	return d->mdiWindowVisibility;
}

CLASS_D_ACCESSOR_IMPL(Project, QString, fileName, FileName, fileName)
BASIC_D_READER_IMPL(Project, QString, author, author)
CLASS_D_ACCESSOR_IMPL(Project, QDateTime, modificationTime, ModificationTime, modificationTime)

STD_SETTER_CMD_IMPL_S(Project, SetAuthor, QString, author)
void Project::setAuthor(const QString& author) {
	if (author != d->author)
		exec(new ProjectSetAuthorCmd(d, author, ki18n("%1: set author")));
}

void Project::setChanged(const bool value) {
	if (isLoading())
		return;

	d->changed = value;

	if (value)
		emit changed();
}

void Project::setSuppressAspectAddedSignal(bool value) {
	d->aspectAddedSignalSuppressed = value;
}

bool Project::aspectAddedSignalSuppressed() const {
	return d->aspectAddedSignalSuppressed;
}

bool Project::hasChanged() const {
	return d->changed;
}

/*!
 * \brief Project::descriptionChanged
 * This function is called, when an object changes its name. When a column changed its name and wasn't connected before to the curve/column(formula) then
 * this is done in this function
 * \param aspect
 */
void Project::descriptionChanged(const AbstractAspect* aspect) {
	if (isLoading())
		return;

	//when the name of a column is being changed, it can match again the names being used in the curves, etc.
	//and we need to update the dependencies
	const auto* column = dynamic_cast<const AbstractColumn*>(aspect);
	if (column) {
		const auto& curves = children<XYCurve>(ChildIndexFlag::Recursive);
		updateColumnDependencies(curves, column);

		const auto& histograms = children<Histogram>(ChildIndexFlag::Recursive);
		updateColumnDependencies(histograms, column);

		const auto& boxPlots = children<BoxPlot>(ChildIndexFlag::Recursive);
		updateColumnDependencies(boxPlots, column);
	}

	d->changed = true;
	emit changed();
}

/*!
 * \brief Project::aspectAddedSlot
 * When adding new columns, these should be connected to the corresponding curves
 * \param aspect
 */
void Project::aspectAddedSlot(const AbstractAspect* aspect) {
	if(isLoading())
		return;

	//check whether new columns were added and if yes,
	//update the dependencies in the project
	QVector<const AbstractColumn*> columns;
	const auto* column = dynamic_cast<const AbstractColumn*>(aspect);
	if (column)
		columns.append(column);
	else {
		for (auto* child : aspect->children<Column>(ChildIndexFlag::Recursive))
			columns.append(static_cast<const AbstractColumn*>(child));
	}

	if (columns.isEmpty())
		return;

	//if a new column was addded, check whether the column names match the missing
	//names in the curves, etc. and update the dependencies
	const auto& curves = children<XYCurve>(ChildIndexFlag::Recursive);
	for (auto column : columns)
		updateColumnDependencies(curves, column);

	const auto& histograms = children<Histogram>(ChildIndexFlag::Recursive);
	for (auto column : columns)
		updateColumnDependencies(histograms, column);


	const auto& boxPlots = children<BoxPlot>(ChildIndexFlag::Recursive);
	for (auto column : columns)
		updateColumnDependencies(boxPlots, column);
}

//TODO: move this update*() functions into the classes, Project shouldn't be aware of the details
void Project::updateColumnDependencies(const QVector<XYCurve*>& curves, const AbstractColumn* column) const {
	const QString& columnPath = column->path();

	// setXColumnPath must not be set, because if curve->column matches column, there already exist a
	// signal/slot connection between the curve and the column to update this. If they are not same,
	// xColumnPath is set in setXColumn. Same for the yColumn.
	for (auto* curve : curves) {
		curve->setUndoAware(false);
		auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(curve);
		if (analysisCurve) {
			if (analysisCurve->xDataColumnPath() == columnPath)
				analysisCurve->setXDataColumn(column);
			if (analysisCurve->yDataColumnPath() == columnPath)
				analysisCurve->setYDataColumn(column);
			if (analysisCurve->y2DataColumnPath() == columnPath)
				analysisCurve->setY2DataColumn(column);

			auto* fitCurve = dynamic_cast<XYFitCurve*>(curve);
			if (fitCurve) {
				if (fitCurve->xErrorColumnPath() == columnPath)
					fitCurve->setXErrorColumn(column);
				if (fitCurve->yErrorColumnPath() == columnPath)
					fitCurve->setYErrorColumn(column);
			}
		} else {
			if (curve->xColumnPath() == columnPath)
				curve->setXColumn(column);
			if (curve->yColumnPath() == columnPath)
				curve->setYColumn(column);
			if (curve->valuesColumnPath() == columnPath)
				curve->setValuesColumn(column);
			if (curve->xErrorPlusColumnPath() == columnPath)
				curve->setXErrorPlusColumn(column);
			if (curve->xErrorMinusColumnPath() == columnPath)
				curve->setXErrorMinusColumn(column);
			if (curve->yErrorPlusColumnPath() == columnPath)
				curve->setYErrorPlusColumn(column);
			if (curve->yErrorMinusColumnPath() == columnPath)
				curve->setYErrorMinusColumn(column);
		}

		if (curve->valuesColumnPath() == columnPath)
			curve->setValuesColumn(column);

		curve->setUndoAware(true);
	}

	const QVector<Column*>& columns = children<Column>(ChildIndexFlag::Recursive);
	for (auto* tempColumn : columns) {
		const QStringList& paths = tempColumn->formulaVariableColumnPaths();
		for (int i = 0; i < paths.count(); i++) {
			if (paths.at(i) == columnPath)
				tempColumn->setformulVariableColumn(i, const_cast<Column*>(static_cast<const Column*>(column)));
		}
	}
}

void Project::updateColumnDependencies(const QVector<Histogram*>& histograms, const AbstractColumn* column) const {
	const QString& columnPath = column->path();
	for (auto* histogram : histograms) {
		if (histogram->dataColumnPath() == columnPath) {
			histogram->setUndoAware(false);
			histogram->setDataColumn(column);
			histogram->setUndoAware(true);
		}

		if (histogram->valuesColumnPath() == columnPath) {
			histogram->setUndoAware(false);
			histogram->setValuesColumn(column);
			histogram->setUndoAware(true);
		}
	}
}

void Project::updateColumnDependencies(const QVector<BoxPlot*>& boxPlots, const AbstractColumn* column) const {
	const QString& columnPath = column->path();
	for (auto* boxPlot : boxPlots) {
		const auto dataColumnPaths = boxPlot->dataColumnPaths();
		auto dataColumns = boxPlot->dataColumns();
		bool changed = false;
		for (int i = 0; i < dataColumnPaths.count(); ++i) {
			const auto& path = dataColumnPaths.at(i);

			if (path == columnPath) {
				dataColumns[i] = column;
				changed = true;
			}
		}

		if (changed) {
			boxPlot->setUndoAware(false);
			boxPlot->setDataColumns(dataColumns);
			boxPlot->setUndoAware(true);
		}
	}
}

void Project::navigateTo(const QString& path) {
	emit requestNavigateTo(path);
}

bool Project::isLabPlotProject(const QString& fileName) {
	return fileName.endsWith(QStringLiteral(".lml"), Qt::CaseInsensitive)
			|| fileName.endsWith(QStringLiteral(".lml.gz"), Qt::CaseInsensitive)
			|| fileName.endsWith(QStringLiteral(".lml.bz2"), Qt::CaseInsensitive)
			|| fileName.endsWith(QStringLiteral(".lml.xz"), Qt::CaseInsensitive);
}

QString Project::supportedExtensions() {
	static const QString extensions = "*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ";
	return extensions;
}

QVector<quintptr> Project::droppedAspects(const QMimeData* mimeData) {
	QByteArray data = mimeData->data(QLatin1String("labplot-dnd"));
	QDataStream stream(&data, QIODevice::ReadOnly);

	//read the project pointer first
	quintptr project = 0;
	stream >> project;

	//read the pointers of the dragged aspects
	QVector<quintptr> vec;
	stream >> vec;

	return vec;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

void Project::save(const QPixmap& thumbnail, QXmlStreamWriter* writer) const {
	//set the version and the modification time to the current values
	d->setVersion(LVERSION);
	d->modificationTime = QDateTime::currentDateTime();

	writer->setAutoFormatting(true);
	writer->writeStartDocument();
	writer->writeDTD("<!DOCTYPE LabPlotXML>");

	writer->writeStartElement("project");
	writer->writeAttribute("version", version());
	writer->writeAttribute("xmlVersion", QString::number(buildXmlVersion));
	writer->writeAttribute("modificationTime", modificationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("author", author());

	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);
	QPixmap scaledThumbnail = thumbnail.scaled(512,512, Qt::KeepAspectRatio);
	scaledThumbnail.save(&buffer, "JPEG");
	QString image = QString::fromLatin1(bArray.toBase64().data());
	writer->writeAttribute("thumbnail", image);

	writeBasicAttributes(writer);

	writeCommentElement(writer);

	save(writer);
}

/**
 * \brief Save as XML
 */
void Project::save(QXmlStreamWriter* writer) const {
	//save all children
	const auto& children = this->children<AbstractAspect>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children) {
		writer->writeStartElement("child_aspect");
		child->save(writer);
		writer->writeEndElement();
	}

	//save the state of the views (visible, maximized/minimized/geometry)
	//and the state of the project explorer (expanded items, currently selected item)
	emit requestSaveState(writer);

	writer->writeEndElement();
	writer->writeEndDocument();
}

bool Project::load(const QString& filename, bool preview) {
	QIODevice* file;
	// first try gzip compression, because projects can be gzipped and end with .lml
	if (filename.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		file = new KCompressionDevice(filename,KFilterDev::compressionTypeForMimeType("application/x-gzip"));
	else	// opens filename using file ending
		file = new KFilterDev(filename);

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

	//parse XML
	XmlStreamReader reader(file);
	setIsLoading(true);
	rc = this->load(&reader, preview);
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

//TODO: show warnings in a kind of "log window" but not in message box
// 		KMessageBox::error(this, msg, i18n("Project loading partly failed"));
	}

	if (reader.hasMissingCASWarnings()) {
		RESET_CURSOR;

		const QString& msg = i18n("The project has content written with %1. "
						"Your installation of LabPlot lacks the support for it.\n\n "
						"You won't be able to see this part of the project. "
						"If you modify and save the project, the CAS content will be lost.\n\n"
						"Do you want to continue?", reader.missingCASWarning());
		auto rc = KMessageBox::warningYesNo(nullptr, msg, i18n("Missing Support for CAS"));
		if (rc == KMessageBox::ButtonCode::No) {
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
	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();

	if (!(reader->atEnd())) {
		if (!reader->skipToNextTag())
			return false;

		if (reader->name() == "project") {
			QString version = reader->attributes().value("version").toString();
			if (version.isEmpty())
				reader->raiseWarning(i18n("Attribute 'version' is missing."));
			else
				d->setVersion(version);

			QString c = reader->attributes().value("xmlVersion").toString();
			if (c.isEmpty())
				d->mXmlVersion = 0;
			else
				d->mXmlVersion = c.toInt();

			if (!readBasicAttributes(reader)) return false;
			if (!readProjectAttributes(reader)) return false;

			while (!reader->atEnd()) {
				reader->readNext();

				if (reader->isEndElement()) break;

				if (reader->isStartElement()) {
					if (reader->name() == "comment") {
						if (!readCommentElement(reader))
							return false;
					} else if (reader->name() == "child_aspect") {
						if (!readChildAspectElement(reader, preview))
							return false;
					} else if (!preview && reader->name() == "state") {
						//load the state of the views (visible, maximized/minimized/geometry)
						//and the state of the project explorer (expanded items, currently selected item)
						emit requestLoadState(reader);
					} else {
						if (!preview)
							reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
						if (!reader->skipToEndElement()) return false;
					}
				}
			}
		} else  // no project element
			reader->raiseError(i18n("no project element found"));
	} else  // no start document
		reader->raiseError(i18n("no valid XML document found"));

	//everything is read now, restore the pointers
	restorePointers(this, preview);

	emit loaded();
	return !reader->hasError();
}

/*!
 * this function is used to restore the pointers to to the columns in xy-curves etc.
 * from the stored column paths. This function is called after the project was loaded
 * and when an aspect is being pasted. In both cases we deserialized from XML and need
 * to restore the pointers.
 */
void Project::restorePointers(AbstractAspect* aspect, bool preview) {
	//wait until all columns are decoded from base64-encoded data
	QThreadPool::globalInstance()->waitForDone();

	bool hasChildren = aspect->childCount<AbstractAspect>();
	const auto& columns = aspect->project()->children<Column>(ChildIndexFlag::Recursive);

	//LiveDataSource:
	//call finalizeLoad() to replace relative with absolute paths if required
	//and to create columns during the initial read
	const auto& sources = aspect->children<LiveDataSource>(ChildIndexFlag::Recursive);
	for (auto* source : sources) {
		if (!source) continue;
		source->finalizeLoad();
	}

	//xy-curves
	// cannot be removed by the column observer, because it does not react
	// on curve changes
	QVector<XYCurve*> curves;
	if (hasChildren)
		curves = aspect->children<XYCurve>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::XYCurve || aspect->inherits(AspectType::XYAnalysisCurve))
		//the object doesn't have any children -> one single aspect is being pasted.
		//check whether the object being pasted is a XYCurve and add it to the
		//list of curves to be retransformed
		curves << static_cast<XYCurve*>(aspect);

	for (auto* curve : qAsConst(curves)) {
		if (!curve) continue;
		curve->suppressRetransform(true);

		auto* equationCurve = dynamic_cast<XYEquationCurve*>(curve);
		auto* analysisCurve = dynamic_cast<XYAnalysisCurve*>(curve);
		if (equationCurve) {
			//curves defined by a mathematical equations recalculate their own columns on load again.
			equationCurve->recalculate();
		} else if (analysisCurve) {
			RESTORE_COLUMN_POINTER(analysisCurve, xDataColumn, XDataColumn);
			RESTORE_COLUMN_POINTER(analysisCurve, yDataColumn, YDataColumn);
			RESTORE_COLUMN_POINTER(analysisCurve, y2DataColumn, Y2DataColumn);
			auto* fitCurve = dynamic_cast<XYFitCurve*>(curve);
			if (fitCurve) {
				RESTORE_COLUMN_POINTER(fitCurve, xErrorColumn, XErrorColumn);
				RESTORE_COLUMN_POINTER(fitCurve, yErrorColumn, YErrorColumn);
			}
		} else {
			RESTORE_COLUMN_POINTER(curve, xColumn, XColumn);
			RESTORE_COLUMN_POINTER(curve, yColumn, YColumn);
			RESTORE_COLUMN_POINTER(curve, valuesColumn, ValuesColumn);
			RESTORE_COLUMN_POINTER(curve, xErrorPlusColumn, XErrorPlusColumn);
			RESTORE_COLUMN_POINTER(curve, xErrorMinusColumn, XErrorMinusColumn);
			RESTORE_COLUMN_POINTER(curve, yErrorPlusColumn, YErrorPlusColumn);
			RESTORE_COLUMN_POINTER(curve, yErrorMinusColumn, YErrorMinusColumn);
		}
		if (dynamic_cast<XYAnalysisCurve*>(curve))
			RESTORE_POINTER(dynamic_cast<XYAnalysisCurve*>(curve), dataSourceCurve, DataSourceCurve, XYCurve, curves);

		curve->suppressRetransform(false);
	}

	// assign to all markers the curves they need
	QVector<InfoElement*> elements;
	if (aspect->type() == AspectType::InfoElement) //check for the type first. InfoElement has children, but they are not relevant here
		elements << static_cast<InfoElement*>(aspect);
	else if (hasChildren)
		elements = aspect->children<InfoElement>(ChildIndexFlag::Recursive);

	for (auto* element : elements)
		element->assignCurve(curves);

	//axes
	QVector<Axis*> axes;
	if (hasChildren)
		axes = aspect->children<Axis>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::Axis)
		axes << static_cast<Axis*>(aspect);

	for (auto* axis : axes) {
		if (!axis) continue;
		RESTORE_COLUMN_POINTER(axis, majorTicksColumn, MajorTicksColumn);
		RESTORE_COLUMN_POINTER(axis, minorTicksColumn, MinorTicksColumn);
		RESTORE_COLUMN_POINTER(axis, labelsTextColumn, LabelsTextColumn);
	}

	//histograms
	QVector<Histogram*> hists;
	if (hasChildren)
		hists = aspect->children<Histogram>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::Histogram)
		hists << static_cast<Histogram*>(aspect);

	for (auto* hist : hists) {
		if (!hist) continue;
		RESTORE_COLUMN_POINTER(hist, dataColumn, DataColumn);
		RESTORE_COLUMN_POINTER(hist, valuesColumn, ValuesColumn);
	}

	//box plots
	QVector<BoxPlot*> boxPlots;
	if (hasChildren)
		boxPlots = aspect->children<BoxPlot>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::BoxPlot)
		boxPlots << static_cast<BoxPlot*>(aspect);

	for (auto* boxPlot : boxPlots) {
		if (!boxPlot) continue;

		//initialize the array for the column pointers
		int count = boxPlot->dataColumnPaths().count();
		QVector<const AbstractColumn*> dataColumns;
		dataColumns.resize(count);

		//restore the pointers
		for (int i = 0; i < count; ++i) {
			dataColumns[i] = nullptr;
			const auto& path = boxPlot->dataColumnPaths().at(i);
			for (Column* column : columns) {
				if (!column) continue;
				if (column->path() == path) {
					dataColumns[i] = column;
					break;
				}
			}
		}

		boxPlot->setDataColumns(dataColumns);
	}

	//data picker curves
	QVector<DatapickerCurve*> dataPickerCurves;
	if (hasChildren)
		dataPickerCurves = aspect->children<DatapickerCurve>(ChildIndexFlag::Recursive);
	else if (aspect->type() == AspectType::DatapickerCurve)
		dataPickerCurves << static_cast<DatapickerCurve*>(aspect);

	for (auto* dataPickerCurve : dataPickerCurves) {
		if (!dataPickerCurve) continue;
		RESTORE_COLUMN_POINTER(dataPickerCurve, posXColumn, PosXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, posYColumn, PosYColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaXColumn, PlusDeltaXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaXColumn, MinusDeltaXColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaYColumn, PlusDeltaYColumn);
		RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaYColumn, MinusDeltaYColumn);
	}

	//if a column was calculated via a formula, restore the pointers to the variable columns defining the formula
	for (auto* col : columns) {
		if (!col->formulaVariableColumnPaths().isEmpty()) {
			auto& formulaVariableColumns = const_cast<QVector<Column*>&>(col->formulaVariableColumns());
			formulaVariableColumns.resize(col->formulaVariableColumnPaths().length());

			for (int i = 0; i < col->formulaVariableColumnPaths().length(); i++) {
				auto path = col->formulaVariableColumnPaths()[i];
				for (Column* c : columns) {
					if (!c) continue;
					if (c->path() == path) {
						formulaVariableColumns[i] = c;
						col->finalizeLoad();
						break;
					}
				}
			}
		}
	}

	if (preview)
		return;

	//all data was read in spreadsheets:
	//call CartesianPlot::retransform() to retransform the plots
	QVector<CartesianPlot*> plots;
	if (hasChildren && aspect->type() != AspectType::CartesianPlot)
		plots = aspect->children<CartesianPlot>(ChildIndexFlag::Recursive);
	else {
		if (aspect->type() == AspectType::CartesianPlot)
			plots << static_cast<CartesianPlot*>(aspect);
		else if (aspect->inherits(AspectType::XYCurve) || aspect->type() == AspectType::Histogram)
			plots << static_cast<CartesianPlot*>(aspect->parentAspect());
	}

	for (auto* plot : plots) {
		plot->setIsLoading(false);
		plot->retransform();
	}

	//all data was read in live-data sources:
	//call CartesianPlot::dataChanged() to notify affected plots about the new data.
	//this needs to be done here since in LiveDataSource::finalizeImport() called above
	//where the data is read the column pointers are not restored yes in curves.
	plots.clear();
	for (auto* source : sources) {
		for (int n = 0; n < source->columnCount(); ++n) {
			Column* column = source->column(n);

			//determine the plots where the column is consumed
			for (const auto* curve : curves) {
				if (curve->xColumn() == column || curve->yColumn() == column) {
					auto* plot = static_cast<CartesianPlot*>(curve->parentAspect());
					if (plots.indexOf(plot) == -1) {
						plots << plot;
						plot->setSuppressDataChangedSignal(true);
					}
				}
			}

			column->setChanged();
		}
	}

	//loop over all affected plots and retransform them
	for (auto* plot : plots) {
		plot->setSuppressDataChangedSignal(false);
		plot->dataChanged(-1, -1);
	}
}

bool Project::readProjectAttributes(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "modificationTime").toString();
	QDateTime modificationTime = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if (str.isEmpty() || !modificationTime.isValid()) {
		reader->raiseWarning(i18n("Invalid project modification time. Using current time."));
		d->modificationTime = QDateTime::currentDateTime();
	} else
		d->modificationTime = modificationTime;

	d->author = attribs.value(reader->namespaceUri().toString(), "author").toString();

	return true;
}
