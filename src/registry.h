#pragma once

#include "common.h"

SharedRegistry* setup_shared_memory(const char* name);
int register_user(const char* user_id, const char* mq_name);
void cleanup_registry();