#ifndef JSONFILTERTEST_H
#define JSONFILTERTEST_H

#include <QtTest/QtTest>

class JsonFilterTest : public QObject {
	Q_OBJECT

private slots:
	void initTestCase();

	void testArrayWithNameImport();
	void testArrayWithoutNameImport();
	void testObjectWithNameImport();
	void testObjectWithoutNameImport();
private:
	QString m_dataDir;
};


#endif
