#pragma once

#include <functional>

#include "msg/actions.h"

using ActionResultCallback = std::function<void(bool accepted)>;
using ActionHandlerCallback = std::function<bool(const msg::ActionRequest&)>;
