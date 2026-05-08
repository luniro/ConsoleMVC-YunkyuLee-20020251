#pragma once
#include "mvc/controller/IController.hpp"
#include "mvc/model/IModel.hpp"
#include "mvc/view/IView.hpp"

namespace mvc {

class TaskController : public IController {
public:
    TaskController(IModel& model, IView& view);
    void run() override;
    void stop() override;

private:
    IModel& model_;
    IView& view_;
    bool running_ = false;

    void handleAdd();
    void handleToggle();
    void handleRemove();
};

} // namespace mvc
