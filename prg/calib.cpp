#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <iostream>
#include <chrono>
#include <unistd.h>

#include "../src/All.h"


int main(int argc, char** argv)
{
    srand(time(NULL));

    cv::Mat frame;
    cv::VideoCapture cap;

    bool isStatic = false;
    bool isPaused = false;

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
    std::vector<int> opkeys = {-1, 32, 82, 84};


    while (true)
    {
        cap.read(frame);
        char key = (char) cv::waitKey(1);        
        
        // cv::Mat<cv::Point2f> corners;
        std::vector<cv::Point2f> corners;
        cv::Size boardSize = cv::Size(9, 6);

        int found = cv::findChessboardCorners(frame, boardSize, corners);

        std::cout << found << std::endl;

        if (found)
        {
            std::cout << "found" << std::endl;
            cv::cornerSubPix(frame, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));
            cv::drawChessboardCorners(frame, boardSize, corners, found);
        }
        
        cv::imshow("test", frame);
    }


    return 0;
}