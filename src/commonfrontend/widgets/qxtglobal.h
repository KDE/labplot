/*
    SPDX-FileCopyrightText: 2006-2011 the LibQxt project <http://libqxt.org, foundation@libqxt.org>
    SPDX-FileCopyrightText: 2006-2008 Adam Higerd
    SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef QXTGLOBAL_H
#define QXTGLOBAL_H

#include <QtGlobal>

#define QXT_VERSION 0x000700
#define QXT_VERSION_STR "0.7.0"

//--------------------------global macros------------------------------

#ifndef QXT_NO_MACROS

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(*x))
#endif

#endif // QXT_NO_MACROS

//--------------------------export macros------------------------------

#define QXT_DLLEXPORT DO_NOT_USE_THIS_ANYMORE

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_CORE)
#        define QXT_CORE_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_CORE_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_CORE_EXPORT
#endif // BUILD_QXT_CORE

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_GUI)
#        define QXT_GUI_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_GUI_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_GUI_EXPORT
#endif // BUILD_QXT_GUI

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_NETWORK)
#        define QXT_NETWORK_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_NETWORK_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_NETWORK_EXPORT
#endif // BUILD_QXT_NETWORK

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_SQL)
#        define QXT_SQL_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_SQL_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_SQL_EXPORT
#endif // BUILD_QXT_SQL

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_WEB)
#        define QXT_WEB_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_WEB_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_WEB_EXPORT
#endif // BUILD_QXT_WEB

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_BERKELEY)
#        define QXT_BERKELEY_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_BERKELEY_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_BERKELEY_EXPORT
#endif // BUILD_QXT_BERKELEY

#if !defined(QXT_STATIC) && !defined(QXT_DOXYGEN_RUN)
#    if defined(BUILD_QXT_ZEROCONF)
#        define QXT_ZEROCONF_EXPORT Q_DECL_EXPORT
#    else
#        define QXT_ZEROCONF_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define QXT_ZEROCONF_EXPORT
#endif // QXT_ZEROCONF_EXPORT

#if defined(BUILD_QXT_CORE) || defined(BUILD_QXT_GUI) || defined(BUILD_QXT_SQL) || defined(BUILD_QXT_NETWORK) || defined(BUILD_QXT_WEB) || defined(BUILD_QXT_BERKELEY) || defined(BUILD_QXT_ZEROCONF)
#   define BUILD_QXT
#endif

QXT_CORE_EXPORT const char* qxtVersion();

#ifndef QT_BEGIN_NAMESPACE
#define QT_BEGIN_NAMESPACE
#endif

#ifndef QT_END_NAMESPACE
#define QT_END_NAMESPACE
#endif

#ifndef QT_FORWARD_DECLARE_CLASS
#define QT_FORWARD_DECLARE_CLASS(Class) class Class;
#endif

#define QXT_DECLARE_PRIVATE(PUB) friend class PUB##Private; QxtPrivateInterface<PUB, PUB##Private> qxt_d;
#define QXT_DECLARE_PUBLIC(PUB) friend class PUB;
#define QXT_INIT_PRIVATE(PUB) qxt_d.setPublic(this);
#define QXT_D(PUB) PUB##Private& d = qxt_d()
#define QXT_P(PUB) PUB& p = qxt_p()

template <typename PUB>
class QxtPrivate {
public:
	virtual ~QxtPrivate()
	    = default;
	inline void QXT_setPublic(PUB* pub) {
		qxt_p_ptr = pub;
	}

protected:
	inline PUB& qxt_p() {
		return *qxt_p_ptr;
	}
	inline const PUB& qxt_p() const {
		return *qxt_p_ptr;
	}
	inline PUB* qxt_ptr() {
		return qxt_p_ptr;
	}
	inline const PUB* qxt_ptr() const {
		return qxt_p_ptr;
	}

private:
	PUB* qxt_p_ptr;
};

template <typename PUB, typename PVT>
class QxtPrivateInterface {
	friend class QxtPrivate<PUB>;
public:
	QxtPrivateInterface() {
		pvt = new PVT;
	}
	~QxtPrivateInterface() {
		delete pvt;
	}

	inline void setPublic(PUB* pub) {
		pvt->QXT_setPublic(pub);
	}
	inline PVT& operator()() {
		return *static_cast<PVT*>(pvt);
	}
	inline const PVT& operator()() const {
		return *static_cast<PVT*>(pvt);
	}
	inline PVT * operator->() {
		return static_cast<PVT*>(pvt);
	}
	inline const PVT * operator->() const {
		return static_cast<PVT*>(pvt);
	}
private:
	QxtPrivateInterface(const QxtPrivateInterface&) { }
	QxtPrivateInterface& operator=(const QxtPrivateInterface& i) {
		return i;
	}
	QxtPrivate<PUB>* pvt;
};

#endif // QXT_GLOBAL
