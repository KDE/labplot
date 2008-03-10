// LabPltot : defs.h

#ifndef DEFS_H
#define DEFS_H

//#define INF 2147483647

#define ACCESS(type, var, method) \
	type method() const { return var; } \
	void set ## method(const type value) { var=value; }
#define ACCESSFLAG(var, method) \
	bool method ## Enabled() const { return var; } \
	void enable ## method(const bool value=true) { var=value; }

#endif // DEFS_H
