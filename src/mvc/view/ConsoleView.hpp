#pragma once
#include "mvc/view/IView.hpp"

namespace mvc {

class ConsoleView : public IView {
public:
    void render(const std::vector<Item>& items) override;
    void showMessage(const std::string& msg) override;
    std::string prompt(const std::string& hint) override;
};

} // namespace mvc
