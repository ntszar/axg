#ifndef CONTACTINFO_H
#define CONTACTINFO_H

#include <map>
#include <string>
struct ContactInfo
{
public:
    ContactInfo(unsigned int uin, const std::string &showName, const std::string nickName, const bool isBuddy, const bool isNormal, const bool isFriend)
        : uin(uin), showName(showName), nickName(nickName), isBuddy(isBuddy), isNormal(isNormal), isFriend(isFriend)
    {

    }

    unsigned int uin;
    std::string showName;
    std::string nickName;
    bool isBuddy;
    bool isNormal;
    bool isFriend;

};

#endif // CONTACTINFO_H