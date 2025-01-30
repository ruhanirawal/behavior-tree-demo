#include <iostream>
#include <chrono>
#include <thread>
#include "behaviortree_cpp/action_node.h"
#include "behaviortree_cpp/bt_factory.h"

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
    std::this_thread::sleep_for(5s); // Approaching the table
    return BT::NodeStatus::SUCCESS;
  }
};

class WaitForSeconds : public BT::SyncActionNode
{
public:
  explicit WaitForSeconds(const std::string &name, int seconds) : BT::SyncActionNode(name, {}), _seconds(seconds) {}

  BT::NodeStatus tick() override
  {
    std::cout << "Waiting for " << _seconds << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(_seconds));
    return BT::NodeStatus::SUCCESS;
  }

private:
  int _seconds;
};

class SayMessage : public BT::SyncActionNode
{
public:
  explicit SayMessage(const std::string &name, const std::string &message) : BT::SyncActionNode(name, {}), _message(message) {}

  BT::NodeStatus tick() override
  {
    std::cout << "Speaker says: " << _message << std::endl;
    return BT::NodeStatus::SUCCESS;
  }

private:
  std::string _message;
};

class DisplaySmiley : public BT::SyncActionNode
{
public:
  explicit DisplaySmiley(const std::string &name) : BT::SyncActionNode(name, {}) {}

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

int main()
{
  BT::BehaviorTreeFactory factory;

  // Register the custom actions
  factory.registerNodeType<ApproachObject>("ApproachTable");
  factory.registerNodeType<WaitForSeconds>("WaitForSeconds");
  factory.registerNodeType<SayMessage>("SayMessage");
  factory.registerNodeType<DisplaySmiley>("DisplaySmiley");
  factory.registerNodeType<Turn360>("Turn360");
  factory.registerNodeType<ReturnToStart>("ReturnToStart");

  // Create Tree
  auto tree = factory.createTreeFromFile("./../bt_tree.xml");

  // Execute the tree
  tree.tickWhileRunning();

  return 0;
}
