#ifndef SL_LINEPROC_H
#define SL_LINEPROC_H

#include <map>
#include <numeric>

#include "Util.h"


class LightLineProcessor
{
public:
    LightLineProcessor() {}

    void addFrame(const cv::Mat& frame_)
    {
        assert((!frame_.empty()));
        frame = frame_;
        proc = frame_;
    }


    /**
     * perform complete processing for `frame` - includes line isolation, connection, and center finding
     */
    void process(DIRECTION d, int strength, int minsize, int padding)
    {
        LightLineProcessor::isolateLineDirection(frame, proc, d, strength);
        // LightLineProcessor::connectLines(proc, proc, d, minsize);
        // LightLineProcessor::visualizeComponents(proc, proc);
        // LightLineProcessor::findCenters(frame, proc, proc, d, padding);
        return;
    }


    static void isolateLineDirection(const cv::Mat& f_in, cv::Mat& f_out, DIRECTION d, int strength)
    {
        // Construct a size object that represents the direction (OF THE LINES, not the blur) and amount of blurring to be applied
        cv::Size blurSize = (d == DIRECTION::VERTICAL) ? cv::Size(1, strength) : cv::Size(strength, 1);
        cv::Mat blur, laplacian;
        
        // Apply directional gaussian blur to filter the lines and use the Laplacian transformation to detect contrasting edges
        cv::cvtColor(f_in, blur, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(blur, blur, blurSize, 0);
        cv::Laplacian(blur, laplacian, CV_16S, 3);

        // Convert and scale the output of the laplacian transformation to a greyscale image  
        laplacian.convertTo(f_out, CV_8UC(laplacian.channels()), 0.5, 0.5 * 256);
        
        // Apply adaptive thresholding to isolate the directional lines
        cv::threshold(f_out, f_out, 140, 255, cv::THRESH_BINARY);
        // cv::adaptiveThreshold(f_out, f_out, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 25, 25);
    }


    static void connectLines(const cv::Mat& f_in, cv::Mat& f_out, DIRECTION d, int minLength)
    {
        int minWidth = 3;
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);
        
        // determine the stats that represent the primary and secondary dimensions of the lines at hand (length and width)
        cv::ConnectedComponentsTypes primaryDim = (d == DIRECTION::VERTICAL) ? cv::CC_STAT_HEIGHT : cv::CC_STAT_WIDTH;
        cv::ConnectedComponentsTypes secondaryDim = (d == DIRECTION::VERTICAL) ? cv::CC_STAT_WIDTH : cv::CC_STAT_HEIGHT;

        // clear the output with black
        f_out = cv::Mat(f_out.size(), CV_8U, cv::Scalar(0));

        for (int i = 1; i < ncomponents; i++) {
            cv::Mat interm;
            
            // find the dimensions of the current component
            int length = stats.at<int>(i, primaryDim);
            int width = stats.at<int>(i, secondaryDim);

            if (length >= minLength && width > minWidth)
            {
                // if the current component is large enough, add it to the `interm` mask
                cv::compare(labels, i, interm, cv::CMP_EQ);

                // add the interm mask to the output
                cv::bitwise_or(interm, f_out, f_out);
            }
        }

        // // quick visualization trick
        // labels.convertTo(f_out, CV_8UC(labels.channels()), 255.0 / (double)ncomponents);
    };


