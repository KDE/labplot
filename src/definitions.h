// LabPltot : defs.h

#ifndef DEFINITIONSS_H
#define DEFINITIONSS_H

#define ACCESSFUNC(type, var, method, Method) \
	type method() const { return var; } \
	void set ## Method(const type value) { var=value; }
#define ACCESSFUNCFLAG(var, Method) \
	bool is ## Method ## Enabled() const { return var; } \
	void enable ## Method(const bool value=true) { var=value; }

#endif // DEFS_H
