#include "mvc/App.hpp"
#include "mvc/model/TaskModel.hpp"
#include "mvc/view/ConsoleView.hpp"
#include "mvc/controller/TaskController.hpp"

namespace mvc {

void App::run() {
    TaskModel model;
    ConsoleView view;
    TaskController controller(model, view);
    controller.run();
}

} // namespace mvc
