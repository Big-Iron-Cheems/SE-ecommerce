#include "Supplier.h"

std::string Supplier::getUserType() const { return "supplier"; }

std::string Supplier::toString() const {
    std::ostringstream oss;
    oss << "Supplier: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Supplier::getBalance() const { User::getBalance(); }

void Supplier::setBalance(const int32_t &balanceChange) { User::setBalance(balanceChange); }
