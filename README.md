# COM TCP

Bibliotecas de comunicación para LLUBOTS sobre TCP y una interfaz web basada en Flask.

## Resumen

Este repositorio contiene dos librerías para Arduino (master y slave) y un paquete Python que proporciona un cliente de comunicación desde PC y una interfaz web para monitorizar y transmitir frames de cámara.

Componentes principales:

- `MasterComm/` — librería Arduino y ejemplos para el dispositivo maestro.
- `SlaveComm/` — librería Arduino y ejemplos para dispositivos esclavos.
- `src/ComRobotLib/` — paquete Python `ComRobotLib` con las clases `RobotComm` e `Interface`.

## Estructura del repositorio

- MasterComm/
	- src/
	- examples/
- SlaveComm/
- src/
	- ComRobotLib/
		- PCComm.py (comunicación Python y la interfaz Flask)
	- examples/
		- example_PCCOM.py (ejemplo para ejecutar la librería Python)

## Inicio rápido

Python (PC)

1. Instalar dependencias (recomendado Python 3.8+):

```bash
pip install flask opencv-python
```

2. Ejecutar el ejemplo desde la carpeta `src` para que Python encuentre el paquete `ComRobotLib`:

```bash
cd src
python examples/example_PCCOM.py
```

Esto iniciará la captura de la cámara, lanzará los hilos de comunicación en segundo plano y ejecutará un servidor web en `http://localhost:5000`.

Arduino (Master / Slave)

1. Añade las carpetas `MasterComm` y `SlaveComm` desde el IDE (`Sketch > Include Library > Add .ZIP Library`) empaquetas en ZIP.
2. Abre uno de los ejemplos en `MasterComm/examples/` o `SlaveComm/examples/` desde el IDE de Arduino y súbelo a la placa.

Notas

- El cliente Python (`src/ComRobotLib/PCComm.py`) necesita que exista un servidor TCP accesible (el maestro Arduino debe exponer el puerto TCP correspondiente). Ajusta la IP/puerto por defecto en `example_PCCOM.py` o al instanciar `RobotComm`.
- El ejemplo Python usa OpenCV (`cv2`) para la captura y el streaming de vídeo.