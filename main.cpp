#include "./controller/client/ControllerClient.h"
#include "./controller/raid/RAID5Manager.h"
#include "./controller/utils/Logger.h"

int main() {
    Logger::Info("Iniciando TECMFS Controller...");

    ControllerClient controller;

    controller.SendPing();

    RAID5Manager raid(controller);
    int blocksWritten = raid.WriteFile("proyecto.pdf");
    raid.ReadFile("reconstruido_raid5.pdf", blocksWritten);

    Logger::Info("Finalizó la operación del controlador.");
    return 0;
}
