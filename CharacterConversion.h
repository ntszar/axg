#ifndef CHARACTERCONVERSION_H
#define CHARACTERCONVERSION_H
#include <boost/locale.hpp>
#include <Wt/WString>
#include <map>
#include "logger.h"
namespace conversions
{
    bool isMapInitialized = false;
    std::map<int,char> latin1ToGG;
    std::map<int,char> ggToLatin1;
    void initializeMap()
    {
        latin1ToGG[-79] = -71;
        latin1ToGG[-95] = -91;
        latin1ToGG[-68] = -97;
        latin1ToGG[-84] = -113;
        latin1ToGG[-74] = -100;
        latin1ToGG[-90] = -116;


        ggToLatin1[-71] = -79;
        ggToLatin1[-91] = -95;
        ggToLatin1[-97] = -68;
        ggToLatin1[-113] = -84;
        ggToLatin1[-100] = -74;
        ggToLatin1[-116] = -90;
        isMapInitialized = true;
    }

std::string fromUtf8(const std::string &utf8)
{
    if(!isMapInitialized)
        initializeMap();
    Wt::WString str(Wt::WString::fromUTF8(utf8));
    return utf8;
    std::string encoded = boost::locale::conv::from_utf(str.operator std::wstring(),"Latin1");
    for(auto it = encoded.begin(); it != encoded.end(); ++it)
    {
        break;
        auto findIt = latin1ToGG.find((int)*it);
        if(findIt != latin1ToGG.end())
        {
            *it = findIt->second;
        }
    }
    return encoded;
}

std::string toUtf8(const std::string &msg)
{
    return msg;
    if(!isMapInitialized)
        initializeMap();
    auto encoded = msg;
    for(auto it = encoded.begin(); it != encoded.end(); ++it)
    {
        break;
        auto findIt = ggToLatin1.find((int)*it);
        if(findIt != ggToLatin1.end())
        {
            *it = findIt->second;
        }
    }
    std::wstring wstr = boost::locale::conv::to_utf<wchar_t>(encoded,"Latin1");
    return Wt::WString(wstr).toUTF8();
}
}
#endif // CHARACTERCONVERSION_H
