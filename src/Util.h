#ifndef SL_UTIL_H
#define SL_UTIL_H

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/viz/viz3d.hpp>

#include <cmath>
#include <queue>

#include "../lib/PolynomialRegression.h"


#define PI 3.14159265358979


enum class DIRECTION {VERTICAL, HORIZONTAL};


inline uint8_t RandInt()
{
    return (rand() % 256);
}


class PlaybackHandler
{

};


/**
 * input thread will push to the frames, worker frames will pop and dispatch it to the processing
 * NOTE: this is only necessary if we're doing live processing
 */
class WorkDispatcher
{
    std::queue<cv::Mat> frames;
};


// class KeyInputHandler
// {
// public:
//     enum class KEY
//     {
//         SPACE = 32,
//         UP = 82, 
//         DOWN = 84,
//         NOKEY = -1
//     };
// };


#endif