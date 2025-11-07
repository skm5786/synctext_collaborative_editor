#include "display.h"
#include "globals.h"

void draw_terminal_display(const std::string& doc_name, const std::vector<std::string>& content) {
    std::cout << "\033[2J\033[1;1H"; //
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "--- SyncText (CRDT Editor) ---" << std::endl;
    std::cout << "Document: " << doc_name << std::endl;
    std::cout << "User: " << g_my_user_id << std::endl;
    std::cout << "Last UI update: " << std::put_time(std::localtime(&now), "%T") << std::endl;
    
    std::cout << "\n--- Document Content ---" << std::endl;
    for (size_t i = 0; i < content.size(); ++i) {
        std::cout << "Line " << std::setw(2) << i << ": " << content[i] << std::endl;
    }
    if (content.empty()) std::cout << "[Document is empty]" << std::endl;

    std::cout << "\n--- Active Users ---" << std::endl;
    for (int i = 0; i < MAX_USERS; ++i) {
        if (g_registry->users[i].pid.load(std::memory_order_relaxed) != 0) {
            std::cout << "  - " << g_registry->users[i].user_id;
            if (i == g_my_slot) std::cout << " (You)";
            std::cout << std::endl;
        }
    }
    
    std::cout << "\n--- Sync Status ---" << std::endl;
    std::cout << "Monitoring for changes..." << std::endl;
    std::cout << "Local ops buffer (to send): " << g_local_updates_buffer.size() << " / " << BROADCAST_BATCH_SIZE << std::endl;
    std::cout << "Recv ops buffer (to merge): " << g_received_updates_buffer.size() << " / " << MERGE_BATCH_SIZE << std::endl;
    std::cout << "Lock-free MQ size:        " << g_received_mq_queue.size() << std::endl;
    std::cout << "Last merge status:        " << g_last_merged_status << std::endl;
}