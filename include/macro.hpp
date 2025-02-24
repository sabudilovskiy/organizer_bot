#pragma once

#include <kelcoro/generator.hpp>

#define $await_all(X) co_yield dd::elements_of(X)
