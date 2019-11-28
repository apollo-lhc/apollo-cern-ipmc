#ifndef USER_EXPERT_H
#define USER_EXPERT_H

int
user_expert_mode_enable(void);

int
user_expert_mode_disable(void);

int
user_expert_mode_is_on(void);

unsigned const char *
user_expert_mode_get_state_msg(void);


#endif // USER_EXPERT_H
