#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{   
    ROS_INFO_STREAM("Robot is driving for the white ball!");
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    // TODO: Request a service and pass the velocities to it to drive the robot
    if (!client.call(srv))
        ROS_ERROR("Failed to call service driveToTarget");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    int CameraHeight = img.height;
    int CameraStep = img.step;
    int DetectionCnt = 0;
    float CenterStep = CameraStep/2;
    float Error=0; // Error from center step
    float TargetX = 0;  //Target linear x velocity
    float TargetZ = 0;  //Target Angular Z 

    for (int i = 0; i < CameraHeight ; i++) {
        for (int j = 0; j < CameraStep; j++) {
            if (img.data[i * CameraStep + j] == white_pixel) {           
                Error += j - CenterStep;
                DetectionCnt++;
            }
        }
    }
    ROS_INFO_STREAM("Step length is " +std::to_string(CameraStep));
    ROS_INFO_STREAM("Error is " +std::to_string(-Error/DetectionCnt/240));
    if (DetectionCnt !=0){TargetX = 1; TargetZ = -Error/DetectionCnt/240;}
    else {TargetX = 0; TargetZ=0; ROS_INFO_STREAM("There is no white ball in front of camera");}
    drive_robot(TargetX, TargetZ);


    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
