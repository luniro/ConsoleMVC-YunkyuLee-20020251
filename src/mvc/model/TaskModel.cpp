#include "mvc/model/TaskModel.hpp"
#include <algorithm>

namespace mvc {

void TaskModel::addItem(const std::string& title) {
    items_.push_back({nextId_++, title, false});
}

void TaskModel::removeItem(int id) {
    items_.erase(
        std::remove_if(items_.begin(), items_.end(),
            [id](const Item& item) { return item.id == id; }),
        items_.end());
}

void TaskModel::toggleItem(int id) {
    for (auto& item : items_) {
        if (item.id == id) {
            item.done = !item.done;
            return;
        }
    }
}

const std::vector<Item>& TaskModel::items() const {
    return items_;
}

} // namespace mvc
