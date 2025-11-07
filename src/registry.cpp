#include "registry.h"
#include "globals.h"

SharedRegistry* setup_shared_memory(const char* name) {
    int shm_fd;
    bool is_creator = false;
    shm_fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm_fd >= 0) { is_creator = true; }
    else if (errno == EEXIST) {
        shm_fd = shm_open(name, O_RDWR, 0666);
        if (shm_fd < 0) { perror("shm_open (existing)"); return nullptr; }
    } else { perror("shm_open (create)"); return nullptr; }
    if (is_creator && ftruncate(shm_fd, sizeof(SharedRegistry)) == -1) {
        perror("ftruncate"); close(shm_fd); shm_unlink(name); return nullptr;
    }
    void* ptr = mmap(0, sizeof(SharedRegistry), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (ptr == MAP_FAILED) { perror("mmap"); return nullptr; }
    
    SharedRegistry* reg = static_cast<SharedRegistry*>(ptr);
    if (is_creator) {
        std::cout << "First user. Initializing shared registry..." << std::endl;
        for (int i = 0; i < MAX_USERS; ++i) { reg->users[i].pid.store(0); }
    }
    return reg;
}

int register_user(const char* user_id, const char* mq_name) {
    pid_t my_pid = getpid();
    for (int i = 0; i < MAX_USERS; ++i) {
        pid_t expected = 0;
        if (g_registry->users[i].pid.compare_exchange_strong(expected, my_pid)) {
            strncpy(g_registry->users[i].user_id, user_id, MAX_USERNAME_LEN - 1);
            g_registry->users[i].user_id[MAX_USERNAME_LEN - 1] = '\0';
            strncpy(g_registry->users[i].mq_name, mq_name, MAX_MQ_NAME_LEN - 1);
            g_registry->users[i].mq_name[MAX_MQ_NAME_LEN - 1] = '\0';
            return i;
        }
    }
    return -1; // No free slots
}

void cleanup_registry() {
    if (g_registry != nullptr && g_my_slot != -1) {
        g_registry->users[g_my_slot].pid.store(0);
        g_my_slot = -1;
    }
    if (g_registry != nullptr) {
        munmap(g_registry, sizeof(SharedRegistry));
        g_registry = nullptr;
    }
}