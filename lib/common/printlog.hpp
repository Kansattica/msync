#ifndef _PRINTLOG_HPP_
#define _PRINTLOG_HPP_

#include <iostream>
#include <fstream>

using std::cout;
using std::ofstream;

enum class logtype
{
    normal,
    verbose
};

template <logtype isverbose = logtype::normal>
struct PrintLogger
{
    PrintLogger::PrintLogger() : logfile("msync.log") {}

    template <typename T>
    PrintLogger &operator<<(const T &towrite)
    {
        if constexpr (isverbose == logtype::verbose)
        {
            if (options.verbose)
                cout << towrite;
        }
        else
        {
            cout << towrite;
        }

        logfile << towrite;
        return *this;
    }

private:
    ofstream logfile;
};
#endif