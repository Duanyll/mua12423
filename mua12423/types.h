#pragma once
#include "basic_types.h"
#include "table.h"
#include "function.h"

namespace mua {
namespace types {
typedef managed_pointer<table, TABLE> table_pointer;
typedef managed_pointer<function, FUNCTION> function_pointer;
}
}  // namespace mua