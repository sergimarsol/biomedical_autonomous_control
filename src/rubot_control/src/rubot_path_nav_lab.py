#!/usr/bin/env python3
import rospy
from rubot_nav import move_rubot
from math import sqrt,sin,cos,pi


def square_path(v,td):
    move_rubot(v,0,0,td)
    move_rubot(0,v,0,td)
    move_rubot(-v,0,0,td)
    move_rubot(0,-v,0,td)


def triangular_path(v, td):
    move_rubot(v,0,0,td)
    move_rubot(-v,v,0,td/sqrt(2))
    move_rubot(-v,-v,0,td/sqrt(2))

def pentagon_path(v, td):
    move_rubot(v,0,0,td)
    move_rubot(-v+cos(2*pi/5)*v,-v*sin(2*pi/5),0,td)
    move_rubot(-v*sin(2*pi/5),v*cos(2*pi/5),0,td)
    move_rubot(0,v,0,td)
    move_rubot(v*sin(2*pi/5),v*cos(2*pi/5),0,td)
    move_rubot(v-cos(2*pi/5)*v,-v*sin(2*pi/5),0,td)

if __name__ == '__main__':
    try:
        rospy.init_node('rubot_nav', anonymous=False)
        v= rospy.get_param("~v")
        w= rospy.get_param("~w")
        td= rospy.get_param("~td")
        path= rospy.get_param("~path")

        if path == "Square":
            square_path(v, td)

        elif path == "Triangular":
            triangular_path(v, td)
            
        elif path == "Pentagon":
            pentagon_path(v, td)
        else:
            print('Error: unknown movement')

    except rospy.ROSInterruptException:
        pass
