/*
	File                 : NetCDFFilter.cpp
	Project              : LabPlot
	Description          : NetCDF I/O-filter
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2019 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NetCDFFilter.h"
#include "NetCDFFilterPrivate.h"
#include "backend/core/column/Column.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/hostprocess.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <QProcess>
#include <QStandardPaths>

///////////// macros ///////////////////////////////////////////////

#define NC_GET_ATT(type, ftype)                                                                                                                                \
	auto* value = (type*)malloc(len * sizeof(type));                                                                                                           \
	m_status = nc_get_att_##ftype(ncid, varid, name, value);                                                                                                   \
	handleError(m_status, QStringLiteral("nc_get_att_" #ftype));                                                                                               \
	for (unsigned int l = 0; l < len; l++)                                                                                                                     \
		valueString << QString::number(value[l]);                                                                                                              \
	free(value);

#define NC_SCAN_VAR(type, ftype)                                                                                                                               \
	type data;                                                                                                                                                 \
	m_status = nc_get_var_##ftype(ncid, i, &data);                                                                                                             \
	handleError(m_status, QStringLiteral("nc_get_var_" #ftype));                                                                                               \
	rowStrings << QString::number(data);

#define NC_READ_VAR(type, ftype, dtype)                                                                                                                        \
	type data;                                                                                                                                                 \
	m_status = nc_get_var_##ftype(ncid, varid, &data);                                                                                                         \
	handleError(m_status, QStringLiteral("nc_get_var_" #ftype));                                                                                               \
                                                                                                                                                               \
	if (dataSource) {                                                                                                                                          \
		dtype* sourceData = static_cast<QVector<dtype>*>(dataContainer[0])->data();                                                                            \
		sourceData[0] = (dtype)data;                                                                                                                           \
	} else { /* preview */                                                                                                                                     \
		dataStrings << (QStringList() << QString::number(data));                                                                                               \
	}

