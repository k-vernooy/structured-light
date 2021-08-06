#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;


class LineProcessor
{
    void addFrame(const Mat& frame)
    {
        assert((!frame.empty()));
    }

    // void process()
    // {
    //     GaussianBlur(frame);
    // }

private:
    Mat frame;
};


int main(int argc, char** argv)
{
    // int deviceID = stoi(argv[1]);
    Mat frame;
    VideoCapture cap(argv[1]);
    // cap.open(deviceID, cv::CAP_ANY);
    
    if (!cap.isOpened())
    {
        cerr << "Unable to open camera\n";
        return -1;
    }

    cap.read(frame);

    int horiz = 1;
    Size direction = {11, 1};
    
    while (true)
    {
        if (frame.empty())
        {
            cerr << "blank frame grabbed\n";
            break;
        }
        
        // cv::cvtColor(frame, frame, COLOR_BGR2GRAY);

        // Mat kernel = (Mat_<int>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
        // cv::filter2D(frame, frame, CV_64F, kernel);

        // std::cout << frame.rows << ", " << frame.cols << std::endl;
        // imshow("Live", frame);

        Mat src, frame_gray, dst, abs_dst;
        int kernel_size = 3;
        int scale = 1;
        int delta = 0;
        int ddepth = CV_16S;
        
        GaussianBlur(frame, frame_gray, direction, 3, 3, BORDER_DEFAULT);
        cvtColor(frame_gray, frame_gray, COLOR_BGR2GRAY);
        Laplacian(frame_gray, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT);
        convertScaleAbs(dst, abs_dst);
        imshow("Live", abs_dst);

        char key = (char) waitKey(30);
        if (key == 32)
        {
            if (horiz == 1)
            {
                horiz = 0;
                direction = {1, 11};
            }
            else
            {
                horiz = 1;
                direction = {11, 1};
            }
        }
        else if (key != -1) break;
    }

    return 0;
}