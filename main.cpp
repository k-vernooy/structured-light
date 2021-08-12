#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <chrono>
#include <map>


// class KeyInputHandler
// {
// public:
//     // implemented as a singleton
//     static KeyInputHandler* Get()
//     {
//         static KeyInputHandler k;
//         return &k;
//     }

//     enum class KEY
//     {
//         SPACE = 32,
//         UP = 82, 
//         DOWN = 84,
//         NOKEY = -1
//     };
// };

inline int RandInt()
{
    return (rand() % 256);
}

class Pinhole
{
    // represent 3d calibration matrices of a pinhole obj
};

class Projector : public Pinhole
{

};

class Camera : public Pinhole
{
    
};


enum class DIRECTION {VERTICAL, HORIZONTAL};


/**
 * A class that manages the initial image processing required to isolate and clean
 * the projected vertical and horizontal stripes. Performs
 */
class LightLineProcessor
{
public:
    static LightLineProcessor* Get()
    {
        static LightLineProcessor instance;
        return &instance;
    }

    void addFrame(const cv::Mat& frame_)
    {
        assert((!frame_.empty()));
        frame = frame_;
        proc = frame_;
    }

    void process(DIRECTION d, int strength, int minsize)
    {
        LightLineProcessor::isolateLineDirection(frame, proc, d, strength);
        LightLineProcessor::connectLines(proc, proc, d, minsize);
        // LightLineProcessor::visualizeComponents(proc, proc);
        LightLineProcessor::findCenters(frame, proc, proc, d);
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
        // cv::threshold(proc, proc, 150, 255, cv::THRESH_BINARY);
        cv::adaptiveThreshold(f_out, f_out, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 15, 15);
    }

    static void connectLines(const cv::Mat& f_in, cv::Mat& f_out, DIRECTION d, int minsize)
    {
        int minOpStat = 3;
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);
        cv::ConnectedComponentsTypes mainstat = (d == DIRECTION::VERTICAL) ? cv::CC_STAT_HEIGHT : cv::CC_STAT_WIDTH;
        cv::ConnectedComponentsTypes opstat = (d == DIRECTION::VERTICAL) ? cv::CC_STAT_WIDTH : cv::CC_STAT_HEIGHT;

        f_out = cv::Mat(f_out.size(), CV_8U, cv::Scalar(0));

        for (int i = 1; i < ncomponents; i++) {
            cv::Mat interm;
            int size = stats.at<int>(i, mainstat);
            int opsize = stats.at<int>(i, opstat);

            if (size >= minsize && opsize > minOpStat)
            {
                // totAdded++;
                cv::compare(labels, i, interm, cv::CMP_EQ);
                cv::bitwise_or(interm, f_out, f_out);
            }
        }