#define NC_READ_AVAR(type, ftype, dtype)                                                                                                                       \
	auto* data = new type[(unsigned int)actualRows];                                                                                                           \
                                                                                                                                                               \
	size_t start = (size_t)(startRow - 1), count = (size_t)actualRows;                                                                                         \
	m_status = nc_get_vara_##ftype(ncid, varid, &start, &count, data);                                                                                         \
	handleError(m_status, QStringLiteral("nc_get_vara_" #ftype));                                                                                              \
                                                                                                                                                               \
	if (dataSource) {                                                                                                                                          \
		dtype* sourceData = static_cast<QVector<dtype>*>(dataContainer[0])->data();                                                                            \
		for (int i = 0; i < actualRows; i++)                                                                                                                   \
			sourceData[i] = (dtype)data[i];                                                                                                                    \
	} else { /* preview */                                                                                                                                     \
		for (int i = 0; i < std::min(actualRows, lines); i++)                                                                                                  \
			dataStrings << (QStringList() << QString::number(data[i]));                                                                                        \
	}                                                                                                                                                          \
	delete[] data;

// for native types (atm: int, double)
#define NC_READ_AVAR_NATIVE(type)                                                                                                                              \
	type* data = nullptr;                                                                                                                                      \
	if (dataSource)                                                                                                                                            \
		data = static_cast<QVector<type>*>(dataContainer[0])->data();                                                                                          \
	else                                                                                                                                                       \
		data = new type[(unsigned int)actualRows];                                                                                                             \
                                                                                                                                                               \
	size_t start = (size_t)(startRow - 1), count = (size_t)actualRows;                                                                                         \
	m_status = nc_get_vara_##type(ncid, varid, &start, &count, data);                                                                                          \
	handleError(m_status, QStringLiteral("nc_get_vara_" #type));                                                                                               \
                                                                                                                                                               \
	if (!dataSource) { /* preview */                                                                                                                           \
		for (int i = 0; i < std::min(actualRows, lines); i++)                                                                                                  \
			dataStrings << (QStringList() << QString::number(data[i]));                                                                                        \
		delete[] data;                                                                                                                                         \
	}

#define NC_READ_VAR2(type, ftype, dtype)                                                                                                                       \
	auto** data = (type**)malloc(rows * sizeof(type*));                                                                                                        \
	data[0] = (type*)malloc(cols * rows * sizeof(type));                                                                                                       \
	for (unsigned int i = 1; i < rows; i++)                                                                                                                    \
		data[i] = data[0] + i * cols;                                                                                                                          \
                                                                                                                                                               \
	m_status = nc_get_var_##ftype(ncid, varid, &data[0][0]);                                                                                                   \
	handleError(m_status, QStringLiteral("nc_get_var_" #ftype));                                                                                               \
                                                                                                                                                               \
	if (m_status == NC_NOERR) {                                                                                                                                \
		for (int i = 0; i < std::min((int)rows, lines); i++) {                                                                                                 \
			QStringList line;                                                                                                                                  \
			for (size_t j = 0; j < cols; j++) {                                                                                                                \
				if (dataSource && dataContainer[0])                                                                                                            \
					static_cast<QVector<dtype>*>(dataContainer[(int)(j - (size_t)startColumn + 1)])->operator[](i - startRow + 1) = data[i][(int)j];           \
				else                                                                                                                                           \
					line << QString::number(data[i][j]);                                                                                                       \
			}                                                                                                                                                  \
			dataStrings << line;                                                                                                                               \
			Q_EMIT q->completed(100 * i / actualRows);                                                                                                         \
		}                                                                                                                                                      \
	}                                                                                                                                                          \
	free(data[0]);                                                                                                                                             \
	free(data);

//////////////////////////////////////////////////////////////////////

/*!
	\class NetCDFFilter
	\brief Manages the import/export of data from/to a NetCDF file.

	\ingroup datasources
*/
NetCDFFilter::NetCDFFilter()
	: AbstractFileFilter(FileType::NETCDF)
	, d(new NetCDFFilterPrivate(this)) {
}

NetCDFFilter::~NetCDFFilter() = default;

/*!
  parses the content of the file \c ileName.
*/
void NetCDFFilter::parse(const QString& fileName, QTreeWidgetItem* rootItem) {
	d->parse(fileName, rootItem);
}

/*!
  reads the content of the selected attribute from file \c fileName.
*/
QString NetCDFFilter::readAttribute(const QString& fileName, const QString& name, const QString& varName) {
	return d->readAttribute(fileName, name, varName);
}

/*!
  reads the content of the current variable from file \c fileName.
*/
QVector<QStringList> NetCDFFilter::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource, ImportMode importMode, int lines) {
	return d->readCurrentVar(fileName, dataSource, importMode, lines);
}

/*!
  reads the content of the file \c fileName to the data source \c dataSource.
*/
void NetCDFFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, ImportMode mode) {
	d->readDataFromFile(fileName, dataSource, mode);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void NetCDFFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
	// 	emit()
}

///////////////////////////////////////////////////////////////////////

void NetCDFFilter::setCurrentVarName(const QString& ds) {
	d->currentVarName = ds;
}

const QString NetCDFFilter::currentVarName() const {
	return d->currentVarName;
}

void NetCDFFilter::setStartRow(const int s) {
	d->startRow = s;
}

int NetCDFFilter::startRow() const {
	return d->startRow;
}

void NetCDFFilter::setEndRow(const int e) {
	d->endRow = e;
}

int NetCDFFilter::endRow() const {
	return d->endRow;
}

void NetCDFFilter::setStartColumn(const int c) {
	d->startColumn = c;
}

int NetCDFFilter::startColumn() const {
	return d->startColumn;
}

void NetCDFFilter::setEndColumn(const int c) {
	d->endColumn = c;
}

int NetCDFFilter::endColumn() const {
	return d->endColumn;
}

QString NetCDFFilter::fileInfoString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO << ", fileName = " << qPrintable(fileName))

	QString info;
#ifdef HAVE_NETCDF
	int ncid, status;
	status = nc_open(qPrintable(fileName), NC_NOWRITE, &ncid);
	NetCDFFilterPrivate::handleError(status, QStringLiteral("nc_open"));
	if (status != NC_NOERR) {
		DEBUG("	File error. Giving up");
		return i18n("Error opening file");
	}

	int ndims, nvars, nattr, uldid;
	status = nc_inq(ncid, &ndims, &nvars, &nattr, &uldid);
	NetCDFFilterPrivate::handleError(status, QStringLiteral("nc_inq"));
	DEBUG(" nattr/ndims/nvars = " << nattr << ' ' << ndims << ' ' << nvars);

	if (status == NC_NOERR) {
		info += i18n("Number of global attributes: %1", QString::number(nattr));
		info += QStringLiteral("<br>");
		info += i18n("Number of dimensions: %1", QString::number(ndims));
		info += QStringLiteral("<br>");
		info += i18n("Number of variables: %1", QString::number(nvars));
		info += QStringLiteral("<br>");

		int format;
		status = nc_inq_format(ncid, &format);
		if (status == NC_NOERR)
			info += i18n("Format version: %1", NetCDFFilterPrivate::translateFormat(format));
		info += QStringLiteral("<br>");

		info += i18n("Using library version %1", QLatin1String(nc_inq_libvers()));
	} else {
		info += i18n("Error getting file info");
	}

	status = ncclose(ncid);
	NetCDFFilterPrivate::handleError(status, QStringLiteral("nc_close"));
#else
	Q_UNUSED(fileName)
#endif

	return info;
}

/*!
 * Get file content in CDL (Common Data form Language) format
 * uses "ncdump"
 */
QString NetCDFFilter::fileCDLString(const QString& fileName) {
	DEBUG(Q_FUNC_INFO);

	QString CDLString;
#ifdef Q_OS_LINUX
	const QString ncdumpFullPath = safeExecutableName(QStringLiteral("ncdump"));
	if (ncdumpFullPath.isEmpty())
		return i18n("ncdump not found.");

	QProcess proc;
	QStringList args;
	args << QStringLiteral("-cs") << fileName;
	startHostProcess(proc, ncdumpFullPath, args);

	if (proc.waitForReadyRead(1000) == false)
		CDLString += i18n("Reading from file %1 failed.", fileName);
	else {
		CDLString += QLatin1String(proc.readAll());
		CDLString.replace(QLatin1Char('\n'), QStringLiteral("<br>\n"));
		CDLString.replace(QLatin1Char('\t'), QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;"));
		// DEBUG("	CDL string: " << STDSTRING(CDLString));
	}
#else // TODO: ncdump on Win, Mac
	Q_UNUSED(fileName)
#endif

	return CDLString;
}

// #####################################################################
// ################### Private implementation ##########################
// #####################################################################

NetCDFFilterPrivate::NetCDFFilterPrivate(NetCDFFilter* owner)
	: q(owner) {
#ifdef HAVE_NETCDF
	m_status = 0;
#endif
}

#ifdef HAVE_NETCDF
void NetCDFFilterPrivate::handleError(int err, const QString& function) {
	Q_UNUSED(function);
	if (err != NC_NOERR) {
		DEBUG("NETCDF ERROR:" << STDSTRING(function) << "() - " << nc_strerror(err));
		// TODO q->setLastError(i18n("NetCDF Error: %1() - %2", function, nc_strerror(err)));
		return;
	}
}

QString NetCDFFilterPrivate::translateDataType(nc_type type) {
	QString typeString;

	switch (type) {
	case NC_BYTE:
		typeString = QStringLiteral("BYTE");
		break;
	case NC_UBYTE:
		typeString = QStringLiteral("UBYTE");
		break;
	case NC_CHAR:
		typeString = QStringLiteral("CHAR");
		break;
	case NC_SHORT:
		typeString = QStringLiteral("SHORT");
		break;
	case NC_USHORT:
		typeString = QStringLiteral("USHORT");
		break;
	case NC_INT:
		typeString = QStringLiteral("INT");
		break;
	case NC_UINT:
		typeString = QStringLiteral("UINT");
		break;
	case NC_INT64:
		typeString = QStringLiteral("INT64");
		break;
	case NC_UINT64:
		typeString = QStringLiteral("UINT64");
		break;
	case NC_FLOAT:
		typeString = QStringLiteral("FLOAT");
		break;
	case NC_DOUBLE:
		typeString = QStringLiteral("DOUBLE");
		break;
	case NC_STRING:
		typeString = QStringLiteral("STRING");
		break;
	default:
		typeString = QStringLiteral("UNKNOWN");
	}

	return typeString;
}

QString NetCDFFilterPrivate::translateFormat(int format) {
	QString formatString;

	switch (format) {
	case NC_FORMAT_CLASSIC:
		formatString = QStringLiteral("NC_FORMAT_CLASSIC");
		break;
	case NC_FORMAT_64BIT:
		formatString = QStringLiteral("NC_FORMAT_64BIT");
		break;
	case NC_FORMAT_NETCDF4:
		formatString = QStringLiteral("NC_FORMAT_NETCDF4");
		break;
	case NC_FORMAT_NETCDF4_CLASSIC:
		formatString = QStringLiteral("NC_FORMAT_NETCDF4_CLASSIC");
		break;
	}

	return formatString;
}

QString NetCDFFilterPrivate::scanAttrs(int ncid, int varid, int attid, QTreeWidgetItem* parentItem) {
	char name[NC_MAX_NAME + 1];

	int nattr, nstart = 0;
	if (attid == -1) {
		m_status = nc_inq_varnatts(ncid, varid, &nattr);
		handleError(m_status, QStringLiteral("nc_inq_varnatts"));
	} else {
		nstart = attid;
		nattr = attid + 1;
	}

	nc_type type;
	size_t len;
	QStringList valueString;
	for (int i = nstart; i < nattr; i++) {
		valueString.clear();
		m_status = nc_inq_attname(ncid, varid, i, name);
		handleError(m_status, QStringLiteral("nc_inq_attname"));

		m_status = nc_inq_att(ncid, varid, name, &type, &len);
		handleError(m_status, QStringLiteral("nc_inq_att"));
		QDEBUG("	attr" << i + 1 << "name/type/len =" << name << translateDataType(type) << len);

		// read attribute
		switch (type) {
		case NC_BYTE: {
			NC_GET_ATT(signed char, schar);
			break;
		}
		case NC_UBYTE: {
			NC_GET_ATT(unsigned char, uchar);
			break;
		}
		case NC_CHAR: { // not number
			char* value = (char*)malloc((len + 1) * sizeof(char));
			m_status = nc_get_att_text(ncid, varid, name, value);
			handleError(m_status, QStringLiteral("nc_get_att_text"));
			value[len] = 0;
			valueString << QLatin1String(value);
			free(value);
			break;
		}
		case NC_SHORT: {
			NC_GET_ATT(short, short);
			break;
		}
		case NC_USHORT: {
			NC_GET_ATT(unsigned short, ushort);
			break;
		}
		case NC_INT: {
			NC_GET_ATT(int, int);
			break;
		}
		case NC_UINT: {
			NC_GET_ATT(unsigned int, uint);
			break;
		}
		case NC_INT64: {
			NC_GET_ATT(long long, longlong);
			break;
		}
		case NC_UINT64: {
			NC_GET_ATT(unsigned long long, ulonglong);
			break;
		}
		case NC_FLOAT: {
			NC_GET_ATT(float, float);
			break;
		}
		case NC_DOUBLE: {
			NC_GET_ATT(double, double);
			break;
		}
		// TODO: NC_STRING
		default:
			valueString << QStringLiteral("not supported");
		}

		if (parentItem != nullptr) {
			QString typeName;
			if (varid == NC_GLOBAL)
				typeName = i18n("global attribute");
			else {
				char varName[NC_MAX_NAME + 1];
				m_status = nc_inq_varname(ncid, varid, varName);
				typeName = i18n("%1 attribute", QLatin1String(varName));
			}
			QStringList props;
			props << translateDataType(type) << QStringLiteral(" (") << QString::number(len) << QStringLiteral(")");
			auto* attrItem =
				new QTreeWidgetItem(QStringList() << QLatin1String(name) << typeName << props.join(QString()) << valueString.join(QStringLiteral(", ")));
			attrItem->setIcon(0, QIcon::fromTheme(QStringLiteral("accessories-calculator")));
			attrItem->setFlags(Qt::ItemIsEnabled);
			parentItem->addChild(attrItem);
		}
	}

	return valueString.join(QLatin1Char('\n'));
}

void NetCDFFilterPrivate::scanDims(int ncid, int ndims, QTreeWidgetItem* parentItem) {
	int ulid;
	m_status = nc_inq_unlimdim(ncid, &ulid);
	handleError(m_status, QStringLiteral("nc_inq_att"));

	char name[NC_MAX_NAME + 1];
	size_t len;
	for (int i = 0; i < ndims; i++) {
		m_status = nc_inq_dim(ncid, i, name, &len);
		handleError(m_status, QStringLiteral("nc_inq_att"));
		DEBUG("	dim" << i + 1 << ": name/len =" << name << len);

		QStringList props;
		props << i18n("length") << QLatin1String(" = ") << QString::number(len);
		QString value;
		if (i == ulid)
			value = i18n("unlimited");
		auto* attrItem = new QTreeWidgetItem(QStringList() << QLatin1String(name) << i18n("dimension") << props.join(QString()) << value);
		attrItem->setIcon(0, QIcon::fromTheme(QStringLiteral("accessories-calculator")));
		attrItem->setFlags(Qt::ItemIsEnabled);
		parentItem->addChild(attrItem);
	}
}

void NetCDFFilterPrivate::scanVars(int ncid, int nvars, QTreeWidgetItem* parentItem) {
	char name[NC_MAX_NAME + 1];
	nc_type type;
	int ndims, nattrs;
	int dimids[NC_MAX_VAR_DIMS];

	for (int i = 0; i < nvars; i++) {
		m_status = nc_inq_var(ncid, i, name, &type, &ndims, dimids, &nattrs);
		handleError(m_status, QStringLiteral("nc_inq_att"));

		QDEBUG("	var" << i + 1 << ": name/type=" << name << translateDataType(type));
		DEBUG("		ndims/nattr" << ndims << nattrs);

		QStringList props; // properties column
		props << translateDataType(type);
		char dname[NC_MAX_NAME + 1];
		size_t dlen;
		props << QStringLiteral("(");
		if (ndims == 0)
			props << QString::number(0);
		for (int j = 0; j < ndims; j++) {
			m_status = nc_inq_dim(ncid, dimids[j], dname, &dlen);
			if (j != 0)
				props << QStringLiteral("x");
			props << QString::number(dlen);
		}
		props << QStringLiteral(")");

		QStringList rowStrings;
		rowStrings << QLatin1String(name) << i18n("variable") << props.join(QString());
		if (ndims == 0) { // get value of zero dim var
			switch (type) {
			case NC_BYTE: {
				NC_SCAN_VAR(signed char, schar);
				break;
			}
			case NC_UBYTE: {
				NC_SCAN_VAR(unsigned char, uchar);
				break;
			}
			case NC_CHAR: { // not number
				char data;
				m_status = nc_get_var_text(ncid, i, &data);
				handleError(m_status, QStringLiteral("nc_get_var_text"));

				rowStrings << QChar::fromLatin1(data);
				break;
			}
			case NC_SHORT: {
				NC_SCAN_VAR(short, short);
				break;
			}
			case NC_USHORT: {
				NC_SCAN_VAR(unsigned short, ushort);
				break;
			}
			case NC_INT: {
				NC_SCAN_VAR(int, int);
				break;
			}
			case NC_UINT: {
				NC_SCAN_VAR(unsigned int, uint);
				break;
			}
			case NC_INT64: {
				NC_SCAN_VAR(long long, longlong);
				break;
			}
			case NC_UINT64: {
				NC_SCAN_VAR(unsigned long long, ulonglong);
				break;
			}
			case NC_DOUBLE: {
				NC_SCAN_VAR(double, double);
				break;
			}
			case NC_FLOAT: {
				NC_SCAN_VAR(float, float);
				break;
			}
			}

		} else {
			rowStrings << QString();
		}

		auto* varItem = new QTreeWidgetItem(rowStrings);
		varItem->setIcon(0, QIcon::fromTheme(QStringLiteral("x-office-spreadsheet")));
		varItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		// highlight item
		for (int c = 0; c < varItem->columnCount(); c++) {
			varItem->setBackground(c, QColor(192, 255, 192));
			varItem->setForeground(c, Qt::black);
		}
		parentItem->addChild(varItem);

		scanAttrs(ncid, i, -1, varItem);
	}
}
#endif

/*!
	parses the content of the file \c fileName and fill the tree using rootItem.
*/
void NetCDFFilterPrivate::parse(const QString& fileName, QTreeWidgetItem* rootItem) {
	DEBUG(Q_FUNC_INFO);
#ifdef HAVE_NETCDF
	DEBUG("fileName = " << qPrintable(fileName));

	int ncid;
	m_status = nc_open(qPrintable(fileName), NC_NOWRITE, &ncid);
	handleError(m_status, QStringLiteral("nc_open"));
	if (m_status != NC_NOERR) {
		DEBUG("	Giving up");
		return;
	}

	int ndims, nvars, nattr, uldid;
	m_status = nc_inq(ncid, &ndims, &nvars, &nattr, &uldid);
	handleError(m_status, QStringLiteral("nc_inq"));
	DEBUG(" nattr/ndims/nvars = " << nattr << ' ' << ndims << ' ' << nvars);

	auto* attrItem = new QTreeWidgetItem(QStringList() << QString(i18n("Attributes")));
	attrItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
	attrItem->setFlags(Qt::ItemIsEnabled);
	rootItem->addChild(attrItem);
	scanAttrs(ncid, NC_GLOBAL, -1, attrItem);

	auto* dimItem = new QTreeWidgetItem(QStringList() << QString(i18n("Dimensions")));
	dimItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
	dimItem->setFlags(Qt::ItemIsEnabled);
	rootItem->addChild(dimItem);
	scanDims(ncid, ndims, dimItem);

	auto* varItem = new QTreeWidgetItem(QStringList() << QString(i18n("Variables")));
	varItem->setIcon(0, QIcon::fromTheme(QStringLiteral("folder")));
	varItem->setFlags(Qt::ItemIsEnabled);
	rootItem->addChild(varItem);
	scanVars(ncid, nvars, varItem);

	m_status = ncclose(ncid);
	handleError(m_status, QStringLiteral("nc_close"));
#else
	Q_UNUSED(fileName)
	Q_UNUSED(rootItem)
#endif
}

QString NetCDFFilterPrivate::readAttribute(const QString& fileName, const QString& name, const QString& varName) {
#ifdef HAVE_NETCDF
	int ncid;
	m_status = nc_open(qPrintable(fileName), NC_NOWRITE, &ncid);
	handleError(m_status, QStringLiteral("nc_open"));
	if (m_status != NC_NOERR) {
		DEBUG("	Giving up");
		return {};
	}

	// get varid
	int varid;
	if (varName == QStringLiteral("global"))
		varid = NC_GLOBAL;
	else {
		m_status = nc_inq_varid(ncid, qPrintable(varName), &varid);
		handleError(m_status, QStringLiteral("nc_inq_varid"));
	}

	// attribute 'name'
	int attid;
	m_status = nc_inq_attid(ncid, varid, qPrintable(name), &attid);
	handleError(m_status, QStringLiteral("nc_inq_attid"));

	QString nameString = scanAttrs(ncid, varid, attid);

	m_status = ncclose(ncid);
	handleError(m_status, QStringLiteral("nc_close"));

	return nameString;
#else
	Q_UNUSED(fileName)
	Q_UNUSED(name)
	Q_UNUSED(varName)
	return {};
#endif
}

/*!
	reads the content of the variable in the file \c fileName to a string (for preview) or to the data source.
*/
QVector<QStringList>
NetCDFFilterPrivate::readCurrentVar(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode, int lines) {
	if (currentVarName.isEmpty()) {
		q->setLastError(i18n("No variable selected."));
		return {};
	}
	DEBUG(" current variable = " << STDSTRING(currentVarName));

	QVector<QStringList> dataStrings;
#ifdef HAVE_NETCDF
	int ncid;
	m_status = nc_open(qPrintable(fileName), NC_NOWRITE, &ncid);
	handleError(m_status, QStringLiteral("nc_open"));
	if (m_status != NC_NOERR) {
		q->setLastError(i18n("Failed to open the file or it's empty."));
		return {};
	}

	int varid;
	m_status = nc_inq_varid(ncid, qPrintable(currentVarName), &varid);
	handleError(m_status, QStringLiteral("nc_inq_varid"));

	int ndims;
	nc_type type;
	m_status = nc_inq_varndims(ncid, varid, &ndims);
	handleError(m_status, QStringLiteral("nc_inq_varndims"));
	m_status = nc_inq_vartype(ncid, varid, &type);
	handleError(m_status, QStringLiteral("nc_inq_type"));

	int* dimids = (int*)malloc(ndims * sizeof(int));
	m_status = nc_inq_vardimid(ncid, varid, dimids);
	handleError(m_status, QStringLiteral("nc_inq_vardimid"));

	int actualRows = 0, actualCols = 0;
	int columnOffset = 0;
	std::vector<void*> dataContainer;
	switch (ndims) {
	case 0: {
		DEBUG("	zero dimensions");
		actualRows = actualCols = 1; // only one value
		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols);
		switch (type) {
		case NC_BYTE:
		case NC_UBYTE:
		case NC_SHORT:
		case NC_USHORT:
		case NC_INT:
			columnModes[0] = AbstractColumn::ColumnMode::Integer;
			break;
		case NC_UINT: // converted to double (int is too small)
		case NC_INT64:
			columnModes[0] = AbstractColumn::ColumnMode::BigInt;
			break;
		case NC_UINT64: // converted to double (int is too small)
		case NC_DOUBLE:
		case NC_FLOAT:
			columnModes[0] = AbstractColumn::ColumnMode::Double;
			break;
		case NC_CHAR:
			columnModes[0] = AbstractColumn::ColumnMode::Text;
			break;
			// TODO: NC_STRING
		}

		// TODO: use given names?
		QStringList vectorNames;

		if (dataSource) {
			bool ok = false;
			columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes, ok);
			if (!ok) {
				q->setLastError(i18n("Not enough memory."));
				return QVector<QStringList>();
			}
		}

		DEBUG("	Reading data of type " << STDSTRING(translateDataType(type)));
		switch (type) {
		case NC_BYTE: {
			NC_READ_VAR(signed char, schar, int);
			break;
		}
		case NC_UBYTE: {
			NC_READ_VAR(unsigned char, uchar, int);
			break;
		}
		case NC_CHAR: { // no number
			char data;

			m_status = nc_get_var_text(ncid, varid, &data);
			handleError(m_status, QStringLiteral("nc_get_var_text"));

			if (dataSource) {
				QString* sourceData = static_cast<QVector<QString>*>(dataContainer[0])->data();
				sourceData[0] = QChar::fromLatin1(data);
			} else { // preview
				dataStrings << (QStringList() << QChar::fromLatin1(data));
			}
			break;
		}
		case NC_SHORT: {
			NC_READ_VAR(short, short, int);
			break;
		}
		case NC_USHORT: {
			NC_READ_VAR(unsigned short, ushort, int);
			break;
		}
		case NC_INT: {
			NC_READ_VAR(int, int, int);
			break;
		}
		case NC_UINT: {
			NC_READ_VAR(unsigned int, uint, double);
			break;
		} // converted to double (int is too small)
		case NC_INT64: {
			NC_READ_VAR(long long, longlong, double);
			break;
		} // converted to double (int is too small)
		case NC_UINT64: {
			NC_READ_VAR(unsigned long long, ulonglong, double);
			break;
		} // converted to double (int is too small)
		case NC_DOUBLE: {
			NC_READ_VAR(double, double, double);
			break;
		}
		case NC_FLOAT: {
			NC_READ_VAR(float, float, double);
			break;
		}
		}
		break;
	}
	case 1: {
		size_t size;
		m_status = nc_inq_dimlen(ncid, dimids[0], &size);
		handleError(m_status, QStringLiteral("nc_inq_dimlen"));

		if (endRow == -1)
			endRow = (int)size;
		if (lines == -1)
			lines = endRow;
		actualRows = endRow - startRow + 1;
		actualCols = 1; // only one column

		DEBUG("start/end row: " << startRow << ' ' << endRow);
		DEBUG("act rows/cols: " << actualRows << ' ' << actualCols);

		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols);
		switch (type) {
		case NC_BYTE:
		case NC_UBYTE:
		case NC_SHORT:
		case NC_USHORT:
		case NC_INT:
			columnModes[0] = AbstractColumn::ColumnMode::Integer;
			break;
		case NC_UINT: // converted to double (int is too small)
		case NC_INT64:
			columnModes[0] = AbstractColumn::ColumnMode::BigInt;
			break;
		case NC_UINT64: // converted to double (int is too small)
		case NC_DOUBLE:
		case NC_FLOAT:
			columnModes[0] = AbstractColumn::ColumnMode::Double;
			break;
		case NC_CHAR:
			columnModes[0] = AbstractColumn::ColumnMode::Text;
			break;
			// TODO: NC_STRING
		}

		// TODO: use given names?
		QStringList vectorNames;

		if (dataSource) {
			bool ok = false;
			columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes, ok);
			if (!ok) {
				q->setLastError(i18n("Not enough memory."));
				return QVector<QStringList>();
			}
		}

		DEBUG("	Reading data of type " << STDSTRING(translateDataType(type)));
		switch (type) {
		case NC_BYTE: {
			NC_READ_AVAR(signed char, schar, int);
			break;
		}
		case NC_UBYTE: {
			NC_READ_AVAR(unsigned char, uchar, int);
			break;
		}
		case NC_CHAR: { // not number
			char* data = new char[(unsigned int)actualRows];

			size_t start = (size_t)(startRow - 1), count = (size_t)actualRows;
			m_status = nc_get_vara_text(ncid, varid, &start, &count, data);
			handleError(m_status, QStringLiteral("nc_get_vara_text"));

			if (dataSource) {
				QString* sourceData = static_cast<QVector<QString>*>(dataContainer[0])->data();
				for (int i = 0; i < actualRows; i++)
					sourceData[i] = QChar::fromLatin1(data[i]);
			} else { // preview
				for (int i = 0; i < std::min(actualRows, lines); i++)
					dataStrings << (QStringList() << QChar::fromLatin1(data[i]));
			}
			delete[] data;

			break;
		}
		case NC_SHORT: {
			NC_READ_AVAR(short, short, int);
			break;
		}
		case NC_USHORT: {
			NC_READ_AVAR(unsigned short, ushort, int);
			break;
		}
		case NC_INT: {
			NC_READ_AVAR_NATIVE(int);
			break;
		}
		case NC_UINT: {
			NC_READ_AVAR(unsigned int, uint, double);
			break;
		} // converted to double (int is too small)
		case NC_INT64: {
			NC_READ_AVAR(long long, longlong, double);
			break;
		} // converted to double (int is too small)
		case NC_UINT64: {
			NC_READ_AVAR(unsigned long long, ulonglong, double);
			break;
		} // converted to double (int is too small)
		case NC_DOUBLE: {
			NC_READ_AVAR_NATIVE(double);
			break;
		}
		case NC_FLOAT: {
			NC_READ_AVAR(float, float, double);
			break;
		}
		// TODO: NC_STRING
		default:
			DEBUG("	data type not supported yet");
			q->setLastError(i18n("Data type not supported yet."));
		}

		break;
	}
	case 2: {
		size_t rows, cols;
		m_status = nc_inq_dimlen(ncid, dimids[0], &rows);
		handleError(m_status, QStringLiteral("nc_inq_dimlen"));
		m_status = nc_inq_dimlen(ncid, dimids[1], &cols);
		handleError(m_status, QStringLiteral("nc_inq_dimlen"));

		if (endRow == -1)
			endRow = (int)rows;
		if (lines == -1)
			lines = endRow;
		if (endColumn == -1)
			endColumn = (int)cols;
		actualRows = endRow - startRow + 1;
		actualCols = endColumn - startColumn + 1;

		DEBUG("dim = " << rows << "x" << cols);
		DEBUG("startRow/endRow: " << startRow << ' ' << endRow);
		DEBUG("startColumn/endColumn: " << startColumn << ' ' << endColumn);
		DEBUG("actual rows/cols: " << actualRows << ' ' << actualCols);
		DEBUG("lines: " << lines);

		QVector<AbstractColumn::ColumnMode> columnModes;
		columnModes.resize(actualCols);
		switch (type) {
		case NC_BYTE:
		case NC_UBYTE:
		case NC_SHORT:
		case NC_USHORT:
		case NC_INT:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::Integer;
			break;
		case NC_UINT: // converted to double (int is too small)
		case NC_INT64:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::BigInt;
			break;
		case NC_UINT64: // converted to double (int is too small)
		case NC_DOUBLE:
		case NC_FLOAT:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::Double;
			break;
		case NC_CHAR:
			for (int i = 0; i < actualCols; i++)
				columnModes[i] = AbstractColumn::ColumnMode::Text;
			break;
			// TODO: NC_STRING
		}

		// TODO: use given names?
		QStringList vectorNames;

		if (dataSource) {
			bool ok = false;
			columnOffset = dataSource->prepareImport(dataContainer, mode, actualRows, actualCols, vectorNames, columnModes, ok);
			if (!ok) {
				q->setLastError(i18n("Not enough memory."));
				return QVector<QStringList>();
			}
		}

		switch (type) {
		case NC_BYTE: {
			NC_READ_VAR2(signed char, schar, int);
			break;
		}
		case NC_UBYTE: {
			NC_READ_VAR2(unsigned char, uchar, int);
			break;
		}
		case NC_CHAR: { // no number
			char** data = (char**)malloc(rows * sizeof(char*));
			data[0] = (char*)malloc(cols * rows * sizeof(char));
			for (unsigned int i = 1; i < rows; i++)
				data[i] = data[0] + i * cols;

			m_status = nc_get_var_text(ncid, varid, &data[0][0]);
			handleError(m_status, QStringLiteral("nc_get_var_text"));

			if (m_status == NC_NOERR) {
				for (int i = 0; i < std::min((int)rows, lines); i++) {
					QStringList line;
					for (size_t j = 0; j < cols; j++) {
						if (dataSource && dataContainer[0])
							static_cast<QVector<QString>*>(dataContainer[(int)(j - (size_t)startColumn + 1)])->operator[](i - startRow + 1) =
								QChar::fromLatin1(data[i][(int)j]);
						else
							line << QChar::fromLatin1(data[i][j]);
					}
					dataStrings << line;
					Q_EMIT q->completed(100 * i / actualRows);
				}
			}
			free(data[0]);
			free(data);

			break;
		}
		case NC_SHORT: {
			NC_READ_VAR2(short, short, int);
			break;
		}
		case NC_USHORT: {
			NC_READ_VAR2(unsigned short, ushort, int);
			break;
		}
		case NC_INT: {
			NC_READ_VAR2(int, int, int);
			break;
		}
		case NC_UINT: {
			NC_READ_VAR2(unsigned int, uint, double);
			break;
		} // converted to double (int is too small)
		case NC_INT64: {
			NC_READ_VAR2(long long, longlong, double);
			break;
		} // converted to double (int is too small)
		case NC_UINT64: {
			NC_READ_VAR2(unsigned long long, ulonglong, double);
			break;
		} // converted to double (int is too small)
		case NC_FLOAT: {
			NC_READ_VAR2(float, float, double);
			break;
		}
		case NC_DOUBLE: {
			NC_READ_VAR2(double, double, double);
			break;
		}
		// TODO: NC_STRING
		default:
			DEBUG("	data type not supported yet");
			q->setLastError(i18n("Data type not supported yet."));
		}
		break;
	}
	default:
		q->setLastError(i18n("%1 dimensional data of type %2 not supported yet", ndims, translateDataType(type)));
	}

	free(dimids);

	m_status = ncclose(ncid);
	handleError(m_status, QStringLiteral("nc_close"));

	// TODO: why 1 row?
	if (dataSource)
		dataSource->finalizeImport(columnOffset, 1, actualCols, QString(), mode);
#else
	Q_UNUSED(fileName)
	Q_UNUSED(dataSource)
	Q_UNUSED(mode)
	Q_UNUSED(lines)
#endif

	return dataStrings;
}

/*!
	reads the content of the current selected variable from file \c fileName to the data source \c dataSource.
	Uses the settings defined in the data source.
*/
QVector<QStringList> NetCDFFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode mode) {
	DEBUG(Q_FUNC_INFO)
	QVector<QStringList> dataStrings;

	if (currentVarName.isEmpty()) {
		DEBUG(Q_FUNC_INFO << ", no variable selected");
		q->setLastError(i18n("No variable selected."));
		return dataStrings;
	}

	return readCurrentVar(fileName, dataSource, mode);
}

/*!
	writes the content of \c dataSource to the file \c fileName.
*/
void NetCDFFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
	// TODO: writing NetCDF files not implemented yet
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

/*!
  Saves as XML.
 */
void NetCDFFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement(QStringLiteral("netcdfFilter"));
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool NetCDFFilter::load(XmlStreamReader*) {
	return true;
}
