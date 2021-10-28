#ifndef SL_PINHOLE_H
#define SL_PINHOLE_H

#include "StructuredLight.h"


class Pinhole
{
    // represent matrices of a pinhole obj
    cv::Mat intrinsics;

    // "distortion coefficient vector"
    cv::Mat distortion;
    
    // rotation, translation
    cv::Mat extrinsics;
};


class Projector : public Pinhole
{
public:
    StructuredLight knownProjection;
};


class Camera : public Pinhole
{
};

#endif