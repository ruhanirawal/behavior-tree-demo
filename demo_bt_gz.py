import time
import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist, PoseStamped
from nav_msgs.msg import Odometry
from py_trees.common import Status
import py_trees


# Custom Behaviors
class ApproachObject(py_trees.behaviour.Behaviour):
    def __init__(self, name, publisher):
        super().__init__(name)
        self.publisher = publisher  

    def update(self):
        print(f"Approaching the table: {self.name}")
        
        # Sending a simple velocity command to approach
        msg = Twist()
        msg.linear.x = 0.2  # Move forward at a moderate speed
        self.publisher.publish(msg)
        
        time.sleep(5)  # Simulating 5 seconds of motion
        msg.linear.x = 0.0  # Stop the robot after moving
        self.publisher.publish(msg)
        
        return Status.SUCCESS


class WaitForSeconds(py_trees.behaviour.Behaviour):
    def __init__(self, name, seconds=10):
        super().__init__(name)
        self.seconds = seconds

    def update(self):
        print(f"Waiting for {self.seconds} seconds")
        time.sleep(self.seconds)
        return Status.SUCCESS


class SayMessage(py_trees.behaviour.Behaviour):
    def __init__(self, name, message="Enjoy your meal"):
        super().__init__(name)
        self.message = message

    def update(self):
        print(self.message)
        return Status.SUCCESS


class DisplaySmiley(py_trees.behaviour.Behaviour):
    def __init__(self, name):
        super().__init__(name)

    def update(self):
        print("Displaying: 😊")  
        return Status.SUCCESS


class Turn360(py_trees.behaviour.Behaviour):
    def __init__(self, name, publisher):
        super().__init__(name)
        self.publisher = publisher  # Publisher for robot's rotation

    def update(self):
        print("Turning 360 degrees...")
        
        # Rotate the robot 360 degrees
        msg = Twist()
        msg.angular.z = 0.5  # Set rotational speed for turning
        self.publisher.publish(msg)
        
        time.sleep(6)  # Simulate 360-degree turn (based on angular speed)
        
        msg.angular.z = 0.0  # Stop rotation
        self.publisher.publish(msg)
        
        return Status.SUCCESS


class ReturnToStart(py_trees.behaviour.Behaviour):
    def __init__(self, name, publisher):
        super().__init__(name)
        self.publisher = publisher  # Publisher for returning to start

    def update(self):
        print("Returning to the starting position...")
        
        # Move robot backward to return to starting position
        msg = Twist()
        msg.linear.x = -0.2  # Move backward at a moderate speed
        self.publisher.publish(msg)
        
        time.sleep(5)  # Simulate return journey
        
        msg.linear.x = 0.0  # Stop the robot after moving
        self.publisher.publish(msg)
        
        return Status.SUCCESS


class GoToGoal(py_trees.behaviour.Behaviour):
    def __init__(self, name, publisher, initial_position, node, distance_threshold=0.1):
        super().__init__(name)
        self.publisher = publisher
        self.goal_position = initial_position  # Will be set dynamically
        self.distance_threshold = distance_threshold  # Distance to stop before goal
        self.current_position = (0.0, 0.0)  # Default position
        self.node = node  # Store the node to create a subscription

        # Create the subscription to '/odom' for getting robot's position
        self.pose_subscriber = self.node.create_subscription(
            Odometry, '/odom', self.odom_callback, 10
        )

        # Create a publisher to send goal position (PoseStamped)
        self.goal_publisher = self.node.create_publisher(PoseStamped, '/move_base_simple/goal', 10)

    def update(self):
        # Calculate the distance to the goal
        distance_to_goal = math.sqrt(
            (self.goal_position[0] - self.current_position[0])**2 + 
            (self.goal_position[1] - self.current_position[1])**2
        )

        # Print the distance to goal for debugging
        print(f"Distance to goal: {distance_to_goal} meters")

        if distance_to_goal > self.distance_threshold:
            # Move towards the goal if the distance is greater than the threshold
            msg = Twist()
            msg.linear.x = 0.2  # Move forward at a moderate speed
            self.publisher.publish(msg)

            # Check if we reached the desired distance to set a new goal
            if distance_to_goal <= 6.0792343775993825:
                print(f"Setting new goal at current position: {self.current_position}")
                self.goal_position = self.current_position  # Set goal to current position
                self.set_goal()  # Update the goal

            return Status.RUNNING  # Continue moving until within threshold
        else:
            # Reached the goal
            print("Goal Reached!")
            msg = Twist()
            msg.linear.x = 0.0  # Stop the robot
            self.publisher.publish(msg)
            return Status.SUCCESS  # Stop the robot once goal is reached

    def odom_callback(self, msg: Odometry):
        # Extract the position (x, y) from the Odometry message
        self.current_position = (
            msg.pose.pose.position.x,
            msg.pose.pose.position.y
        )

    def set_goal(self):
        # Publish the goal location as a PoseStamped message
        goal_msg = PoseStamped()
        goal_msg.header.frame_id = "map"  # Specify the coordinate frame
        goal_msg.header.stamp = self.node.get_clock().now().to_msg()
        goal_msg.pose.position.x = self.goal_position[0]
        goal_msg.pose.position.y = self.goal_position[1]
        goal_msg.pose.position.z = 0.0
        goal_msg.pose.orientation.w = 1.0  # Set orientation to 0 (no rotation)

        # Publish the goal message to /move_base_simple/goal
        self.goal_publisher.publish(goal_msg)


class BehaviorTreeNode(Node):
    def __init__(self):
        super().__init__('behavior_tree_node')

        # Define the initial goal position (coordinates of the green circle or any target)
        initial_goal_position = (0.0, 0.0)  # Initial arbitrary position

        # Create a publisher for controlling the robot's velocity
        self.publisher = self.create_publisher(Twist, '/cmd_vel', 10)

        # Create the behavior tree
        root = py_trees.composites.Sequence(name="Root", memory=None)
        
        # Add behaviors to the tree
        root.add_children([
            GoToGoal("GoToGoal", self.publisher, initial_goal_position, self, distance_threshold=0.1),
            WaitForSeconds("WaitForSeconds", seconds=5),
            SayMessage("SayMessage"),
            DisplaySmiley("DisplaySmiley"),
            Turn360("Turn360", self.publisher),
            ReturnToStart("ReturnToStart", self.publisher)
        ])

        # Initialize behavior tree
        self.behavior_tree = py_trees.trees.BehaviourTree(root)

        # Create a timer to tick the behavior tree
        self.timer = self.create_timer(0.1, self.tick_tree)

    def tick_tree(self):
        # Update behavior tree each cycle
        self.behavior_tree.tick()


def main(args=None):
    rclpy.init(args=args)

    # Instantiate the node that will run the behavior tree
    node = BehaviorTreeNode()

    # Keep the ROS 2 node running
    rclpy.spin(node)

    rclpy.shutdown()


if __name__ == '__main__':
    main()

