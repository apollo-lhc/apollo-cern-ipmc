#include <user_helpers.h>

#include <user_version_def.h>

int
get_version(unsigned char * version) {
  return strlcpy(version, version_str);
}
