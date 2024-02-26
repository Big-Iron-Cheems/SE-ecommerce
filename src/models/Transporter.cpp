#include "Transporter.h"

std::string Transporter::getUserType() const { return "Transporter"; }

std::string Transporter::toString() const {
    std::ostringstream oss;
    oss << "Transporter: {\n"
        << "\tid: " << id << "\n"
        << "\tname: " << name << "\n"
        << "\tbalance: " << balance << "\n"
        << "}";
    return oss.str();
}

void Transporter::getBalance() const { User::getBalance(); }

void Transporter::setBalance(const int32_t &balanceChange) { User::setBalance(balanceChange); }
