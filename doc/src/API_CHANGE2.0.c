/**
  \page api_change

<h1> GFAL 1.0 to GFAL 2.0  CHANGES  </h1>

<h2> BRIEF OF THE API CHANGES </h2>
	- The old POSIX API is still unchanged and follow the same signature.

	- the old non-POSIX functions do not exist anymore, most of them are converted to a POSIX one ( ex : gfal_ls -> gfal_opendir, gfal_readdir; gfal_turlsfromsurls -> gfal_getxattr )

	- SRM related API is now in the gfal_plugin_lib. All the generic use case of the SRM API can be done with the POSIX API.

	- The old confusing error system disappear for a new GError system : \ref gfal_posix_check_error , \ref gfal_posix_print_error, \ref gfal_posix_strerror_r

	- 32 bits GFAL 2.0 need the usage of the Large File Support in Linux

<h2> FEATURE CHANGES</h2>
	- LRC/EDG legacy support is dropped.

	- The LFC host parameter is not configured from the BDII value anymore. This feature is replaced by the usage of the lfc://server/path URL scheme.

	- The SRM plugin does not need the BDII by default, this one is used only if nor the port or a FULL SURL format is provided.

	- SRM specific feature can now be accessed by the extended attribute mechanism if needed (turl resolution)


<h2> ARCHITECTURE CHANGES </h2>
	- Lots of the old direct dependencies are no more needed and are removed ( globus, gsoap, ccheck )

	- The new File API is thread-safe

	- Any parameter can be overwriten at runtime by the \ref config_group functions

<h2>OTHER DOCS </h2>
	More informations about the GError system <a href="http://developer.gnome.org/glib/stable/glib-Error-Reporting.html">http://developer.gnome.org/glib/stable/glib-Error-Reporting.html</a>

**/






