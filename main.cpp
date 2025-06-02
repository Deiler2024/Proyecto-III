#include "./controller/client/ControllerClient.h"
#include "./controller/raid/RAID5Manager.h"
#include "./controller/utils/Logger.h"
#include "./controller/utils/MetadataManager.h"
#include "./controller/utils/MetadataManager.h"

int main() {
    Logger::Info("Iniciando TECMFS Controller...");

    // Crear cliente del controlador
    ControllerClient controller;

    // Enviar un ping de prueba
    controller.SendPing();

    // Administrador RAID
    RAID5Manager raid(controller);

    // Escribir archivo en formato RAID 5
    auto [totalBlocks, lastBlockSize] = raid.WriteFile("proyecto.pdf");

    Logger::Info("Archivo escrito exitosamente en bloques RAID 5.");
    Logger::Info("Bloques de datos: " + std::to_string(totalBlocks));
    Logger::Info("Tamaño del último bloque: " + std::to_string(lastBlockSize) + " bytes");

    // Leer el archivo desde RAID 5
    raid.ReadFile("reconstruido_raid5.pdf");

    Logger::Info("Finalizó la operación del controlador.");
    return 0;
}
