#include "./controller/client/ControllerClient.h"
#include "./controller/raid/RAID5Manager.h"
#include "./controller/utils/Logger.h"
#include "./controller/utils/MetadataManager.h"

int main() {
    Logger::Info("Iniciando recuperación con fallo simulado en un nodo...");

    // Crear cliente del controlador
    ControllerClient controller;

    // Administrador RAID
    RAID5Manager raid(controller);

    // Leer el archivo desde RAID 5 (usando metadata)
    raid.ReadFile("recuperado_falla.pdf");

    Logger::Info("Recuperación finalizada.");
    return 0;
}
