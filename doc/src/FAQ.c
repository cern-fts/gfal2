/**
   \page faq
 *  Doc File for a FAQ
 * 
 * \author : Devresse Adrien
 *

	<h3> 1. Why switch to GFAL 2.0  ? </h3>
		10 good reasons to switch to GFAL 2.0 : <br />
		GFAL 2.0 :
			- is \b extensible with the new plugin system. Any protocol can be supported with a plugin.
			- is independant of the technologies provided ( \b no \b globus \b dependencies, no need to install technologies that are not used ).
			- is now \b thread-safe.
			- has a new error's system with full string errors and a "TRACE" mode.
			- is now designed to be as POSIX as possible, No non-posix like functions will be provided in the future.
			- manages the extended attributes system for advanced functions.
			- is smaller and faster
			- has an integrated cache system.
			- is given with \b gfalFS , a NEW file system which can mount any point in the GRID like a local folder. 
			- is fully supported and easy to install on EMI.

	<h3> 1. What are the main changes from gfal 1.0 ? </h3>
	
		- the NON-POSIX functions of 1.0 do not exist anymore, they are mapped to POSIX one.
		- the gfal_request system is now replaced by a more simpler one.
		- GFal 2.0 provide the getxattr, setxattr, listxattr calls for the plateform's specific calls ( turl resolution, guid resolution, ...)

	<h3> 2. I would make a SRM to TURL resolution with GFAL 2.0 and the gfal_srm_plugin, How to do it ? </h3>
		- char turl[2048];
		- gfal_getxattr("srm://masrul/monfichier", "user.replicas", turl, 2048);

	<h3> 3. I would make a GUID to LFC resolution with GFAL 2.0 and the gfal_lfc_plugin, How to do it ? </h3>
		- char lfn[2048];
		- gfal_getxattr("srm://masrul/monfichier", "user.guid", lfn, 2048);

	<h3 > 4. I want a more verbose error report, How to do it ? </h3>
		- \ref gfal_set_verbose( GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE );
		- [...]
		- \ref gfal_posix_check_error(); // called after each gfal_* call

	<h3> 4. I need to create a GFAL 2.0 plugin, How to do it ? </h3>
		- a tutorial will come for more details.

	<h3> 5 . I need to implement a specific non-POSIX call in GFAL, How to do it ? </h3>
		- create a new xattr's key for your needs.

	<h3> 6. What about the licence ?  : </h3> 
		- The license is still unchanged from gfal 1.0, It is a Apache 2.0 license : \ref apl
	
	<h3> 7. I get a Communication Error ( ECOMM ) on all my gfal Calls : </h3>
		- ECOMM means an un-usual Error, reported by the server-side.
			In Most of the case, this error is due to an access right problem.
				try to setup your voms-proxy with voms-prox-init --voms [your vo] and try again.
	
	<h3> 8. I Wish add/remove a plugin to the plugin list, How to do it ? </h3>
		- The list of the gfal's plugin loaded is in the environment variable GFAL_PLUGIN_LIST,
		you have just to modify this environment variable before start your gfal's program.




*/
