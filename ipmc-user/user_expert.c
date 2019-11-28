#include <user_expert.h>

static int expert_mode = 0;

static const unsigned char on[] =
  "\nExpert mode is enabled.";

static const unsigned char off[] = "";

int
user_expert_mode_enable(void){
  expert_mode = 1;
  return 0;
}

int
user_expert_mode_disable(void)
{
  expert_mode = 0;
  return 0;
}


int
user_expert_mode_is_on(void)
{
  return expert_mode;
}

unsigned const char *
user_expert_mode_get_state_msg(void)
{
  if (expert_mode == 1) {
    return on;
  } else {
    return off;
  }
}
