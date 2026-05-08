#pragma once
#include <string>
#include <vector>
#include "mvc/model/IModel.hpp"

namespace mvc {

class IView {
public:
    virtual ~IView() = default;
    virtual void render(const std::vector<Item>& items) = 0;
    virtual void showMessage(const std::string& msg) = 0;
    virtual std::string prompt(const std::string& hint) = 0;
};

} // namespace mvc
