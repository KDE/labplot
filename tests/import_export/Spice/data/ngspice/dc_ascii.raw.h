namespace dc_ascii {
const QString filename = QStringLiteral("dc_ascii.raw");
QString refFileInfoString = QLatin1String(R"(Title: * simulation de rc2
<br>Date: Sat Jun 16 23:11:45  2018
<br>Plotname: DC transfer characteristic
<br>Flags: real
<br>No. Variables: 8
<br>No. Points: 501
<br>Variables:
<br>	0	v-sweep	voltage
<br>	1	n0	voltage
<br>	2	n1	voltage
<br>	3	n2	voltage
<br>	4	n3	voltage
<br>	5	n4	voltage
<br>	6	n5	voltage
<br>	7	i(v1)	current
)"); // last \n is important

const int refDataRowCount = 501;
const int numberPreviewData = 3;

QStringList columnNames = {QStringLiteral("v-sweep, voltage"),
						   QStringLiteral("n0, voltage"),
						   QStringLiteral("n1, voltage"),
						   QStringLiteral("n2, voltage"),
						   QStringLiteral("n3, voltage"),
						   QStringLiteral("n4, voltage"),
						   QStringLiteral("n5, voltage"),
						   QStringLiteral("i(v1), current")};

} // namespace dc_ascii
