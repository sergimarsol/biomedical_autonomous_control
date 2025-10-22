// Program rUBot mecanum
#include <ros.h>
#include <ros/time.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Bool.h>

//#include "src/RoboticsUB.h"
#include "encoder.h"
#include "kinematics.hpp"
#include "motor.h"
#include "pid.hpp"
//#include "imu.hpp"

ros::NodeHandle nh;

tf::TransformBroadcaster broadcaster;
geometry_msgs::TransformStamped t;
geometry_msgs::Twist twist;
nav_msgs::Odometry odom;
ros::Publisher odom_pub("odom", &odom);
//ros::Publisher imu_pub("imu",&imu_msg);

// Global Variables
float ctrlrate=1.0;
unsigned long lastctrl;
float x=0,y=0,theta=0;
float vx=0,vy=0,w=0;
float vxi=0,vyi=0,wi=0;

// IMU
const int PIN_IMU_INT = 18;
//float *rpw;
//float *q;
// Motors PID
float KP=0.3,KI=0.2,KD=0.2;
MPID PIDA(encA,KP,KI,KD,true);
MPID PIDB(encB,KP,KI,KD,false);
MPID PIDC(encC,KP,KI,KD,true);
MPID PIDD(encD,KP,KI,KD,false);


void cmdVelCb( const geometry_msgs::Twist& twist_msg){
  vx=twist_msg.linear.x;
  vy=twist_msg.linear.y;
  w=twist_msg.angular.z;

  float pwma=0,pwmb=0,pwmc=0,pwmd=0;
  InverseKinematic(vx,vy,w,pwma,pwmb,pwmc,pwmd);
  
  PIDA.tic();PIDB.tic();PIDC.tic();PIDD.tic();
  MotorA(PIDA.getPWM(pwma));
  MotorB(PIDB.getPWM(pwmb));
  MotorC(PIDC.getPWM(pwmc));  
  MotorD(PIDD.getPWM(pwmd));
  PIDA.toc();PIDB.toc();PIDC.toc();PIDD.toc();

  lastctrl=millis();
}

void resetCb(const std_msgs::Bool& reset){
  if(reset.data){
    x=0.0;y=0.0;theta=0.0;
  }else{
    
  }
}

ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", cmdVelCb );
ros::Subscriber<std_msgs::Bool> resub("reset_odom", resetCb );

void setup()
{
  //pinMode(PIN_IMU_INT, INPUT_PULLUP);
  //imu.Install();
  
  IO_init();
  PIDA.init();
  PIDB.init();
  PIDC.init();
  PIDD.init();

  nh.initNode();
  broadcaster.init(nh);
  nh.subscribe(sub);
  nh.subscribe(resub);
  nh.advertise(odom_pub);
  //nh.advertise(imu_pub);
  lastctrl=millis();
}

void loop(){

  float wA,wB,wC,wD;

  PIDA.tic();
  wA=PIDA.getWheelRotatialSpeed();
  PIDA.toc();

  PIDB.tic();
  wB=PIDB.getWheelRotatialSpeed();
  PIDB.toc();

  PIDC.tic();
  wC=PIDC.getWheelRotatialSpeed();
  PIDC.toc();

  PIDD.tic();
  wD=PIDD.getWheelRotatialSpeed();
  PIDD.toc();

  // Twist vector transition
  ForwardKinematic(wA,wB,wC,wD,vxi,vyi,wi);
  float dt=PIDA.getDeltaT();

  // Odom with transition vxi, vyi
  theta+=wi*dt;
  if(theta > 3.14)
    theta=-3.14;
  x+=vxi*cos(theta)*dt-vyi*sin(theta)*dt;
  y+=vxi*sin(theta)*dt+vyi*cos(theta)*dt;
  
  t.header.stamp = nh.now();
  t.header.frame_id = "odom";
  t.child_frame_id = "base_footprint";
  t.transform.translation.x = x;
  t.transform.translation.y = y;
  t.transform.rotation = tf::createQuaternionFromYaw(theta);
  broadcaster.sendTransform(t);
  
  odom.header.stamp = nh.now();
  odom.header.frame_id = "odom";
  odom.child_frame_id = "base_footprint";
  odom.pose.pose.position.x = x;
  odom.pose.pose.position.y = y;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation =tf::createQuaternionFromYaw(theta);;
  odom.twist.twist.linear.x = vxi;
  odom.twist.twist.linear.y = vyi;
  odom.twist.twist.angular.z = wi;
  odom_pub.publish(&odom);

  // IMU
  //if (digitalRead(PIN_IMU_INT) == HIGH)
  //{
  //  imu.ReadSensor();
  //  q = imu.GetQuaternion();
  //}
  //imu_msg.header.stamp = nh.now();
  //imu_msg.orientation.x = q[0]
  //imu_msg.orientation.y = q[0]
  //imu_msg.orientation.z = q[0]
  //imu_msg.orientation.w = q[0]
  //imu_pub.publish(&imu_msg);

  if((millis()-lastctrl)>1000*ctrlrate){
    STOP();
  }
  nh.spinOnce();
 
}
