# -*- rpm-spec -*-
# 
# lksctp-tools.spec.in --- RPM'ed lksctp-tools
# Author          : Francois-Xavier Kowalski
# Created On      : Sat Jan 10 14:53:53 2004
# Last Modified By: Vlad Yasevich
# Last Modified On: Tue Jan 08 10:56 EDT 2013
# 
#   (c) Copyright Hewlett-Packard Company 2004
#   (C) Copyright IBM Corp. 2004
# 
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License v2 as published by the Free Software Foundation; only
#   version 2 of the License is valid for this software, unless
#   explicitly otherwise stated.
# 
#   This software is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public
#   License v2 along with this program; if not, write to the
#   Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
#   MA 02139, USA.

%define lksctp_version 1.0.15

# older lksctp-tools file name did not conform to RPM file naming
# conventions

%define pack_version %{lksctp_version}
%define file_version %{lksctp_version}

Summary: User-space access to Linux Kernel SCTP
Name: lksctp-tools
Version: %{pack_version}
Release: 1
License: LGPL
Group: System Environment/Libraries
URL: http://lksctp.sourceforge.net
Source0: %{name}-%{file_version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: gcc
#BuildRequires: tetex, tetex-latex, tetex-xdvi, tetex-dvips
#BuildRequires: ghostscript, enscript
BuildRequires: libtool, automake, autoconf

%description
This is the lksctp-tools package for Linux Kernel SCTP Reference
Implementation.

This package is intended to supplement the Linux Kernel SCTP Reference
Implementation now available in the Linux kernel source tree in
versions 2.5.36 and following.  For more information on LKSCTP see the
package documentation README file, section titled "LKSCTP - Linux
Kernel SCTP."

This package contains the base run-time library & command-line tools.

%package devel
Summary: Development kit for lksctp-tools
Group: Development/Libraries
Requires: %{name} = %{version}
Requires: glibc-devel

%description devel
Development kit for lksctp-tools

- Man pages
- Header files
- Static libraries
- Symlinks to dynamic libraries
- Tutorial source code

%package doc
Summary: Documents pertaining to SCTP
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description doc
Documents pertaining to LKSCTP & SCTP in general
- IETF RFC's & Internet Drafts

%prep
%setup -q -n %{name}-%{file_version}

%build
%configure --enable-shared --enable-static
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR="$RPM_BUILD_ROOT"

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog COPYING.lib
%{_bindir}/*
%{_libdir}/libsctp.so.*
%{_libdir}/lksctp-tools/*

%files devel
%defattr(-,root,root,-)
%{_includedir}
%{_libdir}/libsctp.so
%{_libdir}/libsctp.a
%{_libdir}/libsctp.la
%{_datadir}/lksctp-tools/*
%{_mandir}/*

%files doc
%defattr(-,root,root,-)
%doc doc/*.txt

%changelog
* Sun May 12 2013 Daniel Borkmann <dborkman@redhat.com> 1.0.15-1
- 1.0.15 Release

* Wed Apr 05 2013 Daniel Borkmann <dborkman@redhat.com> 1.0.14-1
- 1.0.14 Release

* Wed Jan 23 2013 Daniel Borkmann <dborkman@redhat.com> 1.0.13-1
- 1.0.13 Release

* Tue Jan 08 2013 Vlad Yasevich <vyasevich@gmail.com> 1.0.12-1
- 1.0.12 Release

* Wed Oct 21 2009 Vlad Yasevich <vladislav.yasevich@hp.com> 1.0.11-1
- 1.0.11 Release

* Fri Mar 27 2009 Vlad Yasevich <vladislav.yasevich@hp.com> 1.0.10-1
- 1.0.10 Release

* Sun Jun 13 2008 Vlad Yasevich <vladislav.yasevich@hp.com> 1.0.9-1
- 1.0.9 Release

* Fri Feb 01 2008 Vlad Yasevich <vladislav.yasevich@hp.com> 1.0.8-1
- 1.0.8 Release

* Fri Jun 29 2007 Vlad Yasevich <vladislav.yasevich@hp.com> 1.0.7-1
- 1.0.7 Release

* Fri Feb 3 2006 Sridhar Samudrala <sri@us.ibm.com> 1.0.6-1
- 1.0.6 Release

* Tue Jan 3 2006 Sridhar Samudrala <sri@us.ibm.com> 1.0.5-1
- 1.0.5 Release
 
* Fri Oct 28 2005 Sridhar Samudrala <sri@us.ibm.com> 1.0.4-1
- 1.0.4 Release
 
* Thu Sep 1 2005 Sridhar Samudrala <sri@us.ibm.com> 1.0.3-1
- 1.0.3 Release
 
* Thu Dec 30 2004 Sridhar Samudrala <sri@us.ibm.com> 1.0.2-1
- 1.0.2 Release
 
* Tue May 11 2004 Sridhar Samudrala <sri@us.ibm.com> 1.0.1-1
- 1.0.1 Release
 
* Thu Feb 26 2004 Sridhar Samudrala <sri@us.ibm.com> 1.0.0-1
- 1.0.0 Release
 
* Fri Feb  6 2004 Francois-Xavier Kowalski <francois-xavier.kowalski@hp.com> 0.9.0-1
- package only .txt doc files

* Wed Feb  4 2004 Francois-Xavier Kowalski <francois-xavier.kowalski@hp.com> 0.7.5-1
- badly placed & undelivered files
- simplified delivery list

* Tue Jan 27 2004 Francois-Xavier Kowalski <francois-xavier.kowalski@hp.com> 0.7.5-1
- Integrate comment from project team

* Sat Jan 10 2004 Francois-Xavier Kowalski <francois-xavier.kowalski@hp.com> 2.6.0_test7_0.7.4-1
- Creation