        // labels.convertTo(f_out, CV_8UC(labels.channels()), 255.0 / (double)ncomponents);
    };


    static void visualizeComponents(const cv::Mat& f_in, cv::Mat& f_out)
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);     
        f_out = cv::Mat(f_out.size(), CV_8U, cv::Scalar(0));

        for (int i = 1; i < ncomponents; i++) {
            cv::Mat mask;
            cv::Mat fullColor(f_out.size(), CV_8UC3, cv::Scalar(RandInt(), RandInt(), RandInt()));
            cv::compare(labels, i, mask, cv::CMP_EQ);
            fullColor.copyTo(f_out, mask);
        }
    }


    static void extendLines(const cv::Mat& f_in, cv::Mat& f_out)
    {
        return;
    }


    static void findCenters(const cv::Mat& frame, const cv::Mat& f_in, cv::Mat& f_out, DIRECTION d)
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(f_in, labels, stats, centroids, 8);


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
            cv::Point topRight(topLeft.x + lineWidth, topLeft.y);
            cv::Point bottomLeft(topLeft.x, topLeft.y + lineHeight);
            cv::Point bottomRight(topLeft.x + lineWidth, topLeft.y + lineHeight);

            for (int j = 0; j <= lineHeight; j++)
            {
                cv::Mat disp;
                frame.copyTo(disp);

                cv::line(disp, topLeft, topRight, cv::Scalar(255, 0, 0), 2);
                cv::line(disp, topRight, bottomRight, cv::Scalar(255, 0, 0), 2);
                cv::line(disp, bottomRight, bottomLeft, cv::Scalar(255, 0, 0), 2);
                cv::line(disp, bottomLeft, topLeft, cv::Scalar(255, 0, 0), 2);
                
                cv::Point p1, p2;
                p1 = topLeft;

                if (d == DIRECTION::VERTICAL) p1.y += j;
                else p1.x += j;

                p2 = p1;
                if (d == DIRECTION::VERTICAL) p2.x += lineWidth;
                else p2.y += lineWidth;

                cv::LineIterator frameIt(frame, p1, p2, 8);
                cv::LineIterator inIt(labels, p1, p2, 8);

                cv::Point pointsOffset = p1;
                std::vector<int> points;

                bool hasLineStarted = false;
                for (int x = 0; x < frameIt.count; x++, ++frameIt, ++ inIt)
                {
                    int checkval = (int)*(*inIt);

                    if (checkval == i) hasLineStarted = true;
                    else if (hasLineStarted == true) break;

                    if (hasLineStarted) 
                    {
                        points.push_back((int)*(*frameIt));
                        disp.at<cv::Vec3b>(pointsOffset) = cv::Vec3b(0, 0, 255);   
                    }
                    // else
                    // {
                    // }
                    if (d == DIRECTION::VERTICAL) pointsOffset.x++;
                    else pointsOffset.y++;
                }

                // cv::line(disp, p1, p2, cv::Scalar(0, 0, 255), 2);
                cv::imshow("Win", disp);
                cv::waitKey(30);
            }
        }
    }


    inline cv::Mat& getProc() {return proc;}
    inline cv::Mat& getFrame() {return frame;}

private:
    cv::Mat frame;  // The unchanged input frame
    cv::Mat proc;  // The result of the last processing step

    std::map<DIRECTION, std::vector<cv::Mat>> filteredOrderedLines;
};




int main(int argc, char** argv)
{
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
    LightLineProcessor* lp = LightLineProcessor::Get();
    lp->addFrame(frame);

    // parameters for the realtime vis
    int optype = 0;
    int strength = 31;
    int minsize = 15;
    std::vector<int> opkeys = {-1, 32, 82, 84};


    while (true)
    {
        // capture a new frame if we're processing a live feed
        if (!isStatic) cap.read(frame);
        lp->addFrame(frame);

        char key = (char) cv::waitKey(30);
        
        if (key == 32)
        {
            if (optype == 2) optype = 0;
            else optype++;
        }
        else if (key == 82) 
        {
            strength += 2;
            minsize += 2;
        }
        else if (key == 84)
        {
            if (strength != 1) strength -= 2;
            if (minsize != 0) minsize -= 2;
        }


        // if (optype == 1)
        // {
        //     LightLineProcessor::isolateLineDirection(lp->getFrame(), lp->getProc(), DIRECTION::VERTICAL, strength);
        //     LightLineProcessor::connectLines(lp->getProc(), lp->getProc(), DIRECTION::VERTICAL, minsize);
        //     LightLineProcessor::visualizeComponents(lp->getProc(), lp->getProc());
        // }
        // else if (optype == 2)
        // {
            // LightLineProcessor::isolateLineDirection(lp->getFrame(), lp->getProc(), DIRECTION::HORIZONTAL, strength);
            // LightLineProcessor::connectLines(lp->getProc(), lp->getProc(), DIRECTION::HORIZONTAL, minsize);
            // LightLineProcessor::visualizeComponents(lp->getProc(), lp->getProc());
        // }

        // for (int dir : {0, 1})

        if (optype != 0)
            lp->process(static_cast<DIRECTION>(optype - 1), strength, minsize);

        cv::imshow("Live", lp->getProc());
    
        // if (std::find(opkeys.begin(), opkeys.end(), key) == opkeys.end()) break;
    }

    // cv::cvtColor(frame, frame, COLOR_BGR2GRAY);
    // Mat kernel = (Mat_<int>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
    // cv::filter2D(frame, frame, CV_64F, kernel);

    return 0;
}