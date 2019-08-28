#include "net.hpp"

#include <cpr/cpr.h>

std::string read_url()
{
    auto r = cpr::Get(cpr::Url{"https://icanhazip.com/"});
    return r.text;
}