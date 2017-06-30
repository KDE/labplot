/***************************************************************************
    File                 : FileDataSource.h
    Project              : LabPlot
    Description          : File data source
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2013 Alexander Semke (alexander.semke@web.de)

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
#ifndef FILEDATASOURCE_H
#define FILEDATASOURCE_H

#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include <QSerialPort>
#include <QSocketNotifier>
#include <QLocalSocket>

class QString;
class AbstractFileFilter;
class QFileSystemWatcher;
class QAction;

class FileDataSource : public Spreadsheet {
	Q_OBJECT
	Q_ENUMS(FileType)

public:
	enum FileType {Ascii, Binary, Image, HDF, NETCDF, FITS};
    enum SourceType {
        FileOrPipe = 0,
        NetworkSocket,
        LocalSocket,
        SerialPort
    };

    enum UpdateType {
        TimeInterval = 0,
        NewData
    };
	FileDataSource(AbstractScriptingEngine*, const QString& name, bool loading = false);
	~FileDataSource();

	static QStringList supportedBaudRates();
	static QStringList availablePorts();

	static QStringList fileTypes();
	static QString fileInfoString(const QString&);

	void setFileType(const FileType);
	FileType fileType() const;

    UpdateType updateType() const;
    void setUpdateType(const UpdateType);

    SourceType sourceType() const;
    void setSourceType(const SourceType);

    int sampleRate() const;
    void setSampleRate(const int);

    int port() const;
    void setPort(const int);

    QString host() const;
    void setHost(const QString&);

    int baudRate() const;
    void setBaudRate(const int);

    void setUpdateFrequency(const int);
    int updateFrequency() const;

	void setFileWatched(const bool);
	bool isFileWatched() const;

	void setFileLinked(const bool);
	bool isFileLinked() const;

	void setFileName(const QString&);
	QString fileName() const;

	void setFilter(AbstractFileFilter*);
	AbstractFileFilter* filter() const;

	virtual QIcon icon() const;
	virtual QMenu* createContextMenu();
	virtual QWidget* view() const;

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

private:
	void initActions();
	void watch();

	QString m_fileName;
	FileType m_fileType;
	bool m_fileWatched;
	bool m_fileLinked;
	AbstractFileFilter* m_filter;
	QFileSystemWatcher* m_fileSystemWatcher;

    UpdateType m_updateType;
    SourceType m_sourceType;
    int m_sampleRate;
    int m_updateFrequency;
    int m_port;
    int m_baudRate;
    QString m_host;
    QSerialPort m_serialPort;
    QSocketNotifier* m_localSocketNotifier;
    QLocalSocket m_localSocket;

	QAction* m_reloadAction;
	QAction* m_toggleLinkAction;
	QAction* m_toggleWatchAction;
	QAction* m_showEditorAction;
	QAction* m_showSpreadsheetAction;

public slots:
	void read();

private slots:
	void fileChanged();
	void watchToggled();
	void linkToggled();

signals:
	void dataChanged();
	void dataUpdated();
};

#endif
