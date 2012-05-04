/**
	\mainpage GFAL 2.0  Documentation
	\author Devresse Adrien ( adrien.devresse@cern.ch )
	
		

	<h2>FAQ  : </h2>
	- GFAL 2.0 \ref faq

	<h2>API : </h2>
	
	- GFAL 2.0  API:
		- \ref posix_group \n
		- \ref gfal_posix_check_error

	- Gfal internal API for plugins  :
		- \ref _gfal_plugin_interface

	<h2> Examples : </h2>
		- <a href="examples.html"> examples </a>

	<h2>GFAL 2.0, Library Design : </h2>
		- \ref page_design

	<h2> Summary of the changes : </h2>
	- Changes between 1.X and 2.X are resumed \ref api_change 	
	
	<h2>How to compile locally GFAL 2.0</h2>
	- Compile :
		- " 1.svn export http://svnweb.cern.ch/guest/lcgutil/gfal2/trunk gfal2 "
		- " 2.cd gfal2 "
		- " 3.mkdir build; cd build"
		- " 4. cmake ../"
		- " 5. make "
		
	- Compile tests :
		- " 4. cmake -DUNIT_TESTS=TRUE -DFUNCTIONAL_TESTS=TRUE ../ "
		- " 5.  make; make test"	
		
	- make RPMS :
	    - ./packaging/bin/packager_rpm.sh ./packaging/rpm/specs/ ./
	    - mock -r [mycfg] RPMS/gfal2-.....
	
*/
