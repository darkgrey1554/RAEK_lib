#include "struct_enum.h"


std::string CreateNameMutexMemory(TypeData TD, TypeValue TV, int channel)
{
    std::string str;
    str += "Global\\MUTEX_";
    if (TD == TypeData::Analog)
    {
        str += "A";
    }
    else if (TD == TypeData::Discrete)
    {
        str += "D";
    }
    else if (TD == TypeData::Binar)
    {
        str += "B";
    }

    if (TV == TypeValue::INPUT)
    {
        str += "IN";
    }
    else if (TV == TypeValue::INPUT)
    {
        str += "OUT";
    }

    str += "_CHANNEL_" + std::to_string(channel);
    return str;
}

std::string CreateNameMemory(TypeData TD, TypeValue TV, int channel)
{
    std::string str;
    str += "Global\\MAPFILE_";
    if (TD == TypeData::Analog)
    {
        str += "A";
    }
    else if (TD == TypeData::Discrete)
    {
        str += "D";
    }
    else if (TD == TypeData::Binar)
    {
        str += "B";
    }

    if (TV == TypeValue::INPUT)
    {
        str += "IN";
    }
    else if (TV == TypeValue::INPUT)
    {
        str += "OUT";
    }

    str += "_CHANNEL_" + std::to_string(channel);
    return str;
}