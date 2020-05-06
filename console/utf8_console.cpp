// utf8_console.cpp

// Copyright 2019 Michael Thomas Greer
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
//  or copy at https://www.boost.org/LICENSE_1_0.txt )

// Adapted by Grace Lovelace from https://github.com/Duthomhas/utf8_console/blob/master/utf8_console.cpp

//-------------------------------------------------------------------------------------------------
// DESCRIPTION
//-------------------------------------------------------------------------------------------------
// • Makes cin, cout, and cerr (and clog) UTF-8 enabled when attached to the Windows Console.
// • Makes main()'s arguments UTF-8.
// • Requires no changes to written code.

//-------------------------------------------------------------------------------------------------
// USAGE
//-------------------------------------------------------------------------------------------------
// Use "BUILD.bat" to compile "utf8_console.o" / "utf8_console.obj" for your system.
// Then simply link it to your project.
//  • For MSVC and Clang-CL, life is good. We hijack the wmain() entry point to initialize and call main().
//  • For modern MinGW-w64 and Clang-w64 life is also good: Use the -municode -mconsole options when compiling your programs.
//  • For older MinGWs you are stuck with a '.ctor' section function, positioned to construct last. (It works just fine, though.)
//  • For everything else, you are SOL with likely-SIOF code.

//-------------------------------------------------------------------------------------------------
// TECHNICAL NOTES
//-------------------------------------------------------------------------------------------------
// MSVC & Clang-CL
//   Auto-magically changes the actual program entry point to the (totally valid) wmain(), which
//   initializes and calls main() as if nothing happened.
//
// MinGW-w64 & Clang-w64
//   The most recent versions of MinGW-w64 can also use wmain() as the entry point, just like MSVC,
//   by declaring -municode -mconsole at the command line.
//   Prefer this.
//
// MinGW
//   Initializes in the '.ctor' section, which is executed after the '.init' section.
//
//   Initialization short-circuits the usual startup by treating itself as the new entry point,
//   calling main(), and terminating the program.
//
//   Compilation will tell you if you get this version.
//
// Other Compilers
//   There is no current specialization for other compilers. Instead, initialization is activated
//   during globals initialization, meaning you have an even chance of experiencing the Static
//   Initialization Order Fiasco!
//
//   Either use one of MSVC, Clang, or MinGW, or read your compiler's documentation to properly
//   adjust the main entry point through wmain(). If you do this, then tell this library by
//   #defining 'UTF8_CONSOLE_USE_WMAIN' somewhere.
//
// Windows API
//   None of this makes Windows API functions UTF-8 enabled. Unicode Windows functions use UTF-16
//   (or, in some cases, like the Console API, plain old UCS-2), so you must still convert stuff
//   in order to use the 'wchar_t' versions of functions.

//-------------------------------------------------------------------------------------------------
// COMPILATION NOTES
//-------------------------------------------------------------------------------------------------
// It is technically invalid for a C++ program to reference and call main(), but MSVC, Clang, and
// MinGW permit it if you don't tell them to abort on the error.
//
// Specify -municode with MinGW-w64 and Clang w64 when compiling so that the entry point is changed
// to wmain() instead of using the '.ctor' section.
//
// This library is technically C++14, but because of a known bug with the Clang/MSVC combination,
// compile with C++17. Feel free to use any desired C++ standard with your own programs.

#ifdef _WIN32

#include <ciso646>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(lib, "Shell32")
#endif


//-------------------------------------------------------------------------------------------------
// wmain()
//-------------------------------------------------------------------------------------------------

#if defined(_MSC_VER)
  #define EXIT( x ) return x

#elif defined(__GNUC__)
  #if !defined(UNICODE)

  #warning "using 'int wmain() __attribute__ ((constructor (65535)));'"
  int wmain() __attribute__ ((constructor (65535)));

  #endif
#endif

#ifndef EXIT
#define EXIT( x ) exit( x ); return 0
#endif

//-------------------------------------------------------------------------------------------------
extern "C" int main( int, char** );

int wmain()
{
  std::string         args;
  std::vector <char*> argv;

  // Get wide command line arguments
  int          wargc;
  std::wstring wargs;
  LPWSTR*      wargv = CommandLineToArgvW( GetCommandLineW(), &wargc );
  if (wargc and wargv)
  {
    for (int n = 0; n < wargc; n++) wargs.append( wargv[n] ).append( 1, 0 );
    LocalFree( wargv );
  }
  else  // (unlikely, but possible)
  {
    wargs.resize( MAX_PATH + 1 );  // +1 == don't forget the extra null at the end
    wargs.resize( GetModuleFileNameW( NULL, (wchar_t*)wargs.c_str(), MAX_PATH ) );
  }

  // Convert wide argument string to UTF-8
  args.resize( WideCharToMultiByte( CP_UTF8, 0, wargs.c_str(), (int)wargs.size() - 1, NULL, 0, NULL, NULL ) );
  WideCharToMultiByte( CP_UTF8, 0, wargs.c_str(), (int)wargs.size() - 1, (char*)args.c_str(), (int)args.size(), NULL, NULL );

  // Build argv[]
  char* p = (char*)args.c_str();
  argv.push_back( p );
  while (--wargc)
  {
    while (*p) ++p;
    argv.push_back( ++p );
  }
  argv.push_back( nullptr );

  // Finally, call the user's main() and
  // terminate with the returned exit code.
  EXIT( main( (int)argv.size() - 1, (char**)argv.data() ) );
}

//-------------------------------------------------------------------------------------------------
#if !defined(_MSC_VER) && !defined(__GNUC__) && !defined(UTF8_CONSOLE_USE_WMAIN)
  #warning "I don't recognize your compiler. Defaulting to potential C++ SIOF code."
  namespace duthomhas { namespace utf8 { namespace console { int _ = wmain(); } } }
#endif

#endif  // ifdef _WIN32

// end utf8_console.cpp
