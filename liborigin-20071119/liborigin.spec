#
# spec file for liborigin
#
# Copyright (c) 2007 Stefan Gerlach.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments to stefan.gerlach@uni-konstanz.de
#

%define version 20070821
%define release 1

################################################################################

Name: 		liborigin
URL:		http://liborigin.sourceforge.net/
Version:	%{version}
Release:	%{release}
Distribution:	%distro
Summary:	Library for reading ORIGIN files
Source:		%{name}-%{version}.tar.gz
Group:		Applications/Engineering
License:	GPL
Packager:	Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
Vendor:		http://liborigin.sourceforge.net/
BuildRoot:	%{_tmppath}/%{name}-%{version}-build

%description
liborigin is a library for reading Microcal ORIGIN files. It also contains a commandline tool
opj2dat to convert them to data files.
%endif

Authors:
--------
	%{packager}

%prep
%setup

%build
%{__make}

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{name}-%{version}
rm -f ../file.list.%{name}

%install
%{__make} DESTDIR="$RPM_BUILD_ROOT" install
cd $RPM_BUILD_ROOT
find . -type d | sed '1,2d;s,^\.,\%attr(755\,root\,root) \%dir ,' > $RPM_BUILD_DIR/file.list.%{name}
find . -type f -o -type l | sed 's,^\.,\%attr(-\,root\,root) ,' >> $RPM_BUILD_DIR/file.list.%{name}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files -f ../file.list.%{name}
%doc README COPYING FORMAT import.qs

%changelog
* Sat Aug 18 2007 - stefan.gerlach@uni-konstanz.de
newly created
