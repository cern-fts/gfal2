What is GFAL2 ?
===============
GFAL2 offers an a single and simple toolkit for the file operations in grids and cloud environments. 
The set of supported protocols depends on the installed plugins.

Supported protocols :
* Local File <file://>
* SRM <srm://>
* GSIFTP <gsiftp://>
* HTTP(S) <http://>, WebDav(s) <dav://>
* XROOTD <root://>
* DCAP/GSIDCAP/KDCAP <dcap://>
* RFIO <rfio://>
* LFC <lfc://, lfn://, guid://>

License
=======
GFAL2 is under the Apache License 2.0

Documentation 
=============
 See http://dmc.web.cern.ch/projects/gfal-2/home for more details

Build
=====
## Install dependencies
On a clean SLCX/fedora/EL system the following 'extra' packages are needed to be installed:

```bash
yum install cmake doxygen glib2-devel libattr-devel openldap-devel zlib-devel lfc-devel dpm-devel srm-ifce-devel dcap-devel globus-gass-copy-devel davix-devel xrootd-client-devel gtest-devel
```
Additionally, `e2fsprogs-devel` on SLC5, or `libuuid-devel` on higher versions.

## Compile
```bash
git clone https://gitlab.cern.ch/dmc/gfal2.git
cd gfal2
mkdir build
cd build
cmake ..
ccmake .. # configure the plugins that you need and enable the test if wished
make
```

## Installation
```bash
make install 
```

## Tests
```bash
make test
```
## Contributions 
Any contributions, patch or external plugins is welcome.
 
# Contact 
dmc-devel@cern.ch or dmc-support@cern.ch

