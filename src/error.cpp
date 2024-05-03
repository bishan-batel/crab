#include "error.hpp"

#include <stdexcept>

const char *crab::error::todo_exception::what() const noexcept {
  return "This function is not implemented yet.";
}