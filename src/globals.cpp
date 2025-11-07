#include "globals.h"
const char* SHM_REGISTRY_NAME = "/synctext_registry";
// Shared Memory & User
SharedRegistry* g_registry = nullptr;
int g_my_slot = -1;
std::string g_my_user_id = "";

// Messaging
mqd_t g_my_mq_descriptor = (mqd_t)-1;
std::string g_my_mq_name = "";

// Threading & State
std::atomic<bool> g_running = true;
std::thread g_listener_thread;

// Buffers
std::vector<UpdateObject> g_local_updates_buffer;
std::vector<UpdateObject> g_received_updates_buffer;
SPSCQueue<MQ_UpdateObject, LOCK_FREE_QUEUE_SIZE> g_received_mq_queue;

// UI
std::string g_last_merged_status = "System nominal.";