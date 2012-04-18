## configure gfal to load the plugin
##

export LFC_HOST=prod-lfc-shared-central.cern.ch
export LCG_GFAL_INFOSYS=lcg-bdii.cern.ch:2170
export LFC_CONRETRY=2
export LFC_CONRETRYINT=1
export LFC_CONNTIMEOUT=15
export GFAL_PLUGIN_LIST=libgfal_plugin_lfc.so:$GFAL_PLUGIN_LIST


