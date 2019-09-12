#ifndef _PRINTLOG_HPP_
#define _PRINTLOG_HPP_

#include <fstream>
#include <iostream>

using std::cout;
using std::ofstream;

enum class logtype
{
    normal,
    verbose,
    fileonly
};

extern bool verbose_logs;

template <logtype isverbose = logtype::normal>
struct print_logger
{
    print_logger() : logfile("msync.log", std::ios::out | std::ios::app) {}

    void flush()
    {
        cout.flush();
        logfile.flush();
    }

    template <typename T>
    print_logger& operator<<(const T& towrite)
    {
        if constexpr (isverbose == logtype::verbose)
        {
            if (verbose_logs)
                cout << towrite;
        }
        else if constexpr (isverbose != logtype::fileonly)
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