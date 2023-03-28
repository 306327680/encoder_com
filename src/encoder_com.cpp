//
// Created by echo on 23-3-18.
//
//Novatel command : RAWDMI odom1 0 0 0 1 (odom1 is ticks from odom)

#include "../include/encoder_com/encoder_com.h"
#include "ros/ros.h"
#include <serial/serial.h>
#include <unistd.h>
#include "std_msgs/Int32.h"
#include "nav_msgs/Odometry.h"
serial::Serial ser;
 //0x01,0x03,0x00,0x10,0x00,0x02,0xC5,0xCE,0x01,0x03,0x00,0x14,0x00,0x02,0x84,0x0F
std::uint8_t ask[16]={0x01,0x03,0x00,0x10,0x00,0x02,0xC5,0xCE,
                      0x01,0x03,0x00,0x14,0x00,0x02,0x84,0x0F};

std::uint8_t receive[18] ={0x01,0x03,0x00,0x10,0x00,0x02,0xC5,0xCE,
                           0x01,0x03,0x00,0x14,0x00,0x02,0x84,0x0F};;

/*char receive_char[16] ={0x01,0x03,0x00,0x10,0x00,0x02,0xC5,0xCE,
                           0x01,0x03,0x00,0x14,0x00,0x02,0x84,0x0F};;*/
std::string port = "/dev/ttyUSB0";
int baudrate = 115200;

int main(int argc, char** argv)
{
    ros::init(argc, argv, "encoder_com");
    // 声明节点句柄  局部
    ros::NodeHandle nh("~");

    try
    { // 设置串口属性，并打开串口
        ser.setPort(port);
        // ser.setPort("/dev/ttyUSB0");
        ser.setBaudrate(baudrate);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
        ser.setTimeout(to);
        ser.open();
    }
    catch (serial::IOException& e)
    {
        ROS_ERROR_STREAM("Unable to open port ");
        return -1;
    }
    // 检测串口是否已经打开，并给出提示信息
    if(ser.isOpen()){
        ROS_INFO_STREAM("Serial Port initialized");
    }
    else{
        return -1;
    }
    ros::Publisher encoder1_pub = nh.advertise<std_msgs::Int32>("encoder1", 1000);
    ros::Publisher encoder2_pub = nh.advertise<std_msgs::Int32>("encoder2", 1000);
    ros::Publisher encoder_pub = nh.advertise<nav_msgs::Odometry>("encoder", 1000);
    ros::Rate r(100); // 10 hz
    while(ros::ok()){
        size_t n = ser.available();
        ser.write(ask,16);   //发送串口数据
        char aaa[1];
        std::string odom1;
        std::string odom2;
        std::stringstream ss;
//        std::cout<<n<<std::endl;
        ser.read(receive,ser.available());
        if(n==17){
            for(int i=0; i<n; i++){
                std::sprintf(aaa,"%02X",receive[i]);
                if (i<8){
                    ss << std::hex <<  (receive[i] & 0xff);
                    odom1 += aaa;
                } else if(i<16){
                    ss << std::hex <<  (receive[i] & 0xff);
                    odom2 += aaa;
                }
            }
            std_msgs::Int32 msg;
            nav_msgs::Odometry encoder;
            encoder.header.frame_id ="wheel";
            encoder.header.stamp = ros::Time::now();

            int o1,o2;
            o1 = std::stoul(odom1,nullptr,16);
            o2 = std::stoul(odom2,nullptr,16);

            msg.data = o1/1000;
            encoder1_pub.publish(msg);
            msg.data = o2/1000;
            encoder2_pub.publish(msg);
            encoder.pose.pose.position.x = o1/1000;
            encoder.pose.pose.position.y = o2/1000;
            std::cout <<  o1/1000<<"  "<<o2/1000  << std::endl;
        }

        ros::spinOnce();
        r.sleep();
    }

    return 0;
}