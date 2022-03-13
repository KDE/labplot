namespace dc_ascii {
	const QString filename = "dc_ascii.raw";
	QString refFileInfoString = R"(Title: * simulation de rc2
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
)"; // last \n is important

	const int refDataRowCount = 501;
	const int numberPreviewData = 3;

	QVector<QStringList> refData = {
			{"0.000000000000000e+00", "0.000000000000000e+00", "-5.263157894736840e-03", "-1.052631578947368e-02", "-3.684210526315788e-02", "-4.210526315789472e-02", "-4.736842105263156e-02", "-5.263157894736841e-06"},
			{"1.000000000000000e-02", "1.000000000000000e-02", "4.210526315789475e-03", "-1.578947368421050e-03", "-3.052631578947368e-02", "-3.631578947368420e-02", "-4.210526315789473e-02", "-5.789473684210525e-06"},
			{"2.000000000000000e-02", "2.000000000000000e-02", "1.368421052631579e-02", "7.368421052631581e-03", "-2.421052631578947e-02", "-3.052631578947368e-02", "-3.684210526315788e-02", "-6.315789473684212e-06"},
			{"3.000000000000000e-02", "3.000000000000000e-02", "2.315789473684211e-02", "1.631578947368421e-02", "-1.789473684210525e-02", "-2.473684210526315e-02", "-3.157894736842104e-02", "-6.842105263157894e-06"},
			{"4.000000000000000e-02", "4.000000000000000e-02", "3.263157894736842e-02", "2.526315789473684e-02", "-1.157894736842105e-02", "-1.894736842105263e-02", "-2.631578947368421e-02", "-7.368421052631584e-06"},
		};
		
	QStringList columnNames = {"v-sweep, voltage", "n0, voltage", "n1, voltage", "n2, voltage", "n3, voltage", "n4, voltage", "n5, voltage", "i(v1), current"};

} // namespace dc_ascii
