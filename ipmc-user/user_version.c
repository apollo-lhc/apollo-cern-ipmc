#include <user_version_def.h>

#include <user_helpers.h>
#include <debug.h>

int
user_get_version(unsigned char * version) {
  return strcpyl(version, version_str);
}
