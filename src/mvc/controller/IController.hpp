#pragma once

namespace mvc {

class IController {
public:
    virtual ~IController() = default;
    virtual void run() = 0;
    virtual void stop() = 0;
};

} // namespace mvc
