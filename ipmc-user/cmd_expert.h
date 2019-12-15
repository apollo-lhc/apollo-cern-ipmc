#ifndef CMD_EXPERT_H
#define CMD_EXPERT_H

#include <user_expert.h>

static const unsigned char expert_label_str[] = " (E)";


int
expert_mode(unsigned char * params,
            unsigned char * reply,
            const int conn_idx);

#endif // CMD_EXPERT_H
