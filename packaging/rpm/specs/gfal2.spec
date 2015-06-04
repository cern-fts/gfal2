# unversionned doc dir F20 change https://fedoraproject.org/wiki/Changes/UnversionedDocdirs
%{!?_pkgdocdir: %global _pkgdocdir %{_docdir}/%{name}-%{version}}

%{!?_with_mock_plugin: %{!?_without_mock_plugin: %define _with_mock_plugin -DPLUGIN_MOCK=TRUE}}


Name:               gfal2
Version:            2.10.0
Release:            1%{?dist}
Summary:            Grid file access library 2
Group:              Applications/Internet
License:            ASL 2.0
URL:                http://dmc.web.cern.ch/projects/gfal-2/home
# git clone https://gitlab.cern.ch/dmc/gfal2.git gfal2-2.10.0
# pushd gfal2-2.10.0
# git checkout v2.10.0
# git submodule init && git submodule update
# popd
# tar czf gfal2-2.10.0.tar.gz gfal2-2.10.0
Source0:            %{name}-%{version}.tar.gz
BuildRoot:          %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
BuildRequires:      cmake
BuildRequires:      doxygen
%if 0%{?el5}
BuildRequires:      glib2-devel
%else
BuildRequires:      glib2-devel >= 2.28
%endif
BuildRequires:      libattr-devel
BuildRequires:      openldap-devel
%if ! 0%{?el5}
BuildRequires:      pugixml-devel
%endif

## libuuid is in a different rpm for el5
%if 0%{?el5}
BuildRequires:      e2fsprogs-devel
%else
BuildRequires:      libuuid-devel
%endif
#file plugin dependencies
BuildRequires:      zlib-devel
#lfc plugin dependencies
BuildRequires:      lfc-devel
#rfio plugin dependencies
BuildRequires:      dpm-devel
#srm plugin dependencies
BuildRequires:      srm-ifce-devel >= 1.23.1
#dcap plugin dependencies
BuildRequires:      dcap-devel
#gridftp plugin dependencies
BuildRequires:      globus-gass-copy-devel
#http plugin dependencies
BuildRequires:      davix-devel >= 0.4.0
#tests dependencies
BuildRequires:      gtest-devel

Obsoletes:          %{name}-core < %{version}-%{release}
Provides:           %{name}-core = %{version}-%{release}
Obsoletes:          %{name}-transfer < %{version}-%{release}
Provides:           %{name}-transfer = %{version}-%{release}

%description
GFAL 2.0 offers an a single and simple POSIX-like API 
for the file operations in grids and cloud environments. 
The set of supported protocols depends 
of the %{name} installed plugins.

%package devel
Summary:            Development files of %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}
Requires:           glib2-devel%{?_isa}
Requires:           libattr-devel%{?_isa} 
Requires:           pkgconfig

%description devel
development files for %{name}

%package doc
Summary:            Documentation for %{name}
Group:              Documentation
%if 0%{?fedora} > 10 || 0%{?rhel}>5
BuildArch:          noarch
%endif

%description doc
Documentation, Doxygen and examples of %{name}.


%package plugin-file
Summary:            Provides file support for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}

%description plugin-file
Provides the file support (file://) for %{name}.
The file plugin provides local file operations, as copying from local
to remote or the other way around.

%package plugin-lfc
Summary:            Provides the lfc support for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}

%description plugin-lfc
Provides the lfc support (lfn://) for %{name}.
The LFC plugin allows read-only POSIX operations 
for the LFC catalog.


%package plugin-rfio
Summary:            Provides the rfio support for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release} 
Requires:           dpm-libs%{?_isa}

%description plugin-rfio
Provides the rfio support (rfio://) for %{name}. 
The rfio plugin provides the POSIX operations for 
the rfio URLs, the rfio protocol is used on the DPM 
and on the Castor storage systems.


%package plugin-dcap
Summary:            Provides the support access for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release} 
Requires:           dcap-tunnel-gsi%{?_isa}

%description plugin-dcap
Provides the dcap support (gsidcap://, dcap://) for %{name}. 
The dcap plugin provides the POSIX operations for the dcap \
URLs, the dcap protocol is used on the DCACHE storage system


%package plugin-srm
Summary:            Provides the srm access for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release} 
Requires:           srm-ifce >= 1.23.1

