namespace FFT {

const QString filename = QStringLiteral("FFT/FFT.raw");

QString refFileInfoString = QLatin1String(R"(Title: * C:\users\martin\My Documents\Labplot\ltspice\FFT.asc
<br>Date: Thu Mar 31 18:29:43 2022
<br>Plotname: Transient Analysis
<br>Flags: real forward
<br>No. Variables: 8
<br>No. Points:          899
<br>Offset:   0.0000000000000000e+000
<br>Command: Linear Technology Corporation LTspice XVII
<br>Variables:
<br>	0	time	time
<br>	1	V(n002)	voltage
<br>	2	V(n001)	voltage
<br>	3	V(n003)	voltage
<br>	4	I(B1)	device_current
<br>	5	I(R2)	device_current
<br>	6	I(R1)	device_current
<br>	7	I(V1)	device_current
)"); // last \n is important

const int refDataRowCount = 899;
const int numberPreviewData = 89;

QStringList columnNames = {QStringLiteral("time, time"),
						   QStringLiteral("V(n002), voltage"),
						   QStringLiteral("V(n001), voltage"),
						   QStringLiteral("V(n003), voltage"),
						   QStringLiteral("I(B1), device_current"),
						   QStringLiteral("I(R2), device_current"),
						   QStringLiteral("I(R1), device_current"),
						   QStringLiteral("I(V1), device_current")};
}
