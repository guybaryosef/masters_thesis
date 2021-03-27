

#include <iostream>
#include "slot_map.h"


class LockedSlotMap
{
public:

    LockedSlotMap()
    {
        a = 1;
        std::cout << "a is " << a << std::endl;
    }

private:
    int a;
    stdext::slot_map<int> map;
};