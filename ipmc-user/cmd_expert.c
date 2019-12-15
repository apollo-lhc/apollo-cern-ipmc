#include <cmd_expert.h>

#include <user_expert.h>

#include <user_expert.h>
#include <user_helpers.h>
#include <user_cmd.h>

#include <cmd_defs.h>

static const unsigned char expert_help_str[] =
  "Usage: expert_mode <value>.\n"
  "Value should be 'on' (no quotes) to allow "
  "overwrite of special signals.\n"
  "Any other value, including empty (no value), "
  "deactivates expert mode.";

static const unsigned char expert_off_str[] =
  "Expert mode deactivated.";


int
expert_mode(unsigned char * params,
            unsigned char * reply,
            const int conn_idx)
{
  static unsigned char param[MAX_PARAM_LEN];
  get_next_param(param, params, ' ');


  if (str_eq(param, help_str) == 1
      || str_eq(param, question_mark_str) == 1) {
    return strcpyl(reply, expert_help_str);
  }

  static unsigned char on[] = "on";
  if (str_eq(param, on) == 1){
    user_expert_mode_enable();
    return 0;
  }

  user_expert_mode_disable();
  return strcpyl(reply, expert_off_str);
}
