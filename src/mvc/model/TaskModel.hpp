#pragma once
#include "mvc/model/IModel.hpp"

namespace mvc {

class TaskModel : public IModel {
public:
    void addItem(const std::string& title) override;
    void removeItem(int id) override;
    void toggleItem(int id) override;
    const std::vector<Item>& items() const override;

private:
    std::vector<Item> items_;
    int nextId_ = 1;
};

} // namespace mvc
