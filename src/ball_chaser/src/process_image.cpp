#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

/* https://dabit-industries.github.io/turtlebot2-tutorials/14c-OpenCV2_CPP.html */
 // Include opencv2
 #include <opencv2/imgproc/imgproc.hpp>
 #include <opencv2/highgui/highgui.hpp>

 // Include CvBridge, Image Transport, Image msg
 #include <image_transport/image_transport.h>
 #include <cv_bridge/cv_bridge.h>
 #include <sensor_msgs/image_encodings.h>

 // OpenCV Window Name
 static const std::string OPENCV_WINDOW = "Process Image";

 // Topics
 static const std::string IMAGE_TOPIC = "/camera/rgb/image_raw";
 static const std::string PUBLISH_TOPIC = "/image_converter/output_video";
  // Publisher
    ros::Publisher pub;

/* -------------------------------------------------------------------------------------*/


// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot


    ball_chaser::DriveToTarget srv;

    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the DriveToTarget service and pass the requested angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    bool found_ball = false;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int h = 1;
    int w = 1;

    //ROS_INFO_STREAM("Image- \nWidth:" + std::to_string(img.width) + " \nHeight:" + std::to_string(img.height) + " \nStep:" + std::to_string(img.step));
    
    int col ;
    for (int i = 0; i < img.height ; i++) 
    {
        for (int j = 0; j < img.step ; j+=3)
        {
            if (img.data[img.step*i + j] ==255 && img.data[img.step*i + j+1] ==255 && img.data[img.step*i + j+2] ==255)
            {
                found_ball = true;
                col = j/3;
                break;
            }
        }
    }

    float direction;
    float speed = 0.5;
    if (found_ball){
        if (col <= img.width/4){direction = 0.5;}   //turn left
        if (col > img.width/4 && col < img.width*3/4 ){direction = 0.0;}  //go straight
        if (col >= img.width*3/4){direction = -0.5;}  //turn right

        ROS_INFO_STREAM("found_ball :" + std::to_string(found_ball) + " dir:" + std::to_string(direction) + " col:" + std::to_string(col) );         
        
        drive_robot(speed,direction); 

    } else {drive_robot(0,0);}


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
