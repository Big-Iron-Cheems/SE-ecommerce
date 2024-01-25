#include "Customer.h"

std::string Customer::toString() const {
    std::ostringstream oss;
    oss << "Customer: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}
