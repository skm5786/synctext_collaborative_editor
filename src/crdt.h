#pragma once

#include "common.h"

std::vector<UpdateObject> resolve_conflicts(std::vector<UpdateObject>& all_ops);
bool apply_updates(std::vector<std::string>& doc, std::vector<UpdateObject>& winning_ops);