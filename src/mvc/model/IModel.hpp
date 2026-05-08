#pragma once
#include <string>
#include <vector>

namespace mvc {

struct Item {
    int id;
    std::string title;
    bool done;
};

class IModel {
public:
    virtual ~IModel() = default;
    virtual void addItem(const std::string& title) = 0;
    virtual void removeItem(int id) = 0;
    virtual void toggleItem(int id) = 0;
    virtual const std::vector<Item>& items() const = 0;
};

} // namespace mvc
