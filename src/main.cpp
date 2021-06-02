
#include <slot_map.h>
#include "locked_slot_map.h"
#include "lock_free_const_sized_slot_map.h"


int main()
{
    stdext::slot_map<int> map;
    gby::locked_slot_map<int> locked_map;
    gby::lock_free_const_sized_slot_map<int, 10> lock_free_map;
    
    return 0;
}

