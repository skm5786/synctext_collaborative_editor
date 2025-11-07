#pragma once

#include "common.h"

// Shared Memory & User
extern SharedRegistry* g_registry;
extern int g_my_slot;
extern std::string g_my_user_id;

// Messaging
extern mqd_t g_my_mq_descriptor;
extern std::string g_my_mq_name;

// Threading & State
extern std::atomic<bool> g_running;
extern std::thread g_listener_thread;

// Buffers
extern std::vector<UpdateObject> g_local_updates_buffer;
extern std::vector<UpdateObject> g_received_updates_buffer;
extern SPSCQueue<MQ_UpdateObject, LOCK_FREE_QUEUE_SIZE> g_received_mq_queue;

// UI
extern std::string g_last_merged_status;