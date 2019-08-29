#include "net.hpp"

#include <cpr/cpr.h>

std::string read_url(std::string url)
{
    auto r = cpr::Get(cpr::Url{url});
    return r.text;
}