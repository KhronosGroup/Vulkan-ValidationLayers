#! /bin/bash -x
# Run vkcube with portability validation
# mikew@lunarg.com

set -o nounset

echo "DISPLAY = ${DISPLAY:=:0}"
export DISPLAY

GITHUB="${HOME}/gits/github.com"

# Location of .json manifest files (shared libraries to be searched using PATH)
export VK_LAYER_PATH="${GITHUB}/KhronosGroup/Vulkan-ValidationLayers/BUILD/layers"

export VK_INSTANCE_LAYERS="VK_LAYER_LUNARG_portability_validation"

# error, warn, info, debug, all
#export VK_LOADER_DEBUG="debug"

APP="${GITHUB}/KhronosGroup/Vulkan-Tools/BUILD/cube/vkcube"

$APP

# vim: set sw=4 ts=8 et ic ai:
