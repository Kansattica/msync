#include <frozen/unordered_set.h>

bool is_good(int n)
{
    constexpr frozen::unordered_set<int, 5> good_numbers{420, 100, 2, 22, 72};

    return good_numbers.count(n) > 0;
}