/**
	\mainpage GFAL 2.0  Documentation
	\author Devresse Adrien ( adrien.devresse@cern.ch )
	
		

	<h2>I.FAQ  : </h2>
	- GFAL 2.0 \ref faq

	<h2> II API : </h2>
	
	- GFAL 2.0 POSIX lib API:
		- the POSIX style API : \ref posix_group \n
		- the Error management API : \ref gfal_posix_check_error

	- Gfal internal API to construct plugin ( not stable currently ) :
		- \ref _gfal_plugin_interface

	<h2> III. Examples : </h2>
		- <a href="examples.html"> examples </a>

	<h2>IV. GFAL 2.0, Library Design : </h2>
		- \ref page_design

	<h2>V. Report of the Scheduled changes : </h2>
	- Changes between 1.X and 2.X are resumed \ref api_change 	
	
	<h2>VI. How to compile locally GFAL 2.0</h2>
	- Compile :
		- enable EMI and EPEL repositories ( and equivalent )
		- Install scons ( http://www.scons.org, python script )
		- " git svn clone  http://svnweb.cern.ch/guest/lcgutil/gfal/branches/gfal_2_0_main gfal2 "
		- " cd gfal2 "
		- "scons -j 8"
	- Compile tests :
		- Install cgreen, unit test framework
		- Install scons 
		- execute "scons test -j 8" in the CERNgfal dir
	- Clean builds :
		- scons -c
	
*/