%description plugin-srm
Provides the srm support (srm://) for %{name}. 
The srm plugin provides the POSIX operations and 
the third party transfer support on the SRM URLs.


%package plugin-gridftp
Summary:            Provides the gridftp support for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release} 

%description plugin-gridftp
Provides the gridftp support (gsiftp://) for %{name}. 
The gridftp plugin provides the POSIX operations and 
the third party transfer support on the GSIFTP URLs.


%package plugin-http
Summary:            Provides the HTTP/DAV support for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}
Requires:           davix-libs >= 0.3.2

%description plugin-http
Provides the HTTP (http[s]://) and WevDAV (dav[s]://) support for %{name}.
this plugin is able to do third-party copy with WebDAV if the storage supports it.

%if %{?_with_mock_plugin:1}%{!?_with_mock_plugin:0}
%package plugin-mock
Summary:            Provides a Mock dummy protocol for %{name}
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}

%description plugin-mock
Provides a dummy mock:// protocol for %{name}.
%endif


%package all
Summary:            Meta package for GFAL 2.0 install
Group:              Applications/Internet
Requires:           %{name}%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-file%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-lfc%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-dcap%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-srm%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-rfio%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-gridftp%{?_isa} = %{version}-%{release}
Requires:           %{name}-plugin-http%{?_isa} = %{version}-%{release}

%description all
Meta-package for complete install of GFAL 2.0 
with all the protocol plugins.


%clean
rm -rf %{buildroot};
make clean

%prep
%setup -q

%build
# Make sure the version in the spec file and the version used
# for building matches
gfal2_cmake_ver=`sed -n 's/^set *(VERSION_\(MAJOR\|MINOR\|PATCH\) \+\([0-9]\+\).*/\2/p' CMakeLists.txt | paste -sd '.'`
gfal2_spec_ver=`expr "%{version}" : '\([0-9]*\\.[0-9]*\\.[0-9]*\)'`
if [ "$gfal2_cmake_ver" != "$gfal2_spec_ver" ]; then
    echo "The version in the spec file does not match the CMakeLists.txt version!"
    echo "$gfal2_cmake_ver != %{version}"
    exit 1
fi

%cmake \
-DDOC_INSTALL_DIR=%{_pkgdocdir} \
-DUNIT_TESTS=TRUE \
%{_with_mock_plugin}\
.
make %{?_smp_mflags}
make doc

%check
export GFAL_PLUGIN_DIR=${PWD}/plugins/
export GFAL_CONFIG_DIR=${PWD}/test/conf_test/
export LD_LIBRARY_PATH=${PWD}/src/core:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${PWD}/plugins:${LD_LIBRARY_PATH}
ctest -V

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_bindir}/gfal2_version
%{_libdir}/libgfal2.so.*
%{_libdir}/libgfal_transfer.so.*
%dir %{_libdir}/%{name}-plugins
%dir %{_sysconfdir}/%{name}.d
%config(noreplace) %{_sysconfdir}/%{name}.d/bdii.conf
%config(noreplace) %{_sysconfdir}/%{name}.d/gfal2_core.conf

%{_mandir}/man1/gfal2_version.1*
%dir %{_pkgdocdir}
%{_pkgdocdir}/DESCRIPTION
%{_pkgdocdir}/README
%{_pkgdocdir}/LICENSE
%{_pkgdocdir}/RELEASE-NOTES
%{_pkgdocdir}/README_TRANSFER


%files devel
%{_includedir}/%{name}/
%{_libdir}/pkgconfig/gfal2.pc
%{_libdir}/pkgconfig/gfal_transfer.pc
%{_libdir}/libgfal2.so
%{_libdir}/libgfal_transfer.so

%files doc
%{_pkgdocdir}/readme.html
%{_pkgdocdir}/html/
%{_pkgdocdir}/examples/

%files plugin-file
%{_libdir}/%{name}-plugins/libgfal_plugin_file.so*
%{_pkgdocdir}/README_PLUGIN_FILE

%files plugin-lfc
%{_libdir}/%{name}-plugins/libgfal_plugin_lfc.so*
%{_pkgdocdir}/README_PLUGIN_LFC
%config(noreplace) %{_sysconfdir}/%{name}.d/lfc_plugin.conf

%files plugin-rfio
%{_libdir}/%{name}-plugins/libgfal_plugin_rfio.so*
%{_pkgdocdir}/README_PLUGIN_RFIO
%config(noreplace) %{_sysconfdir}/%{name}.d/rfio_plugin.conf

