from ComRobotLib import RobotComm
import time

#robot_comm = RobotComm(ip="192.168.137.232", logfile="datalog.txt")
robot_comm = RobotComm(ip="10.74.94.237", logfile="datalog.txt")

# Registrar robots
robot_comm.addRobot(0)
robot_comm.addRobot(1)

ang = 0.0
dist = 0.0
out = False

def comm_loop():
    global ang, dist, out
    i = 0

    while True:
        id_robot = robot_comm.robots[i]

        robot_comm.enviarRobot(id_robot, ang , dist, out)
        time.sleep(0.2)
        robot_comm.recibirRespuesta()

        ang += 5
        dist += 1

        i += 1
        if i >= len(robot_comm.robots):
            i = 0

if __name__ == "__main__":
    comm_loop()