/***************************************************************************
    File                 : globals.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Definition of global constants and enums

 ***************************************************************************/

#include "globals.h"
#include <QMessageBox>
#include <QIcon>
#include <QObject>
#include <QMetaObject>
#include <QMetaEnum>

/**
 * \class SciDAVis
 * \brief Definition of global constants and enums
 *
 * This class must not be instanced. All members are static.
 */

/**
 * \enum SciDAVis::PlotDesignation
 * \brief Types of plot designations
 */
/**
 * \var SciDAVis::noDesignation
 * \brief no plot designation
 */
/**
 * \var SciDAVis::X
 * \brief x values
 */
/**
 * \var SciDAVis::Y
 * \brief y values
 */
/**
 * \var SciDAVis::Z
 * \brief z values
 */
/**
 * \var SciDAVis::xErr
 * \brief x errors
 */
/**
 * \var SciDAVis::yErr
 * \brief y errors
 */

/**
 * \enum SciDAVis::ColumnMode
 * \brief The column mode (defines output and input filter for table columns)
 */
/**
 * \var SciDAVis::Numeric
 * \brief column contains doubles
 */
/**
 * \var SciDAVis::Text
 * \brief column contains strings
 */
/**
 * \var SciDAVis::Month
 * \brief column contains month names
 */
/**
 * \var SciDAVis::Day
 * \brief column contains day of week names
 */
/**
 * \var SciDAVis::DateTime
 * \brief column contains dates and/or times
 */

/**
 * \enum SciDAVis::ColumnDataType
 * \brief Column data type
 */

// TODO: This is still very SciDAVis specific, should be adjusted to be used by LabPlot as well.
//       Maybe rename to class Globals and add several #ifdefs.

/**
 * \brief SciDAVis version number
 *
 * 0xMMmmbb means MM.mm.bb with<br>
 * MM = major version
 * mm = minor version
 * bb = bugfix version
 */
//  Don't forget to change the Doxyfile when changing these!
const int SciDAVis::scidavis_version = 0x000300;

/**
 * \brief Extra version information string (like "-alpha", "-beta", "-rc1", etc...)
 */
const char * SciDAVis::extra_version = "-SVN";

/**
 * \brief Copyright string containing the author names etc.
 */
const char * SciDAVis::copyright_string = "Developers (alphabetical order):\nKnut Franke\nTilman Benkert\n\nDocumentation:\nRoger Gadiou\n\nSpecial thanks to (alphabetical order):\nQuentin Denis\nGudjon I. Gudjonsson\nAlex Kargovsky\nIon Vasilief\n\nThanks to (no particular order):\nthe developers of Qt, Qwt, QwtPlot3D, GSL, muParser, zlib, Python, PyQt, and liborigin\nall bug reporters, translators and other contributors";

/**
 * \brief Release date as a string
 */
const char * SciDAVis::release_date = "XXXX-XX-XX";
		
const QString SciDAVis::appName = "SciDAVis";

/**
 * \brief Return the SciDAVis version number
 */
int SciDAVis::version()
{
	return scidavis_version;
}

/**
 * \brief Return the SciDAVis version string ("SciDAVis x.y.z" without extra version)
 */
QString SciDAVis::versionString()
{
	return appName + " " + 
			QString::number((scidavis_version & 0xFF0000) >> 16)+"."+ 
			QString::number((scidavis_version & 0x00FF00) >> 8)+"."+
			QString::number(scidavis_version & 0x0000FF);
}
			
/**
 * \brief Return the extra version as a string
 */
QString SciDAVis::extraVersion()
{
	return	QString(extra_version);
}

/**
 * \brief Show about dialog
 */
void SciDAVis::about()
{
	QString text = "<h2>"+ versionString() + extraVersion() + "</h2>";
	text += "<h3>" + QObject::tr("Released") + ": " + QString(SciDAVis::release_date) + "</h3>";
	text +=	"<h3>" + QString(SciDAVis::copyright_string).replace("\n", "<br>") + "</h3>";

	QMessageBox *mb = new QMessageBox();
	mb->setAttribute(Qt::WA_DeleteOnClose);
	mb->setWindowTitle(QObject::tr("About SciDAVis"));
	mb->setWindowIcon(QIcon(":/appicon"));
	mb->setIconPixmap(QPixmap(":/appicon"));
	mb->setText(text);
	mb->exec();
}

/**
 * \brief Return the copyright string
 */
QString SciDAVis::copyrightString()
{
	return copyright_string;
}

/**
 * \brief Return the release date as a string
 */
QString SciDAVis::releaseDateString()
{
	return release_date;
}

QString SciDAVis::enumValueToString(int key, const QString& enum_name)
{
	int index = staticMetaObject.indexOfEnumerator(enum_name.toAscii());
	if(index == -1) return QString("invalid");
	QMetaEnum meta_enum = staticMetaObject.enumerator(index);
	return QString(meta_enum.valueToKey(key));
}

int SciDAVis::enumStringToValue(const QString& string, const QString& enum_name)
{
	int index = staticMetaObject.indexOfEnumerator(enum_name.toAscii());
	if(index == -1) return -1;
	QMetaEnum meta_enum = staticMetaObject.enumerator(index);
	return meta_enum.keyToValue(string.toAscii());
}

