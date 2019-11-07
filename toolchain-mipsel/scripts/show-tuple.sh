# What we need:
#  - the .config file

# Parse the common functions
. "${CT_LIB_DIR}/scripts/functions"

# Read the sample settings
CT_LoadConfig

eval "echo \"$1\""
