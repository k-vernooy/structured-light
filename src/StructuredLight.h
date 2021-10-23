#ifndef SL_STRUCTUREDLIGHT_H
#define SL_STRUCTUREDLIGHT_H

#include "Util.h"

class Plane
{

};


class StructuredLight
{
public:
    cv::Mat data;

private:
    StructuredLight();
};


class GridPlanesPattern : public StructuredLight
{ 
    // these planes will be specified in "normalized device coordinates" rather than 3d world space;
    // projection of the planes into world space can be achieved via calibration data in the Projector class

    std::vector<Plane> verticalPlanes;
    std::vector<Plane> horizontalPlanes;
};


class StructuredLightGenerator
{
public:
    static void GenerateLines(cv::Mat& in)
    {
        cv::Mat vert(in.size(), CV_8UC3, cv::Scalar(0));
        cv::Mat horiz(in.size(), CV_8UC3, cv::Scalar(0));

        int minHorizontalDist = 0;
        int maxHorizontalRand = 60;
        int verticalPadding = 5;

        for (int i = 0; i < in.cols; i += verticalPadding)
        {
            cv::line(vert, cv::Point(i, 0), cv::Point(i, in.rows), cv::Scalar(255,255,255), 3);
            i += lineWidth;
        }

        int y = 0;
        while (y < in.rows)
        {
            y += minHorizontalDist + (rand() % maxHorizontalRand);
            cv::line(horiz, cv::Point(0, y), cv::Point(in.cols, y), cv::Scalar(255, 255, 255), 3);
            y += lineWidth;
        }

        in = horiz + vert;
        cv::GaussianBlur(in, in, {5, 5}, 3, 3);
    }

    const static int lineWidth = 8;
};


#endif