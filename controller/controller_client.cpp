#include <iostream>
#include "./raid/RAID5Manager.h"

int main() {
    RAID5Manager manager;

    int blocksWritten = manager.WriteFileRAID5("proyecto.pdf");
    manager.ReadFileRAID5("reconstruido_raid5.pdf", blocksWritten);

    return 0;
}
