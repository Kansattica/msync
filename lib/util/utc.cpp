// from https://stackoverflow.com/a/59797075/5587653
// Algorithm: http://howardhinnant.github.io/date_algorithms.html

#include <ctime>

inline int days_from_civil(int y, int m, int d) noexcept
{
    y -= m <= 2;
    int era = y / 400;
    int yoe = y - era * 400;                                   // [0, 399]
    int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
    int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;           // [0, 146096]

    return era * 146097 + doe - 719468;
}

// Converts a broken-down time structure with UTC time to a simple time representation.
// It does not modify broken-down time structure as BSD timegm() does.
std::time_t timegm_const(std::tm const* t)
{
    int year = t->tm_year + 1900;
    int month = t->tm_mon;          // 0-11
    if (month > 11)
    {
        year += month / 12;
        month %= 12;
    }
    else if (month < 0)
    {
        int years_diff = (11 - month) / 12;
        year -= years_diff;
        month += 12 * years_diff;
    }
    int days_since_epoch = days_from_civil(year, month + 1, t->tm_mday);

    return 60 * (60 * (24L * days_since_epoch + t->tm_hour) + t->tm_min) + t->tm_sec;
}
