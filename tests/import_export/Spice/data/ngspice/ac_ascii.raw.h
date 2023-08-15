namespace ac_ascii {
const QString filename = QStringLiteral("ac_ascii.raw");
const int refDataRowCount = 201;
const int numberPreviewData = 3;

QString refFileInfoString = QLatin1String(R"(Title: * simulation de rc2
<br>Date: Sat Jun 16 23:11:45  2018
<br>Plotname: AC Analysis
<br>Flags: complex
<br>No. Variables: 8
<br>No. Points: 201
<br>Variables:
<br>	0	frequency	frequency grid=3
<br>	1	n0	voltage
<br>	2	n1	voltage
<br>	3	n2	voltage
<br>	4	n3	voltage
<br>	5	n4	voltage
<br>	6	n5	voltage
<br>	7	i(v1)	current
)"); // last \n is important

QStringList columnNames = {QStringLiteral("frequency, frequency grid=3 REAL"),
						   QStringLiteral("frequency, frequency grid=3 IMAGINARY"),
						   QStringLiteral("n0, voltage REAL"),
						   QStringLiteral("n0, voltage IMAGINARY"),
						   QStringLiteral("n1, voltage REAL"),
						   QStringLiteral("n1, voltage IMAGINARY"),
						   QStringLiteral("n2, voltage REAL"),
						   QStringLiteral("n2, voltage IMAGINARY"),
						   QStringLiteral("n3, voltage REAL"),
						   QStringLiteral("n3, voltage IMAGINARY"),
						   QStringLiteral("n4, voltage REAL"),
						   QStringLiteral("n4, voltage IMAGINARY"),
						   QStringLiteral("n5, voltage REAL"),
						   QStringLiteral("n5, voltage IMAGINARY"),
						   QStringLiteral("i(v1), current REAL"),
						   QStringLiteral("i(v1), current IMAGINARY")};
}
