## configure gfal to load the plugin
##

setenv LCG_RFIO_TYPE "dpm"
setenv GFAL_PLUGIN_LIST "libgfal_plugin_rfio.so:$GFAL_PLUGIN_LIST"


