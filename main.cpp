#include "controller/client/ControllerClient.h"
#include "controller/raid/RAID5Manager.h"
#include "controller/utils/Logger.h"

int main(int argc, char** argv) {
    Logger::Info("Iniciando TECMFS Controller...");

    ControllerClient controller;
    controller.SendPing();

    RAID5Manager raid(controller);

    // Mostrar archivos almacenados si se llama sin argumentos
    if (argc == 1) {
        raid.ListFiles();
        Logger::Info("Uso:");
        Logger::Info("  ./controller write <archivo>         # Escribir archivo a RAID 5");
        Logger::Info("  ./controller read <archivo> <salida> # Leer y reconstruir archivo");
        Logger::Info("  ./controller delete <archivo>        # Eliminar archivo del RAID 5");
        return 0;
    }

    std::string command = argv[1];

    if (command == "write" && argc == 3) {
        std::string archivoOriginal = argv[2];
        auto [bloques, tamUltimo] = raid.WriteFile(archivoOriginal);
        Logger::Info("Archivo '" + archivoOriginal + "' guardado en RAID 5.");
        Logger::Info("Bloques de datos: " + std::to_string(bloques));
        Logger::Info("Último bloque: " + std::to_string(tamUltimo) + " bytes");

    } else if (command == "read" && argc == 4) {
        std::string archivoOriginal = argv[2];
        std::string archivoRecuperado = argv[3];
        if (raid.ReadFile(archivoRecuperado, archivoOriginal)) {
            Logger::Info("Archivo recuperado como: " + archivoRecuperado);
        } else {
            Logger::Error("No se pudo recuperar el archivo '" + archivoOriginal + "'.");
        }

    } else if (command == "delete" && argc == 3) {
        std::string archivoAEliminar = argv[2];
        raid.DeleteFile(archivoAEliminar);

    } else {
        Logger::Error("Comando no válido o argumentos incorrectos.");
        Logger::Info("Uso:");
        Logger::Info("  ./controller write <archivo>");
        Logger::Info("  ./controller read <archivo> <salida>");
        Logger::Info("  ./controller delete <archivo>");
    }

    Logger::Info("Finalizó la operación del controlador.");
    return 0;
}
