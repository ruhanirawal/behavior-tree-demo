#include <iostream>
#include <chrono>
#include <functional>
#include <thread>
#include "rclcpp/rclcpp.hpp"
#include "behaviortree_cpp_v3/action_node.h"
#include "behaviortree_cpp_v3/bt_factory.h"

using namespace std::chrono_literals;

class ApproachObject : public BT::SyncActionNode
{
public:
  explicit ApproachObject(const std::string &name) : BT::SyncActionNode(name, {})
  {
  }

  BT::NodeStatus tick() override
  {
    std::cout << "Approaching the table: " << this->name() << std::endl;
    std::this_thread::sleep_for(5s); // Simulating approaching the table
    return BT::NodeStatus::SUCCESS;
  }
};

class WaitForSeconds : public BT::SyncActionNode
{
public:
    // Constructor remains the same
    WaitForSeconds(const std::string& name) : BT::SyncActionNode(name, {}) {}

    virtual BT::NodeStatus tick() override
    {
        std::this_thread::sleep_for(std::chrono::seconds(seconds_));
        return BT::NodeStatus::SUCCESS;
    }

private:
    int seconds_ = 10;  // Default to 10 seconds
};


class SayMessage : public BT::SyncActionNode
{
public:
    // Constructor remains the same, no input parameters
    SayMessage(const std::string& name) : BT::SyncActionNode(name, {}) {}

    virtual BT::NodeStatus tick() override
    {
        std::cout << message_ << std::endl;
        return BT::NodeStatus::SUCCESS;
    }

private:
    std::string message_ = "Enjoy your meal";  // Default message
};

class DisplaySmiley : public BT::SyncActionNode
{
public:
    // Constructor remains the same, no input parameters
    DisplaySmiley(const std::string& name) : BT::SyncActionNode(name, {}) {}

    BT::NodeStatus tick() override
    {
        std::cout << "Displaying: 😊" << std::endl; // Simulate displaying a smiley
        return BT::NodeStatus::SUCCESS;
    }
};

class Turn360 : public BT::SyncActionNode
{
public:
  explicit Turn360(const std::string &name) : BT::SyncActionNode(name, {}) {}

  BT::NodeStatus tick() override
  {
    std::cout << "Turning 360 degrees..." << std::endl;
    std::this_thread::sleep_for(3s); // Simulate the time to turn
    return BT::NodeStatus::SUCCESS;
  }
};

class ReturnToStart : public BT::SyncActionNode
{
public:
  explicit ReturnToStart(const std::string &name) : BT::SyncActionNode(name, {}) {}

  BT::NodeStatus tick() override
  {
    std::cout << "Returning to the starting position..." << std::endl;
    std::this_thread::sleep_for(5s); // Simulate the return to start
    return BT::NodeStatus::SUCCESS;
  }
};

class BehaviorTreeNode : public rclcpp::Node
{
public:
    // Constructor with NodeOptions
    BehaviorTreeNode(const std::string& node_name, const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
        : Node(node_name, options)
    {
        BT::BehaviorTreeFactory factory;

        // Register the custom actions
        factory.registerNodeType<ApproachObject>("ApproachTable");
        factory.registerNodeType<WaitForSeconds>("WaitForSeconds");
        factory.registerNodeType<SayMessage>("SayMessage");
        factory.registerNodeType<DisplaySmiley>("DisplaySmiley");
        factory.registerNodeType<Turn360>("Turn360");
        factory.registerNodeType<ReturnToStart>("ReturnToStart");

        // Create Tree from XML file provided as parameter
        tree_ = factory.createTreeFromFile(this->get_parameter("bt_tree_file").as_string());

        // Timer to regularly call the behavior tree's tickRoot method
        timer_ = this->create_wall_timer(
          100ms, std::bind(&BehaviorTreeNode::tickTree, this));
    }

private:
    void tickTree()
    {
        tree_.tickRoot();
    }

    BT::Tree tree_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    if (argc < 2)
    {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Usage: behavior_tree_node <bt_tree_file>");
        return 1;
    }

    // Initialize the ROS 2 node and load the behavior tree file as a parameter
    rclcpp::NodeOptions options;
    options.parameter_overrides({
        {"bt_tree_file", argv[1]}
    });

    // Create and spin the node
    auto node = std::make_shared<BehaviorTreeNode>("behavior_tree_node", options);
    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
