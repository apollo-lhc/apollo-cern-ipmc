#include <cmd_version.h>

#include <user_version.h>
#include <user_cmd.h>
#include <user_helpers.h>

#include <cmd_defs.h>

static const unsigned char version_help_str[] =
  "Usage: version";

int
version(unsigned char * params,
        unsigned char * reply,
        const int conn_idx)
{
  static unsigned char param[20];
  get_next_param(param, params, ' ');

  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, version_help_str);
  }
  
  return user_get_version(reply);
}


