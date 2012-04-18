Name:				gfal2
Version:			2.0.0
Release:			5beta1%{?dist}
Summary:			Grid file access library 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
# svn export http://svn.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz 
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
BuildRequires:		cmake
BuildRequires:		doxygen
BuildRequires:		glib2-devel
BuildRequires:		glibmm24-devel
BuildRequires:		libattr-devel
BuildRequires:		openldap-devel
## libuuid is in a different rpm for el5
%if 0%{?el5}
BuildRequires:		e2fsprogs-devel
%else
BuildRequires:		libuuid-devel	
%endif
#lfc plugin dependencies
BuildRequires:		lfc-devel
#rfio plugin dependencies
BuildRequires:		dpm-devel
#srm plugin dependencies
BuildRequires:		srm-ifce-devel
#dcap plugin dependencies
BuildRequires:		dcap-devel
#gridftp plugin dependencies
BuildRequires:		globus-gass-copy-devel

Requires:			%{name}-core = %{version}
Requires:			%{name}-transfer = %{version}

%description
GFAL 2.0 provides a unified interface for POSIX-like file interaction \
and file transfer operations for a set of file access protocols. \
Designed for the wlcg data management, It offers an extensible \
plugin systems able to support new protocols. 

%package core
Summary:			Core of the Grid File access Library 2.0
Group:				Applications/Internet
Requires:			openldap%{?_isa}

%description core
Core and main files of %{name}

%package transfer
Summary:			File Transfer logic of %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description transfer
File transfer functions of %{name}. gfal-transfer provides the \
third party transfer support and a high level interface to manage files \
transfers.

%package devel
Summary:			Development files of %{name}
Group:				Applications/Internet
Requires:			%{name}%{?_isa} = %{version}
Requires:			%{name}-transfer%{?_isa} = %{version}
Requires:			glib2-devel%{?_isa} 
Requires:			libattr-devel%{?_isa} 
Requires:			pkgconfig

%description devel
development files for %{name}

%package doc
Summary:			Documentation for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}


%description doc
Doxygen documentation of %{name} .

%package plugin-lfc
Summary:			Provide the lfc support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-lfc
Provide the lfc support (lfn :// ) for %{name}, lfc plugin 

%package plugin-rfio
Summary:			Provide the rfio support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}
Requires:			dpm-libs%{?_isa}

%description plugin-rfio
Provide the rfio support (rfio:// ) for %{name}, the rfio plugin \
is able to use both of the dpm and castor rfio libraries

%package plugin-dcap
Summary:			Provide the support access for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}
Requires:			dcap-tunnel-gsi%{?_isa}

%description plugin-dcap
Provide the dcap access (gsidcap://, dcap:// ) for %{name}, \
the dcap plugin can be use to interact with the dCache storage systems.

%package plugin-srm
Summary:			Provide the srm access for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-srm
Provide the srm support (srm:// ) for %{name}, srm plugin

%package plugin-gridftp
Summary:			Provide the gridftp support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-gridftp
Provide the gridftp support (gsiftp:// , ftp:// ) for %{name}, \
The gridftp plugin allow to do third party transfer with the gsiftp \
protocols in addition of the POSIX file access.

%package plugin-devel
Summary:			Development files for GFAL 2.0 plugin development
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}

%description plugin-devel
Provide the headers files for %{name} 's plugin development

%package all
Summary:			Meta package for gfal 2.0 global install
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}
Requires:			%{name}-transfer%{?_isa} = %{version}
Requires:			%{name}-plugin-lfc%{?_isa} = %{version}
Requires:			%{name}-plugin-dcap%{?_isa} = %{version}
Requires:			%{name}-plugin-srm%{?_isa} = %{version}
Requires:			%{name}-plugin-rfio%{?_isa} = %{version}
Requires:			%{name}-plugin-gridftp%{?_isa} = %{version}

%description all
Install gfal 2.0 and all standard plugins

%clean
rm -rf "$RPM_BUILD_ROOT";
make clean

%prep
%setup -q

%build
%cmake -DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}
make doc

%install
rm -rf "$RPM_BUILD_ROOT"
make DESTDIR=$RPM_BUILD_ROOT install

%post core -p /sbin/ldconfig

%postun core -p /sbin/ldconfig

%post transfer -p /sbin/ldconfig

%postun transfer -p /sbin/ldconfig


%files
%defattr (-,root,root)
%{_bindir}/gfal2_version
%{_docdir}/%{name}-%{version}/DESCRIPTION
%{_docdir}/%{name}-%{version}/VERSION

%files core
%defattr (-,root,root)
%{_libdir}/libgfal2.so.*
%{_docdir}/%{name}-%{version}/LICENSE

%files transfer
%defattr (-,root,root)
%{_libdir}/libgfal_transfer.so.*

%files devel
%defattr (-,root,root)
%{_includedir}/gfal2/gfal_api.h
%{_includedir}/gfal2/common/gfal_constants.h
%{_includedir}/gfal2/posix/gfal_posix_api.h
%{_includedir}/gfal2/global/*.h*
%{_includedir}/gfal2/transfer/*.h*
%{_libdir}/pkgconfig/gfal2.pc
%{_libdir}/pkgconfig/gfal_transfer.pc
%{_libdir}/libgfal2.so
%{_libdir}/libgfal_transfer.so
%{_docdir}/%{name}-%{version}/RELEASE-NOTES

%files doc
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/html/*
%{_docdir}/%{name}-%{version}/examples/*.c

%files plugin-lfc
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_lfc.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_lfc.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_lfc.sh

%files plugin-rfio
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_rfio.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_rfio.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_rfio.sh

%files plugin-dcap
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_dcap.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_dcap.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_dcap.sh

%files plugin-srm
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_srm.so*
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_srm.csh
%config(noreplace) %{_sysconfdir}/profile.d/gfal_plugin_srm.sh

%files plugin-gridftp
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_gridftp.so*

%files all
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/README

%files plugin-devel
%defattr (-,root,root)
%{_includedir}/gfal2/common/gfal_common_plugin_interface.h
%{_includedir}/gfal2/common/gfal_prototypes.h
%{_includedir}/gfal2/common/gfal_types.h
%{_includedir}/gfal2/common/gfal_common_plugin.h

%changelog
* Mon Dec 12 2011 Adrien Devress <adevress at cern.ch> - 2.0.0-5beta1
 - Initial gfal 2.0 preview release
