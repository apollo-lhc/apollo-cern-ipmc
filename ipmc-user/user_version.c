#include <user_helpers.h>

#include <user_version_def.h>

int
user_get_version(unsigned char * version) {
  return strlcpy(version, version_str);
}
