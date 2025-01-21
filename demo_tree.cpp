#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Bool.h>
#include <behavior_tree_cpp/behavior_tree.h>
#include <iostream>

// Global variables
ros::Publisher cmd_pub;

bool object_detected = false;

void objectDetectionCallback(const std_msgs::Bool::ConstPtr& msg)
{
    object_detected = msg->data;
}

class Rotate360 : public BT::ActionNodeBase
{
public:
    Rotate360(const std::string& name) : BT::ActionNodeBase(name)
    {
        cmd_pub = ros::NodeHandle().advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    }

    BT::NodeStatus tick() override
    {
        if (object_detected)
        {
            ROS_INFO("Object detected! Performing 360-degree turn.");
            geometry_msgs::Twist move_cmd;
            move_cmd.angular.z = 1.0; 
            cmd_pub.publish(move_cmd);
            
            ros::Duration(6).sleep();  
            move_cmd.angular.z = 0;   
            cmd_pub.publish(move_cmd);

            ROS_INFO("360-degree rotation completed.");
            return BT::NodeStatus::SUCCESS; 
        }
        else
        {
            ROS_INFO("No object detected. Waiting...");
            return BT::NodeStatus::FAILURE; 
        }
    }
};


class ObjectDetectedCondition : public BT::ConditionNodeBase
{
public:
    ObjectDetectedCondition(const std::string& name) : BT::ConditionNodeBase(name) {}

    BT::NodeStatus tick() override
    {
        if (object_detected)
        {
            ROS_INFO("Object detected condition met!");
            return BT::NodeStatus::SUCCESS;
        }
        else
        {
            ROS_INFO("Object not detected.");
            return BT::NodeStatus::FAILURE;
        }
    }
};


BT::Tree createTree()
{

    BT::SequenceNode* root = new BT::SequenceNode("Root");

    
    BT::ConditionNodeBase* object_detected_condition = new ObjectDetectedCondition("Object Detected");

    
    BT::ActionNodeBase* rotate_360_action = new Rotate360("Rotate 360 Degrees");

    
    root->addChild(object_detected_condition);
    root->addChild(rotate_360_action);

    BT::Tree tree;
    tree.rootNode(root);
    return tree;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "robot_behavior_tree_cpp");
    ros::NodeHandle nh;

    
    ros::Subscriber object_detected_sub = nh.subscribe("/object_detected", 10, objectDetectionCallback);

    
    BT::Tree tree = createTree();

    
    ros::Rate rate(10); 
    while (ros::ok())
    {
        tree.tickRoot(); 
        ros::spinOnce(); 
        rate.sleep();
    }

    return 0;
}
