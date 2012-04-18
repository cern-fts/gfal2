/**
  \page api_change
  
  
	<h1> API CHANGE 2.0  : </h1> 
		List of the scheduled changes for the GFal 2.0 API

	<h2> I. BRIEF OF THE CHANGES : </h2>
		- The old POSIX API is still unchanged, some new POSIX functions are added to the old one.

		- the old non-POSIX functions do not exist anymore, most of them are converted to a POSIX one ( ex : gfal_ls -> gfal_opendir, gfal_readdir; gfal_turlsfromsurls -> gfal_getxattr )

		- SRM related API is now in the gfal_plugin_lib. All the generic use case of the SRM API can be done with the POSIX API.

		- The old confusing error system disappear for a new GError system : \ref gfal_posix_check_error , \ref gfal_posix_print_error, \ref gfal_posix_strerror_r

		- LRC/EDG legacy support is dropped.

		- Lots of the old dependencies are no more needed and are removed ( globus, gsoap, ccheck )

		- gfal works now with a plugin's architecture, it is easy to add/remove a plugin for a specific URL type with the env var GFAL_PLUGIN_LIST ( ex : GFAL_PLUGIN_LIST=libgfal_plugin_lfc.so:libgfal_plugin_srm.so:libgfal_plugin_rfio.so )


		


	<h2> III. ANOTHER DOCS : </h2>
		- More informations about the GError system : http://developer.gnome.org/glib/stable/glib-Error-Reporting.html
		- More informations about scons : http://www.scons.org/
*/




 
 
