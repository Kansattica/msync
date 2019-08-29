#include <frozen/unordered_set.h>

bool is_good(int n)
{
    constexpr frozen::unordered_set<int, 3> good_numbers{420, 100, 2};

    return good_numbers.count(n) > 0;
}