%files plugin-dcap
%{_libdir}/%{name}-plugins/libgfal_plugin_dcap.so*
%{_pkgdocdir}/README_PLUGIN_DCAP
%config(noreplace) %{_sysconfdir}/%{name}.d/dcap_plugin.conf

%files plugin-srm
%{_libdir}/%{name}-plugins/libgfal_plugin_srm.so*
%{_pkgdocdir}/README_PLUGIN_SRM
%config(noreplace) %{_sysconfdir}/%{name}.d/srm_plugin.conf

%files plugin-gridftp
%{_libdir}/%{name}-plugins/libgfal_plugin_gridftp.so*
%{_pkgdocdir}/README_PLUGIN_GRIDFTP
%config(noreplace) %{_sysconfdir}/%{name}.d/gsiftp_plugin.conf

%files plugin-http
%{_libdir}/%{name}-plugins/libgfal_plugin_http.so*
%{_pkgdocdir}/README_PLUGIN_HTTP
%config(noreplace) %{_sysconfdir}/%{name}.d/http_plugin.conf

%if %{?_with_mock_plugin:1}%{!?_with_mock_plugin:0}
%files plugin-mock
%{_libdir}/%{name}-plugins/libgfal_plugin_mock.so*
%{_pkgdocdir}/README_PLUGIN_MOCK
%config(noreplace) %{_sysconfdir}/%{name}.d/mock_plugin.conf
%endif

%files all
%{_pkgdocdir}/README


%changelog
* Thu Apr 16 2015 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.9.1-1
- Upgraded to upstream release 2.9.1

* Mon Mar 02 2015 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.8.4-1
- Upgraded to upstream release 2.8.4

* Mon Jan 12 2015 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.8.1-1
- Upgraded to upstream release 2.8.1

* Mon Dec 15 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.7.8-3
- Applied patch moving buffer to heap to avoid SIGSEGV when the stack size is limited

* Mon Dec 02 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.7.8-2
- Patched a bug in a call to gfal2_set_error

* Mon Nov 17 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.7.8-1
- Upstream backported fix for protocol honoring on SRM GET and PUT

* Mon Nov 10 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.7.7-1
- Upgraded to upstream release 2.7.7

* Fri Nov 07 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.7.6-1
- New upstream release

* Mon Sep 08 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.6.8-6
- Patch to use lseek64 instead of lseek in the http plugin

* Thu Sep 04 2014 Orion Poplawski <orion@cora.nwra.com> - 2.6.8-5
- Rebuild for pugixml 1.4

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.6.8-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Mon Aug 11 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.6.8-3
- Disable GridFTP session reuse by default (see LCGUTIL-448)

* Fri Aug 08 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.6.8-2
- Patch for symbol that dissapeared in Davix

* Mon Jul 28 2014 Alejandro Alvarez Ayllon <aalvarez at cern.ch> - 2.6.8-1
- Release 2.6.8 of GFAL2

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.5.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Thu Mar 13 2014 Alejandro Alvarez <aalvarez at cern.ch> - 2.5.5-2
 - Backported patch that fixes segfault on the SRM plugin when
   listing empty directories

* Wed Feb 26 2014 Adrien Devresse <adevress at cern.ch> - 2.5.5-1
 - Release 2.5.5 of GFAL2

* Thu Dec 05 2013 Alejandro Alvarez <aalvarez at cern.ch> - 2.4.8-1
 - Release 2.4.8 of GFAL2

* Mon Dec 02 2013 Alejandro Alvarez <aalvarez at cern.ch> - 2.4.7-1
 - Release 2.4.7 of GFAL2

* Thu Nov 07 2013 Alejandro Alvarez <aalvarez at cern.ch> - 2.4.6-1
 - Release 2.4.6 of GFAL 2

* Wed Oct 23 2013 Alejandro Alvarez <aalvarez at cern.ch> - 2.4.5-3
 - Release 2.4.5 of GFAL 2

* Tue Jul 02 2013 Adrien Devresse <adevress at cern.ch> - 2.3.0-0
 - Release 2.3.0 of GFAL 2.0

* Tue Apr 30 2013 Adrien Devresse <adevress at cern.ch> - 2.2.1-0
 - export transfer plugin API ( needed for xrootd plugin )

