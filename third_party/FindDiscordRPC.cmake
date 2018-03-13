# - Try to find Discord RPC
# Once done this will define
#  DISCORD_RPC_FOUND - System has DiscordRPC
#  DISCORD_RPC_INCLUDE_DIRS - The DiscordRPC include directories

find_package(DiscordRPC)

find_path(DISCORD_RPC_INCLUDE_DIR discord_rpc/include/discord_rpc.h)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(DiscordRPC  DEFAULT_MSG
                                  DISCORD_RPC_LIBRARY DISCORD_RPC_INCLUDE_DIR)

set(DISCORD_RPC_INCLUDE_DIRS ${DISCORD_RPC_INCLUDE_DIR} )