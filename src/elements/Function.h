#ifndef FUNCTION_H
#define FUNCTION_H

#include <QString>
#include "../definitions.h"

class Function{
public:
	Function();
	~Function();

	ACCESS(QString, text, Text);
	ACCESS(float, axis1Start, Axis1Start);
	ACCESS(float, axis1End, Axis1End);
	ACCESS(float, axis1Number, Axis1Number);
	ACCESS(float, axis2Start, Axis2Start);
	ACCESS(float, axis2End, Axis2End);
	ACCESS(float, axis2Number, Axis2Number);

private:
	QString m_text;
	float m_axis1Start;
	float m_axis1End;
	float m_axis1Number;
	float m_axis2Start;
	float m_axis2End;
	float m_axis2Number;
};

#endif
