#include "mvc/controller/TaskController.hpp"
#include <stdexcept>

namespace mvc {

TaskController::TaskController(IModel& model, IView& view)
    : model_(model), view_(view) {}

void TaskController::run() {
    running_ = true;
    while (running_) {
        view_.render(model_.items());
        view_.showMessage("[1] Add  [2] Toggle  [3] Remove  [q] Quit");
        const std::string cmd = view_.prompt("> ");

        if      (cmd == "1")            { handleAdd(); }
        else if (cmd == "2")            { handleToggle(); }
        else if (cmd == "3")            { handleRemove(); }
        else if (cmd == "q" || cmd == "Q") { stop(); }
        else                            { view_.showMessage("Unknown command."); }
    }
}

void TaskController::stop() {
    running_ = false;
}

void TaskController::handleAdd() {
    const std::string title = view_.prompt("Title: ");
    if (!title.empty()) {
        model_.addItem(title);
    }
}

void TaskController::handleToggle() {
    const std::string input = view_.prompt("ID: ");
    try {
        model_.toggleItem(std::stoi(input));
    } catch (const std::exception&) {
        view_.showMessage("Invalid ID.");
    }
}

void TaskController::handleRemove() {
    const std::string input = view_.prompt("ID: ");
    try {
        model_.removeItem(std::stoi(input));
    } catch (const std::exception&) {
        view_.showMessage("Invalid ID.");
    }
}

} // namespace mvc
