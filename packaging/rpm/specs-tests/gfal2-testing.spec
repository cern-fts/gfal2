%global checkout_tag 20120828svn
%define debug_package %{nil}

Name:				gfal2-testing
Version:			2.8.1
Release:			1.%{checkout_tag}%{?dist}
Summary:			Grid file access library 2.0
Group:				Applications/Internet
License:			ASL 2.0
URL:				http://dmc.web.cern.ch/projects/gfal-2/home
# svn export http://svn.cern.ch/guest/lcgutil/gfal2/trunk gfal2
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}-%{checkout_tag}.tar.gz
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
AutoReqProv:                    no

#main lib dependencies
Requires:		cmake%{?_isa}
Requires:		gfal2-devel%{?_isa}
Requires:		gfal2-all%{?_isa}
Requires:		voms-clients%{?_isa}
Requires:		glibmm24-devel%{?_isa}
Requires:		libattr-devel%{?_isa}
Requires:		openldap-devel%{?_isa}
#tests dependencies
Requires:		gtest-devel%{?_isa}
## libuuid is in a different rpm for el5
%if 0%{?el5}
Requires:		e2fsprogs-devel%{?_isa}
%else
Requires:		libuuid-devel%{?_isa}
%endif
#lfc plugin dependencies
Requires:		lfc-devel%{?_isa}
#rfio plugin dependencies
Requires:		dpm-devel%{?_isa}
#srm plugin dependencies
Requires:		srm-ifce-devel%{?_isa}
#dcap plugin dependencies
Requires:		dcap-devel%{?_isa}
#gridftp plugin dependencies
Requires:		globus-gass-copy-devel%{?_isa}
# globus testing tools
Requires:       globus-proxy-utils

%description
test suite for gfal 2.0

%clean
rm -rf %{buildroot};

%prep
%setup -q

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/src/gfal2
cp -r * %{buildroot}/usr/src/gfal2/

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr (-,root,root)
/usr/src/gfal2/*


%changelog
* Fri Jul 20 2012 Adrien Devresse <adevress at cern.ch> - 2.0.0-1
 - initial test set 
