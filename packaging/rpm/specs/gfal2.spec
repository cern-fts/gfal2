%define checkout_tag 2012062323snap

Name:				gfal2
Version:			2.0.0
Release:			0.10.%{checkout_tag}%{?dist}
Summary:			Grid file access library 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgutil/wiki/gfal2
# svn export http://svn.cern.ch/guest/lcgutil/gfal2/trunk gfal2
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}-%{checkout_tag}.tar.gz 
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

Requires:			%{name}-core = %{version}-%{release}
Requires:			%{name}-transfer = %{version}-%{release}

%description
GFAL 2.0 offers an a single and simple POSIX-like API 
for the file operations in grids and cloud environments. 
The set of supported protocols depends 
of the %{name} plugin install.

%package core
Summary:			Core of the Grid File access Library 2.0
Group:				Applications/Internet
Requires:			openldap%{?_isa}

%description core
The main library of %{name}. \
the %{name} protocol support relies on a plugin system.

%package transfer
Summary:			File Transfer logic of %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release}

%description transfer
%{name}-transfer is the high level API for file transfer operations \
in %{name}. Transfer monitoring and third party transfers \
are supported.

%package devel
Summary:			Development files of %{name}
Group:				Applications/Internet
Requires:			%{name}%{?_isa} = %{version}-%{release} 
Requires:			%{name}-transfer%{?_isa} = %{version}-%{release} 
Requires:			glib2-devel%{?_isa} 
Requires:			libattr-devel%{?_isa} 
Requires:			pkgconfig

%description devel
development files for %{name}

%package doc
Summary:			Documentation for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 

%description doc
documentation, Doxygen and examples of %{name} .

%package plugin-lfc
Summary:			Provide the lfc support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 

%description plugin-lfc
Provide the lfc support (LFN://) for %{name}.
The LFC plugin allows read-only POSIX operations 
for the LFC catalog.

%package plugin-rfio
Summary:			Provide the rfio support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 
Requires:			dpm-libs%{?_isa}

%description plugin-rfio
Provide the rfio support (RFIO://) for %{name}. 
The rfio plugin provides the POSIX operations for 
the rfio URLs, the rfio protocol is used on the DPM 
and on the Castor storage systems.

%package plugin-dcap
Summary:			Provide the support access for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 
Requires:			dcap-tunnel-gsi%{?_isa}

%description plugin-dcap
Provide the dcap support (GSIDCAP://, DCAP://) for %{name}. 
The dcap plugin provides the POSIX operations for the dcap \
URLs, the dcap protocol is used on the DCACHE storage system

%package plugin-srm
Summary:			Provide the srm access for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 

%description plugin-srm
Provide the srm support (SRM://) for %{name}. 
The srm plugin provides the POSIX operations and 
the third party transfer support on the SRM URLs.

%package plugin-gridftp
Summary:			Provide the gridftp support for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 

%description plugin-gridftp
Provide the gridftp support (GSIFTP://) for %{name}. \
The gridftp plugin provides the POSIX operations and \
the third party transfer support on the GSIFTP URLs.

%package all
Summary:			Meta package for GFAL 2.0 install
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 
Requires:			%{name}-transfer%{?_isa} = %{version}-%{release} 
Requires:			%{name}-plugin-lfc%{?_isa} = %{version}-%{release} 
Requires:			%{name}-plugin-dcap%{?_isa} = %{version}-%{release} 
Requires:			%{name}-plugin-srm%{?_isa} = %{version}-%{release} 
Requires:			%{name}-plugin-rfio%{?_isa} = %{version}-%{release} 
Requires:			%{name}-plugin-gridftp%{?_isa} = %{version}-%{release} 

%description all
Meta-package for complete install of GFAL 2.0 \
with all the protocol plugins.

%clean
rm -rf %{buildroot};
make clean

%prep
%setup -q

%build
%cmake -DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}
make doc

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install


%post core -p /sbin/ldconfig

%postun core -p /sbin/ldconfig

%post transfer -p /sbin/ldconfig

%postun transfer -p /sbin/ldconfig


%files
%defattr (-,root,root)
%{_bindir}/gfal2_version
%{_docdir}/%{name}-%{version}/DESCRIPTION
%{_docdir}/%{name}-%{version}/VERSION
%{_mandir}/man1/gfal2_version.1*

%files core
%defattr (-,root,root)
%{_libdir}/libgfal2.so.*
%dir %{_libdir}/%{name}-plugins
%dir %{_docdir}/%{name}-%{version}
%dir %{_sysconfdir}/%{name}.d
%{_libdir}/%{name}-plugins/libgfal_plugin_file.so*
%{_docdir}/%{name}-%{version}/README_PLUGIN_FILE
%{_docdir}/%{name}-%{version}/LICENSE
%config(noreplace) %{_sysconfdir}/%{name}.d/bdii.conf
%config(noreplace) %{_sysconfdir}/%{name}.d/gfal2_core.conf

%files transfer
%defattr (-,root,root)
%{_libdir}/libgfal_transfer.so.*
%{_docdir}/%{name}-%{version}/README_TRANSFER

%files devel
%defattr (-,root,root)
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*.h
%{_includedir}/%{name}/*/*.h
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
%{_docdir}/%{name}-%{version}/README_PLUGIN_LFC
%config(noreplace) %{_sysconfdir}/%{name}.d/lfc_plugin.conf

%files plugin-rfio
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_rfio.so*
%{_docdir}/%{name}-%{version}/README_PLUGIN_RFIO
%config(noreplace) %{_sysconfdir}/%{name}.d/rfio_plugin.conf

%files plugin-dcap
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_dcap.so*
%{_docdir}/%{name}-%{version}/README_PLUGIN_DCAP
%config(noreplace) %{_sysconfdir}/%{name}.d/dcap_plugin.conf

%files plugin-srm
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_srm.so*
%{_docdir}/%{name}-%{version}/README_PLUGIN_SRM
%config(noreplace) %{_sysconfdir}/%{name}.d/srm_plugin.conf

%files plugin-gridftp
%defattr (-,root,root)
%{_libdir}/%{name}-plugins/libgfal_plugin_gridftp.so*
%{_docdir}/%{name}-%{version}/README_PLUGIN_GRIDFTP
%config(noreplace) %{_sysconfdir}/%{name}.d/gsiftp_plugin.conf

%files all
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/README


%changelog
* Sat Jun 23 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-0.10.2012062323snap
 - Snapshot of the 0.10 version for testing

* Fri Jun 15 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-0.9.2012061511snap
 - Snapshot of the 0.9 version for testing

* Fri May 04 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-0.8.2012052812snap
 - Snapshot of the 0.8 version for testing.

* Fri May 04 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-0.7.2012050413snap
 - Improve gridftp plugin with severals other calls
 - Correct dcap/rfio/srm bugs related to error report
 - big work on the documentation
 
* Mon Dec 12 2011 Adrien Devresse <adevress at cern.ch> - 2.0.0-0.6.2012041515snap
 - Initial gfal 2.0 preview release
