
#ifndef __Common_h_
#define __Common_h_

#include <iostream>

template <typename Desc>
void log(const char* desc, Desc desc2)
{
    std::cout << desc << ": " << desc2 << std::endl;
}

void log(const char* desc)
{
    std::cout << desc << std::endl;
}


#endif
