/**
	\mainpage GFAL 2.0  Documentation
    \author Data Management Clients support, CERN IT-ST ( dmc-support@cern.ch )
	\author Devresse Adrien ( adrien.devresse@cern.ch )
	\author Alejandro Alvarez Ayllon  ( Alejandro.Alvarez.Ayllon@cern.ch )
        \author Andres Manzi ( andrea.manzi.cern.ch)

<h2>GFAL 2.0 Wiki:</h2>
- <a href="http://dmc.web.cern.ch/">Data Management Clients web page</a>

<h2>API:</h2>	
Main file : gfal_api.h

- GFAL2 File API:
	- \ref file_group
- Transfer API:
	- \ref transfer_group
- Configuration & parameters:
	- \ref config_group
- GFAL API for plugin development:
	- \ref _gfal_plugin_interface
- gfal1.0 Deprecated API, provided for compatibility purpose only:
	- \ref posix_group ( see \ref api_change for more details )			

<h2>FAQ:</h2>
- GFAL 2.0  \ref faq

<h2> Examples:</h2>
	- <a href="examples.html"> examples</a>

<h2>GFAL 2.0, Library Design:</h2>
	- \ref page_design

<h2> Summary of the changes:</h2>
- Changes between 1.X and 2.X are resumed \ref api_change 	

<h2>How to compile locally GFAL 2.0</h2>
- Compile :
	- " 1. git clone  https://gitlab.cern.ch/dmc/gfal2 "
	- " 2. cd gfal2 "
	- " 3. mkdir build; cd build"
	- " 4. cmake −D CMAKE_INSTALL_PREFIX=/usr"
	- " 5. make "
	
- Compile tests :
	- " 4. cmake -DUNIT_TESTS=TRUE -DFUNCTIONAL_TESTS=TRUE ../ "
	- " 5.  make; make test"	
	
- make RPMS :
	- cd packaging/; make rpm
	
*/
