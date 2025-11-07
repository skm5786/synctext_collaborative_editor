#pragma once

#include "common.h"

void create_initial_doc_if_not_exists(const std::string& filename);
time_t get_mod_time(const std::string& filename);
std::vector<std::string> read_file_content(const std::string& filename);
void write_file(const std::string& filename, const std::vector<std::string>& content);
long long get_timestamp();
std::vector<UpdateObject> diff_documents(const std::vector<std::string>& old_doc, const std::vector<std::string>& new_doc, const char* user_id);