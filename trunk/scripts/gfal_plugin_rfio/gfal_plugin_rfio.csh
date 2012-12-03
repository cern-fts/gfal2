## configure gfal to load the plugin
##

setenv LCG_RFIO_TYPE "libdpm.so"
setenv GFAL_PLUGIN_LIST "libgfal_plugin_rfio.so:$GFAL_PLUGIN_LIST"


