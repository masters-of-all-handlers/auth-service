#include "check_of_input_data.h"

using namespace std;

bool IsRegistrationDataValid(const std::string_view login,
                             const std::string_view password) {
  return !login.empty() && !password.empty();
}