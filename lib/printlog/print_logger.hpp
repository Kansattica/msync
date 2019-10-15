#ifndef _PRINTLOG_HPP_
#define _PRINTLOG_HPP_

#include <iostream>
#include <fstream>

using std::cout;

enum class logtype
{
    normal,
    verbose,
    fileonly
};
extern bool verbose_logs;
extern bool logs_off;

template <logtype isverbose = logtype::normal>
struct print_logger
{
    print_logger(std::ofstream& file) : logfile(file) {}

    void flush()
    {
        cout.flush();
        logfile.flush();
    }

    template <typename T>
    print_logger& operator<<(const T& towrite)
    {
		if (logs_off)
			return *this;

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
    std::ofstream& logfile;
};

print_logger<logtype::normal>& pl();
print_logger<logtype::verbose>& plverb();
print_logger<logtype::fileonly>& plfile();
#endif