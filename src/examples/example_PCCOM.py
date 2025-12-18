# !/usr/bin/env python3
# Autor: Ruben Sahuquillo y Pablo Navarro
# Este script prueba la comunicacion, interfaz y camara utilizando la libreria ComRobotLib.

# --- LIBRERÍAS --- #
from ComRobotLib import RobotComm, Interface
import time
import threading
import cv2

# Instanciar la comunicacion con los robots
robot_comm = RobotComm(ip="10.74.94.237", logfile="datalog.txt")

# Registrar robots
robot_comm.addRobot(0)
robot_comm.addRobot(1)
robot_comm.addRobot(2)

# Configurar la cámara
print("Abriendo cámara...")
cap = cv2.VideoCapture(0)

# Verificar que la cámara se abrió correctamente
if not cap.isOpened():
    print("No se puede abrir la cámara ID 0")
    exit(1)

print("Cámara abierta correctamente")

# Probar captura de un frame
ret, test_frame = cap.read()
if not ret:
    print("La cámara se abrió pero no puede capturar frames")
    cap.release()
    exit(1)

print(f"Frame de prueba capturado: {test_frame.shape}")


# Bucle de comunicacion
def comm_loop():
    """
    Bucle infinito de envio y recepcion de datos
    """
    global ang, dist, out
    i = 0
    # Bucle infinito de envio y recepcion de datos
    while True:
        # Seleccionar el robot actual
        id_robot = robot_comm.robots[i]
        # Enviar comando al robot
        robot_comm.enviarRobot(id_robot, ang , dist, out)
        time.sleep(0.2)
        # Recibir respuesta del robot
        robot_comm.recibirRespuesta()

        # Actualizar variables de prueba
        if ang >= 360:
            ang = 0

        if dist >= 10:
            dist = 0

        ang += 5
        dist += 1

        # Cambiar al siguiente robot
        i += 1

        # Reiniciar el indice si se supera el numero de robots
        if i >= len(robot_comm.robots):
            i = 0


# Bucle de captura de cámara
def camera_loop():
    """Captura frames y los envía a la interfaz"""
    print("Iniciando bucle de cámara...")
    frame_count = 0
    # Bucle infinito de captura
    while True:
        ret, frame = cap.read()
        # Si la captura fue exitosa, enviar el frame a la interfaz
        if ret:
            interface.update_frame(frame)
            frame_count += 1
            # Mostrar contador cada 100 frames
            if frame_count % 100 == 0:
                print(f"Frames capturados: {frame_count}")

        # Si no fue exitosa, mostrar advertencia
        else:
            print("WARNING: No se pudo leer frame de la cámara")
            time.sleep(0.1)
        time.sleep(0.033)  # Aproximadamente 30 FPS


# Instanciar la interfaz web
interface = Interface(robot_comm)

# Lanzar el bucle de comunicación en un hilo aparte
t_comm = threading.Thread(target=comm_loop, daemon=True)
t_comm.start()
print("Hilo de comunicación iniciado")

# Lanzar el bucle de cámara en un hilo aparte
t_camera = threading.Thread(target=camera_loop, daemon=True)
t_camera.start()
print("Hilo de cámara iniciado")


# Mostrar mensaje de inicio de interfaz
print("\n" + "="*60)
print("Iniciando servidor web en http://localhost:5000")
print("Presiona Ctrl+C para detener")
print("="*60 + "\n")


# Ejecutar la interfaz web
try:
    # Opcion 1: Flask en el hilo principal
    interface.run_server(debug=False)
    
    # Opcion 2: Flask en otro hilo (descomentar para usar)
    # t_flask = threading.Thread(target=lambda: interface.run_server(debug=False), daemon=True)
    # t_flask.start()
    # 
    # # Mantener el programa vivo
    # while True:
    #     time.sleep(1)
    
# Capturar Ctrl+C para cerrar recursos
except KeyboardInterrupt:
    print("\n[CTRL+C] Deteniendo programa...")
    cap.release()
    robot_comm.close()
    print("Recursos liberados")
