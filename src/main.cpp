#include "common.h"
#include "globals.h"
#include "registry.h"
#include "messaging.h"
#include "file_monitor.h"
#include "crdt.h"
#include "display.h"

// --- Forward Declarations for Threads & Signals ---
void listener_thread_func();
void main_thread_func(const std::string& doc_filename);
void cleanup(int sig);
void setup_signal_handler();

// --- Main Function ---
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <user_id>" << std::endl;
        return 1;
    }
    g_my_user_id = argv[1];
    if (g_my_user_id.length() >= MAX_USERNAME_LEN) {
        std::cerr << "Error: user_id is too long." << std::endl;
        return 1;
    }

    g_registry = setup_shared_memory(SHM_REGISTRY_NAME);
    if (g_registry == nullptr) { return 1; }

    g_my_mq_name = "/queue_" + g_my_user_id;
    g_my_mq_descriptor = setup_message_queue(g_my_mq_name.c_str());
    if (g_my_mq_descriptor == (mqd_t)-1) {
        cleanup(0); return 1;
    }
    std::cout << "Message queue created: " << g_my_mq_name << std::endl;

    g_my_slot = register_user(g_my_user_id.c_str(), g_my_mq_name.c_str());
    if (g_my_slot == -1) {
        std::cerr << "Error: Failed to register user. Registry might be full." << std::endl;
        cleanup(0); return 1;
    }
    std::cout << "Registered as " << g_my_user_id << " in slot " << g_my_slot << std::endl;

    setup_signal_handler();

    std::string doc_filename = g_my_user_id + "_doc.txt";
    create_initial_doc_if_not_exists(doc_filename);

    g_running = true;
    g_listener_thread = std::thread(listener_thread_func); // Launch listener

    main_thread_func(doc_filename); // Run main logic

    g_listener_thread.join();
    
    return 0;
}

// --- Signal Handling ---
void cleanup(int sig) {
    (void)sig;
    if (g_running.exchange(false) == false) return;
    
    std::cout << "\nCleaning up and exiting..." << std::endl;
    
    if (g_listener_thread.joinable()) { g_listener_thread.join(); }
    
    cleanup_messaging();
    cleanup_registry();
    
    std::cout << "Cleanup complete. Goodbye." << std::endl;
    exit(0);
}

void setup_signal_handler() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
}

// --- Thread Functions ---
void listener_thread_func() {
    MQ_UpdateObject mq_op;
    while (g_running) {
        ssize_t bytes_read = mq_receive(g_my_mq_descriptor, (char*)&mq_op, sizeof(MQ_UpdateObject), nullptr);
        if (bytes_read == sizeof(MQ_UpdateObject)) {
            while (!g_received_mq_queue.push(mq_op) && g_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        } else if (errno == EAGAIN) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else if (g_running) {
            perror("mq_receive");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void main_thread_func(const std::string& doc_filename) {
    std::vector<std::string> local_content = read_file_content(doc_filename);
    time_t last_mod_time = get_mod_time(doc_filename);

    while (g_running) {
        // 1. Process Received Updates
        MQ_UpdateObject mq_op;
        while(g_received_mq_queue.pop(mq_op)) {
            g_received_updates_buffer.push_back(convert_from_mq_op(mq_op));
        }

        // 2. Check for Local File Changes
        time_t new_mod_time = get_mod_time(doc_filename);
        if (new_mod_time > last_mod_time) {
            std::vector<std::string> new_content = read_file_content(doc_filename);
            std::vector<UpdateObject> new_ops = diff_documents(local_content, new_content, g_my_user_id.c_str());
            g_local_updates_buffer.insert(g_local_updates_buffer.end(), new_ops.begin(), new_ops.end());
            local_content = new_content;
            last_mod_time = new_mod_time;
        }

        // 3. Broadcast Condition
        if (g_local_updates_buffer.size() >= BROADCAST_BATCH_SIZE) {
            broadcast_updates(g_local_updates_buffer);
            g_last_merged_status = "Broadcasted " + std::to_string(g_local_updates_buffer.size()) + " local ops.";
            g_local_updates_buffer.clear();
        }

        // 4. Merge Condition
        size_t ops_to_merge_count = g_local_updates_buffer.size() + g_received_updates_buffer.size();
        if (ops_to_merge_count >= MERGE_BATCH_SIZE) {
            std::vector<UpdateObject> all_ops = g_local_updates_buffer;
            all_ops.insert(all_ops.end(), g_received_updates_buffer.begin(), g_received_updates_buffer.end());
            g_local_updates_buffer.clear();
            g_received_updates_buffer.clear();

            std::vector<UpdateObject> winning_ops = resolve_conflicts(all_ops);
            bool changed = apply_updates(local_content, winning_ops);

            if (changed) {
                write_file(doc_filename, local_content);
                last_mod_time = get_mod_time(doc_filename);
            }
            g_last_merged_status = "Merged " + std::to_string(all_ops.size()) + " ops. " +
                                 std::to_string(winning_ops.size()) + " won.";
        }
        
        // 5. Draw UI & Sleep
        draw_terminal_display(doc_filename, local_content);
        std::this_thread::sleep_for(std::chrono::milliseconds(MONITOR_INTERVAL_MS));
    }
}