* Mon Apr 29 2013 Michail Salichos <msalicho at cern.ch> - 2.2.0-5
 - make all gridftp ops async to avoid stalling processes
 
* Fri Apr 26 2013 Michail Salichos <msalicho at cern.ch> - 2.2.0-4
 - replace gass stat with gridftp stat

* Mon Apr 22 2013 Michail Salichos <msalicho at cern.ch> - 2.2.0-3
 - change gridftp error string pattern to satisfy Griffin

* Wed Apr 10 2013 Michail Salichos <msalicho at cern.ch> - 2.2.0-2
 - display turls in verbose mode, needed by fts3

* Mon Mar 25 2013 Michail Salichos <msalicho at cern.ch> - 2.2.0-1
 - fix memory leaks in bringonline SRM op
 
* Wed Mar 20 2013 Adrien Devresse <adevress at cern.ch> - 2.2.0-0
 - fix thread safety issue with gsiftp plugin
 - add the bring online API
 - support for the http plugin by default
 - remove executable stack need
 - remove openMP dependency
 - add synchronous cancellation API
 - add gsiftp performance marker timeout
 - support for srm session reuse
 - reduce memory footprint

* Fri Feb 22 2013 Adrien Devresse <adevress at cern.ch> - 2.1.6-0
 - FTS 3.0 EMI 3 update
 - minor fix on the cancel logic
 - change the performance marker auto-cancel threading model
 - change the performance marker default timeout value

* Mon Feb 11 2013 Adrien Devresse <adevress at cern.ch> - 2.1.5-0
 - FTS 3.0 EMI 3 release sync
 - include event hooks support
 - include cancel logic support
 - include performance marker auto-cancel for gsiftp
 - include checksum timeout support for gsiftp
 - include srm session re-use support


* Thu Jan 10 2013 Adrien Devresse <adevress at cern.ch> - 2.1.1-0
 - fix a minor memory issue with the gfal_transfer stack
 - fix a wrong error report problem with srm third party copy

* Wed Dec 05 2012 Adrien Devresse <adevress at cern.ch> - 2.1.0-2
 - fix an issue this surl to turl resolution for SRM third party copy

* Fri Nov 30 2012 Adrien Devresse <adevress at cern.ch> - 2.1.0-0
 - One-globus session system for gsiftp plugin ( FTS 3.0 need )
 - correct a major issue with the gass attribute system in gsiftp plugin
 - change the lfc set/get env var for a one compatible with set/get opt
 - add set/nb streams option for gsiftp
 - add the mkdir rec function for SRM transfer
 - correct an issue with opendir and srm_ls ( ENOTDIR error silent )
 - correct a memory leak in the cache system
 - correct timeout support for gsiftp transfer
 - implement tcp buffer size support for gsiftp layer
 - apply a correction on the SRM over-write logic, related to a BeStMan errcode problem on File Not Found with srmRm ( EOS )
 - apply a fix on the transfer gsiftp timeout ( protection against multiple cancel )
 - fix for SRM filesize problem ( defined to 0, workaround ) related to globus 426 error bad filesize
 - secure the callback system for globus gass timeout
 - base implementation of the http plugin
 - improve reliability of the bdii resolution
 - add a fallback mechanism in case of bdii bad resolution
 - correct several race conditions in the bdii layer
 - add thread safe support for set/get variables in liblfc
 - correct a deadlock problem with globus and gisftp plugin
 - implement the mkdir_rec logic for general purpose
 - implement the parent folder creation logic with gridftp
 - add support for lfc://host/path URL style for the lfc plugin
 - switch off_t to 64bits size by default ( _FILE_OFFSET_BITS=64)
 - provide a "nobdii" like option
 - provide the choice of turl protocol resolution for srm plugin

* Fri Jul 20 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-1
 - Official initial release candidate of gfal 2.0
 - Transfer API is official
 - gridftp support for performance marker, checksum
 - gridftp support for gridftpv2, dcau param
 - SRM support for spacetoken in transfer
 - SRM abort auto-management
 - parallel operations in transfers
 - file protocol dedicated in a plugin
 - configuration file support
 - srm timeout support
 - general purpose checksum operation support
 - POSIX operation support for gridftp
 - cleaner plugin API
 - new documentation
 - I hope that you will enjoy gfal 2.0 :)

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

