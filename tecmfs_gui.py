import PySimpleGUI as sg
import subprocess
import os

def run_command(command):
    try:
        output = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT, text=True)
        return output
    except subprocess.CalledProcessError as e:
        return f"[ERROR] {e.output}"

layout = [
    [sg.Text("TEC Media File System (RAID 5)", font=("Helvetica", 16), justification='center')],
    
    [sg.Frame("Subir archivo", [
        [sg.Input(key="-FILE_WRITE-"), sg.FileBrowse()],
        [sg.Button("Subir a RAID 5")]
    ])],

    [sg.Frame("Leer archivo", [
        [sg.Text("Nombre original:"), sg.Input(key="-FILE_READ_ORIG-")],
        [sg.Text("Archivo de salida:"), sg.Input(key="-FILE_READ_OUT-"), sg.FileSaveAs()],
        [sg.Button("Leer y reconstruir")]
    ])],

    [sg.Frame("Eliminar archivo", [
        [sg.Text("Nombre del archivo:"), sg.Input(key="-FILE_DELETE-")],
        [sg.Button("Eliminar")]
    ])],

    [sg.Frame("Archivos almacenados", [
        [sg.Button("Listar archivos")]
    ])],

    [sg.Output(size=(100, 25), key='-OUTPUT-')],
    [sg.Button("Salir")]
]

window = sg.Window("TECMFS GUI", layout)

while True:
    event, values = window.read()
    if event == sg.WINDOW_CLOSED or event == "Salir":
        break

    if event == "Subir a RAID 5":
        filepath = values["-FILE_WRITE-"]
        if filepath:
            cmd = f"./build/controller write '{filepath}'"
            print(run_command(cmd))
        else:
            print("[ERROR] Debes seleccionar un archivo.")

    elif event == "Leer y reconstruir":
        orig = values["-FILE_READ_ORIG-"]
        out = values["-FILE_READ_OUT-"]
        if orig and out:
            cmd = f"./build/controller read '{orig}' '{out}'"
            print(run_command(cmd))
        else:
            print("[ERROR] Debes especificar archivo original y de salida.")

    elif event == "Eliminar":
        filename = values["-FILE_DELETE-"]
        if filename:
            cmd = f"./build/controller delete '{filename}'"
            print(run_command(cmd))
        else:
            print("[ERROR] Debes ingresar un nombre de archivo para eliminar.")

    elif event == "Listar archivos":
        cmd = "./build/controller"
        print(run_command(cmd))

window.close()
