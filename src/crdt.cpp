#include "crdt.h"

// Helper function, local to this file
static bool ops_overlap(const UpdateObject& a, const UpdateObject& b) {
    return (a.col_start < b.col_end) && (a.col_end > b.col_start);
}

std::vector<UpdateObject> resolve_conflicts(std::vector<UpdateObject>& all_ops) {
    std::vector<UpdateObject> winning_ops;
    std::vector<bool> discarded(all_ops.size(), false);

    for (size_t i = 0; i < all_ops.size(); ++i) {
        if (discarded[i]) continue;
        UpdateObject& op_A = all_ops[i];

        for (size_t j = i + 1; j < all_ops.size(); ++j) {
            if (discarded[j]) continue;
            UpdateObject& op_B = all_ops[j];

            if (op_A.line_num == op_B.line_num && ops_overlap(op_A, op_B)) {
                if (op_A.timestamp > op_B.timestamp) {
                    discarded[j] = true; // B loses
                } else if (op_B.timestamp > op_A.timestamp) {
                    discarded[i] = true; // A loses
                    break; 
                } else {
                    if (strcmp(op_A.user_id, op_B.user_id) < 0) {
                        discarded[j] = true; // A (smaller user_id) wins
                    } else {
                        discarded[i] = true; // B wins
                        break;
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < all_ops.size(); ++i) {
        if (!discarded[i]) {
            winning_ops.push_back(all_ops[i]);
        }
    }
    return winning_ops;
}

bool apply_updates(std::vector<std::string>& doc, std::vector<UpdateObject>& winning_ops) {
    if (winning_ops.empty()) return false;

    std::sort(winning_ops.begin(), winning_ops.end());

    for (const auto& op : winning_ops) {
        switch (op.type) {
            case DELETE:
                if (op.line_num < (int)doc.size()) {
                    doc.erase(doc.begin() + op.line_num);
                }
                break;
            case REPLACE:
                if (op.line_num < (int)doc.size()) {
                    std::string& line = doc[op.line_num];
                    try {
                         line.replace(op.col_start, op.old_content.length(), op.new_content);
                    } catch (const std::out_of_range& e) {
                        doc[op.line_num] = op.new_content;
                    }
                }
                break;
            case INSERT:
                if (op.line_num <= (int)doc.size()) {
                    doc.insert(doc.begin() + op.line_num, op.new_content);
                } else {
                    doc.push_back(op.new_content);
                }
                break;
        }
    }
    return true;
}