    static void visualizeComponents(const cv::Mat& f_in, cv::Mat& f_out)
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);     
        f_out = cv::Mat(f_out.size(), CV_8U, cv::Scalar(0));

        for (int i = 1; i < ncomponents; i++)
        {
            cv::Mat mask;

            // create a single random-colored image 
            cv::Mat fullColor(f_out.size(), CV_8UC3, cv::Scalar(RandInt(), RandInt(), RandInt()));
        
            // set the mask to the current component
            cv::compare(labels, i, mask, cv::CMP_EQ);

            // copy the colored image to the output, using the previously created mask
            fullColor.copyTo(f_out, mask);
        }
    }


    static void extendLines(const cv::Mat& f_in, cv::Mat& f_out)
    {
        return;
    }


    static void findCenters(const cv::Mat& frame, const cv::Mat& f_in, cv::Mat& f_out, DIRECTION d, int padding)
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);

        // cv::Mat disp;
        f_out = cv::Mat(frame.size(), CV_8UC3, cv::Scalar(0, 0, 0));
        // f_out = frame;

        for (int i = 1; i < ncomponents; i++)
        {
            int lineWidth, lineHeight;

            if (d == DIRECTION::VERTICAL)
            {
                lineHeight = stats.at<int>(i, cv::CC_STAT_HEIGHT);
                lineWidth = stats.at<int>(i, cv::CC_STAT_WIDTH);
            }
            else
            {
                lineHeight = stats.at<int>(i, cv::CC_STAT_WIDTH);
                lineWidth = stats.at<int>(i, cv::CC_STAT_HEIGHT);
            }

            cv::Point topLeft(stats.at<int>(i, cv::CC_STAT_LEFT), stats.at<int>(i, cv::CC_STAT_TOP));

            for (int j = 0; j <= lineHeight; j++)
            {

                cv::Point p1, p2;
                p1 = topLeft;

                if (d == DIRECTION::VERTICAL) p1.y += j;
                else p1.x += j;

                p2 = p1;
                if (d == DIRECTION::VERTICAL) p2.x += lineWidth;
                else p2.y += lineWidth;

                cv::LineIterator inIt(labels, p1, p2, 8);
                cv::Point p1Cut, p2Cut;
                int pLength = 0;
                cv::Point pOffset = p1;
                bool hasLineStarted = false;

                for (int x = 0; x < inIt.count; x++)
                {
                    int checkval = labels.at<int>(pOffset);

                    if (checkval == i && !hasLineStarted)
                    {
                        hasLineStarted = true;
                        p1Cut = pOffset;
                        p2Cut = pOffset;
                    }
                    else if (checkval == i && hasLineStarted)
                    {
                        pLength++;
                        p2Cut = pOffset;
                    }
                    else if (checkval != i && hasLineStarted)
                    {
                        break;
                    }

                    if (d == DIRECTION::VERTICAL) pOffset.x++;
                    else pOffset.y++;
                }

                if (d == DIRECTION::VERTICAL)
                {
                    p1Cut.x -= padding;
                    p2Cut.x += padding;
                }
                else
                {
                    p1Cut.y -= padding;
                    p2Cut.y += padding;
                }

                // cv::line(f_out, p1Cut, p2Cut, cv::Scalar(0, 0, 255), 1);
                // cv::line(f_out, p1Orig, p2Orig, cv::Scalar(255, 0, 0), 1);

                std::vector<double> points;
                cv::LineIterator frameIt(frame, p1Cut, p2Cut, 8);
                for (int x = 0; x < frameIt.count; x++, ++frameIt)
                {
                    points.push_back(static_cast<double>((int)*(*frameIt)));
                }

                std::vector<double> x(points.size());
                std::iota(x.begin(), x.end(), 0);

                PolynomialRegression<double> polyreg;
                std::vector<double> coeffs;
                polyreg.fitIt(x, points, 2, coeffs);

                // find the x-coordinate of the vertex of the quadratic regression
                double quadVertex = (-1.0 * coeffs[1]) / (2.0 * coeffs[2]);

                // mean represents the linear center point of the isolated line (with no regression)
                double mean = padding + pLength / 2.0;

                // centerPoint will eventually hold the subpixel position of the strip
                cv::Point centerPoint = p1Cut;

                double delta = 2.0;
                double toAdd;


                // if vertexPos is more than delta away from the mean center, use the mean

                if (std::isnan(quadVertex) || abs(mean - quadVertex) > delta)
                    toAdd = mean;
                else
                    toAdd = quadVertex;
                
                if (d == DIRECTION::VERTICAL) centerPoint.x += round(toAdd);
                else centerPoint.y += round(toAdd);

                
                if (centerPoint.x >= 0 && centerPoint.y >= 0)
                    f_out.at<cv::Vec3b>(centerPoint) = cv::Vec3b(255, 255, 255);
                
                // cv::imshow("Win", f_out);
                // cv::waitKey(1);
            }
        }

        // f_out = disp;
    }


    inline cv::Mat& getProc() {return proc;}
    inline cv::Mat& getFrame() {return frame;}

private:
    cv::Mat frame;  // The unchanged input frame
    cv::Mat proc;  // The result of the last processing step

    std::map<DIRECTION, std::vector<cv::Mat>> filteredOrderedLines;
};


#endif