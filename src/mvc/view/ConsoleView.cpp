#include "mvc/view/ConsoleView.hpp"
#include <iostream>

namespace mvc {

void ConsoleView::render(const std::vector<Item>& items) {
    std::cout << "\n=== Task List ===\n";
    if (items.empty()) {
        std::cout << "  (empty)\n";
    } else {
        for (const auto& item : items) {
            std::cout << "  [" << item.id << "] "
                      << (item.done ? "[X]" : "[ ]")
                      << " " << item.title << "\n";
        }
    }
    std::cout << "\n";
}

void ConsoleView::showMessage(const std::string& msg) {
    std::cout << msg << "\n";
}

std::string ConsoleView::prompt(const std::string& hint) {
    std::cout << hint;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

} // namespace mvc
