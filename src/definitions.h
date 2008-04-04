// LabPltot : definitions.h

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// (QString, title, Title)	variable is m_title
#define ACCESS(type, name, Method) \
	type name() const { return m_ ## name; } \
	void set ## Method(const type value) { m_ ## name=value; }
// (m_transparent, Transparent)
#define ACCESSFLAG(var, Method) \
	bool is ## Method() const { return var; } \
	bool has ## Method() const { return var; } \
	void enable ## Method(const bool value=true) { var=value; } \
	void set ## Method(const bool value=true) { var=value; }

#endif // DEFINITIONS_H
