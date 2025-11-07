#include "file_monitor.h"

// Helper function, local to this file
static void find_diff_substrings(const std::string& old_line, const std::string& new_line, int& col_start, std::string& old_sub, std::string& new_sub) {
    size_t start = 0;
    while (start < old_line.length() && start < new_line.length() && old_line[start] == new_line[start]) {
        start++;
    }
    col_start = start;
    size_t old_end = old_line.length(), new_end = new_line.length();
    while (old_end > start && new_end > start && old_line[old_end - 1] == new_line[new_end - 1]) {
        old_end--; new_end--;
    }
    old_sub = old_line.substr(start, old_end - start);
    new_sub = new_line.substr(start, new_end - start);
}

void create_initial_doc_if_not_exists(const std::string& filename) {
    std::ifstream f(filename);
    if (f.good()) { f.close(); return; }
    f.close();
    std::ofstream out(filename);
    if (out.is_open()) {
        out << "Line 0: Hello World" << std::endl;
        out << "Line 1: This is a collaborative editor" << std::endl;
        out << "Line 2: Welcome to SyncText" << std::endl;
        out << "Line 3: Edit this document and see real-time updates" << std::endl;
        out.close();
    }
}

time_t get_mod_time(const std::string& filename) {
    struct stat file_stats;
    if (stat(filename.c_str(), &file_stats) != 0) return 0;
    return file_stats.st_mtime;
}

std::vector<std::string> read_file_content(const std::string& filename) {
    std::vector<std::string> content;
    std::ifstream f(filename);
    std::string line;
    while (std::getline(f, line)) content.push_back(line);
    f.close();
    return content;
}

void write_file(const std::string& filename, const std::vector<std::string>& content) {
    std::ofstream out(filename, std::ios::trunc);
    if (out.is_open()) {
        for(const auto& line : content) {
            out << line << std::endl;
        }
        out.close();
    }
}

long long get_timestamp() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

std::vector<UpdateObject> diff_documents(const std::vector<std::string>& old_doc, const std::vector<std::string>& new_doc, const char* user_id) {
    std::vector<UpdateObject> updates;
    size_t max_lines = std::max(old_doc.size(), new_doc.size());
    for (size_t i = 0; i < max_lines; ++i) {
        std::string old_line = (i < old_doc.size()) ? old_doc[i] : "";
        std::string new_line = (i < new_doc.size()) ? new_doc[i] : "";
        if (old_line != new_line) {
            UpdateObject op;
            op.line_num = i;
            strncpy(op.user_id, user_id, MAX_USERNAME_LEN - 1);
            op.user_id[MAX_USERNAME_LEN - 1] = '\0';
            op.timestamp = get_timestamp();
            if (i >= old_doc.size()) {
                op.type = INSERT;
                op.col_start = 0; op.old_content = ""; op.new_content = new_line;
                op.col_end = op.new_content.length();
            } else if (i >= new_doc.size()) {
                op.type = DELETE;
                op.col_start = 0; op.old_content = old_line; op.new_content = "";
                op.col_end = op.old_content.length();
            } else {
                op.type = REPLACE;
                find_diff_substrings(old_line, new_line, op.col_start, op.old_content, op.new_content);
                op.col_end = op.col_start + op.old_content.length();
            }
            updates.push_back(op);
        }
    }
    return updates;
}