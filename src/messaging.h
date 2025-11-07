#pragma once

#include "common.h"

mqd_t setup_message_queue(const char* mq_name);
void broadcast_updates(const std::vector<UpdateObject>& ops);
void cleanup_messaging();

// Helper functions for conversion
MQ_UpdateObject convert_to_mq_op(const UpdateObject& op);
UpdateObject convert_from_mq_op(const MQ_UpdateObject& mq_op);