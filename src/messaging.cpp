#include "messaging.h"
#include "globals.h"

mqd_t setup_message_queue(const char* mq_name) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof(MQ_UpdateObject);
    mq_unlink(mq_name); //
    mqd_t mqd = mq_open(mq_name, O_CREAT | O_RDONLY | O_NONBLOCK, 0666, &attr);
    if (mqd == (mqd_t)-1) { perror("mq_open"); }
    return mqd;
}

void broadcast_updates(const std::vector<UpdateObject>& ops) {
    for (int i = 0; i < MAX_USERS; ++i) {
        pid_t pid = g_registry->users[i].pid.load(std::memory_order_acquire);
        if (pid != 0 && i != g_my_slot) {
            const char* other_mq_name = g_registry->users[i].mq_name;
            mqd_t other_mq = mq_open(other_mq_name, O_WRONLY);
            if (other_mq == (mqd_t)-1) continue;
            for (const auto& op : ops) {
                MQ_UpdateObject mq_op = convert_to_mq_op(op);
                mq_send(other_mq, (const char*)&mq_op, sizeof(mq_op), 0);
            }
            mq_close(other_mq);
        }
    }
}

void cleanup_messaging() {
    if (g_my_mq_descriptor != (mqd_t)-1) {
        mq_close(g_my_mq_descriptor);
        mq_unlink(g_my_mq_name.c_str());
        g_my_mq_descriptor = (mqd_t)-1;
    }
}

MQ_UpdateObject convert_to_mq_op(const UpdateObject& op) {
    MQ_UpdateObject mq_op;
    mq_op.type = op.type;
    mq_op.line_num = op.line_num;
    mq_op.col_start = op.col_start;
    mq_op.col_end = op.col_end;
    mq_op.timestamp = op.timestamp;
    strncpy(mq_op.user_id, op.user_id, MAX_USERNAME_LEN - 1);
    mq_op.user_id[MAX_USERNAME_LEN - 1] = '\0';
    strncpy(mq_op.old_content, op.old_content.c_str(), MAX_CONTENT_LEN - 1);
    mq_op.old_content[MAX_CONTENT_LEN - 1] = '\0';
    strncpy(mq_op.new_content, op.new_content.c_str(), MAX_CONTENT_LEN - 1);
    mq_op.new_content[MAX_CONTENT_LEN - 1] = '\0';
    return mq_op;
}

UpdateObject convert_from_mq_op(const MQ_UpdateObject& mq_op) {
    UpdateObject op;
    op.type = mq_op.type;
    op.line_num = mq_op.line_num;
    op.col_start = mq_op.col_start;
    op.col_end = mq_op.col_end;
    op.timestamp = mq_op.timestamp;
    op.user_id[MAX_USERNAME_LEN - 1] = '\0';
    strncpy(op.user_id, mq_op.user_id, MAX_USERNAME_LEN - 1);
    op.old_content = std::string(mq_op.old_content);
    op.new_content = std::string(mq_op.new_content);
    return op;
}