import PySimpleGUI as sg
import os

# Rutas de los nodos RAID (ajusta si cambian los nombres de las carpetas)
DISK_PATHS = [
    "disk_node_1/storage",
    "disk_node_2/storage",
    "disk_node_3/storage",
    "disk_node_4/storage"
]

BLOCKS_PER_NODE = 100
BLOCK_SIZE = 4096

def get_block_status(path):
    status = []
    for i in range(BLOCKS_PER_NODE):
        block_file = os.path.join(path, f"block_{i}.bin")

        if not os.path.exists(block_file):
            status.append("missing")
        else:
            try:
                with open(block_file, "rb") as f:
                    data = f.read()
                    if all(b == 0 for b in data):
                        status.append("free")
                    else:
                        status.append("used")
            except:
                status.append("missing")
    return status

def build_node_column(node_index, status):
    layout = [[sg.Text(f"Nodo {node_index + 1}", justification='center')]]
    for row in range(10):  # 10x10 bloques por nodo
        row_buttons = []
        for col in range(10):
            idx = row * 10 + col
            color = {
                "used": "gray",
                "free": "white",
                "missing": "red"
            }[status[idx]]

            row_buttons.append(
                sg.Button(
                    "",
                    size=(2, 1),
                    button_color=("black", color),
                    key=f"N{node_index}_B{idx}",
                    pad=(1, 1),
                    disabled=True
                )
            )
        layout.append(row_buttons)
    return sg.Column(layout, element_justification='center', pad=(10, 10), vertical_alignment='top')

def main():
    statuses = [get_block_status(path) for path in DISK_PATHS]

    layout = [
        [sg.Text("Visualización del estado de bloques en nodos RAID 5", font=("Helvetica", 14), justification='center')],
        [
            build_node_column(0, statuses[0]),
            build_node_column(1, statuses[1]),
            build_node_column(2, statuses[2]),
            build_node_column(3, statuses[3]),
        ],
        [sg.Button("Cerrar")]
    ]

    window = sg.Window("Estado de Bloques RAID 5", layout)

    while True:
        event, _ = window.read()
        if event == sg.WINDOW_CLOSED or event == "Cerrar":
            break

    window.close()

if __name__ == "__main__":
    main()
