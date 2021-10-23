#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <chrono>
#include <unistd.h>

#include "../src/All.h"


enum class KEYS
{
    NONE, TAB, SPACE, COMMA, PERIOD, UP, DOWN, KEY_1, KEY_2, KEY_3, KEY_4
};

std::map<KEYS, char> keyMap
{
    {KEYS::NONE, -1},
    {KEYS::TAB, 9},
    {KEYS::SPACE, 32},
    {KEYS::COMMA, 44},
    {KEYS::PERIOD, 46},
    {KEYS::UP, 82},
    {KEYS::DOWN, 84},
    {KEYS::KEY_1, 49},
    {KEYS::KEY_2, 50},
    {KEYS::KEY_3, 51},
    {KEYS::KEY_4, 52}
};


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
    LightLineProcessor lp = LightLineProcessor();
    lp.addFrame(frame);

    int unPauseOneFrame = -1;

    // parameters for the realtime vis
    int optype = 0;
    int strength = 31;
    int minsize = 15;
    int padding = 2;
    int rotation = -4;

    std::vector<int> opkeys = {-1, 32, 82, 84};


    while (true)
    {
        // capture a new frame if we're processing a live feed
        if (!isStatic && !isPaused)
        {
            if (unPauseOneFrame != -1) unPauseOneFrame++;
            cap.read(frame);
            // cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point(frame.cols / 2, frame.rows / 2), rotation, 1.0);
            // cv::warpAffine(frame, frame, rotationMatrix, cv::Point(frame.cols, frame.rows));
        }


        lp.addFrame(frame);
        char key = (char) cv::waitKey(1);
        

        if (key == keyMap[KEYS::TAB])
        {
            optype = (optype + 1) % 3;
        }
        else if (key == keyMap[KEYS::SPACE])
        {
            isPaused = !isPaused;
        }
        else if (key == keyMap[KEYS::PERIOD])
        {
            isPaused = false;
            unPauseOneFrame++;
        }


        if (key == keyMap[KEYS::KEY_1]) 
        {
            padding++;
            strength += 2;
            minsize += 2;
        }
        else if (key == keyMap[KEYS::KEY_2])
        {
            padding--;
            if (strength != 1) strength -= 2;
            if (minsize != 0) minsize -= 2;
        }

        if (key == keyMap[KEYS::KEY_3])
        {
            rotation--;
        }

        std::cout << "a " << unPauseOneFrame << std::endl;
        std::cout << isPaused << std::endl;

        if (optype != 0)
            lp.process(static_cast<DIRECTION>(optype - 1), strength, minsize, padding);

        cv::imshow("Live", lp.getProc());

        if (unPauseOneFrame >= 1)
        {
            isPaused = true;
            unPauseOneFrame = -1;
        }

        // if (std::find(opkeys.begin(), opkeys.end(), key) == opkeys.end()) break;
    }


    // cv::Mat pattern(cv::Size(640, 480), CV_8UC3, cv::Scalar(0));

    // while (true)
    // {
    //     StructuredLightGenerator::GenerateLines(pattern);
    //     cv::imshow("test", pattern);
    //     cv::waitKey(0);
    // }

    return 0;
}