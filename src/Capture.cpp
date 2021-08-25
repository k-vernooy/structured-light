#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <chrono>
#include <unistd.h>

#include "Unified.h"



int main(int argc, char** argv)
{
    srand(time(NULL));

    cv::Mat frame;
    cv::VideoCapture cap;
    bool isStatic = false;

    // initialize opencv video device as either a static image or a camera
    try
    {
        cap.open(std::stoi(argv[1]), cv::CAP_ANY);
    }
    catch (...)
    {
        isStatic = true;
        cap.open(argv[1]);
    }

    assert(cap.isOpened()); 


    // Read a single frame from the video device and add it to the processor
    cap.read(frame);
    LightLineProcessor lp = LightLineProcessor();
    lp.addFrame(frame);

    // parameters for the realtime vis
    int optype = 0;
    int strength = 31;
    int minsize = 15;
    std::vector<int> opkeys = {-1, 32, 82, 84};


    // while (true)
    // {
    //     // capture a new frame if we're processing a live feed
    //     if (!isStatic) cap.read(frame);
    //     lp.addFrame(frame);

    //     char key = (char) cv::waitKey(30);
        
    //     if (key == 32)
    //     {
    //         if (optype == 2) optype = 0;
    //         else optype++;
    //     }
    //     else if (key == 82) 
    //     {
    //         strength += 2;
    //         minsize += 2;
    //     }
    //     else if (key == 84)
    //     {
    //         if (strength != 1) strength -= 2;
    //         if (minsize != 0) minsize -= 2;
    //     }


    //     if (optype != 0)
    //         lp.process(static_cast<DIRECTION>(optype - 1), strength, minsize);

    //     cv::imshow("Live", lp.getProc());

    //     if (std::find(opkeys.begin(), opkeys.end(), key) == opkeys.end()) break;
    // }


    cv::Mat pattern(cv::Size(640, 480), CV_8UC3, cv::Scalar(0));

    while (true)
    {
        StructuredLightGenerator::GenerateLines(pattern);
        cv::imshow("test", pattern);
        cv::waitKey(0);
    }

    return 0;
}