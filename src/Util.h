#ifndef SL_UTIL_H
#define SL_UTIL_H

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>

#include "PolynomialRegression.h"

#define PI 3.14159265358979


enum class DIRECTION {VERTICAL, HORIZONTAL};

inline int RandInt()
{
    return (rand() % 256);
}


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