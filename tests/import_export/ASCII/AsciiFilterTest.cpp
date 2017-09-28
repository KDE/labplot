#include "AsciiFilterTest.h"

void AsciiFilterTest::test001() {
	//TODO
	QString str = "Hello";
	QVERIFY(str.toUpper() == "HELLO");
}

QTEST_MAIN(AsciiFilterTest)
