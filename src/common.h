#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <mqueue.h>
#include <atomic>
#include <cstring>
#include <csignal>
#include <iomanip>
#include <algorithm>

// --- Constants ---
const int MAX_USERS = 5;
const int MAX_USERNAME_LEN = 32;
const int MAX_MQ_NAME_LEN = 64;
extern const char* SHM_REGISTRY_NAME;
const int MONITOR_INTERVAL_MS = 500;
const int BROADCAST_BATCH_SIZE = 5;
const int MERGE_BATCH_SIZE = 5;
const int MQ_MAX_MESSAGES = 10;
const int MAX_CONTENT_LEN = 128;
const int LOCK_FREE_QUEUE_SIZE = 1024;

// --- Data Structures ---

struct UserInfo {
    std::atomic<pid_t> pid;
    char user_id[MAX_USERNAME_LEN];
    char mq_name[MAX_MQ_NAME_LEN];
};

struct SharedRegistry {
    UserInfo users[MAX_USERS];
};

enum OpType { DELETE, REPLACE, INSERT };
struct UpdateObject {
    OpType type;
    int line_num;
    int col_start;
    int col_end;
    std::string old_content;
    std::string new_content;
    long long timestamp;
    char user_id[MAX_USERNAME_LEN];

    bool operator<(const UpdateObject& other) const {
        if (type != other.type) return type < other.type;
        if (type == DELETE) return line_num > other.line_num;
        return line_num < other.line_num;
    }
};

struct MQ_UpdateObject {
    OpType type;
    int line_num;
    int col_start;
    int col_end;
    char old_content[MAX_CONTENT_LEN];
    char new_content[MAX_CONTENT_LEN];
    long long timestamp;
    char user_id[MAX_USERNAME_LEN];
};

template<typename T, size_t Size>
class SPSCQueue {
private:
    T m_buffer[Size];
    std::atomic<size_t> m_read_index;
    std::atomic<size_t> m_write_index;
public:
    SPSCQueue() : m_read_index(0), m_write_index(0) {}
    bool push(const T& item) {
        size_t write_idx = m_write_index.load(std::memory_order_relaxed);
        size_t next_write_idx = (write_idx + 1) % Size;
        if (next_write_idx == m_read_index.load(std::memory_order_acquire)) return false;
        m_buffer[write_idx] = item;
        m_write_index.store(next_write_idx, std::memory_order_release);
        return true;
    }
    bool pop(T& item) {
        size_t read_idx = m_read_index.load(std::memory_order_relaxed);
        if (read_idx == m_write_index.load(std::memory_order_acquire)) return false;
        item = m_buffer[read_idx];
        m_read_index.store((read_idx + 1) % Size, std::memory_order_release);
        return true;
    }
    size_t size() {
        size_t write_idx = m_write_index.load(std::memory_order_acquire);
        size_t read_idx = m_read_index.load(std::memory_order_acquire);
        return (write_idx - read_idx + Size) % Size;
    }
};