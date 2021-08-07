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
        static LightLineProcessor proc;
        return &proc;
    }

    void addFrame(const cv::Mat& frame_)
    {
        assert((!frame_.empty()));
        frame = frame_;
        endpoint = frame_;
    }

    void process() {}

    inline void draw()
    {
        assert((!endpoint.empty()));
        cv::imshow("Live", endpoint);    
    }

    void isolateLineDirection(DIRECTION d, int strength)
    {
        // Construct a size object that represents the direction (OF THE LINES, not the blur) and amount of blurring to be applied
        cv::Size blurSize = (d == DIRECTION::VERTICAL) ? cv::Size(1, strength) : cv::Size(strength, 1);
        cv::Mat blur, laplacian;
        
        // Apply directional gaussian blur to filter the lines and use the Laplacian transformation to detect contrasting edges
        cv::cvtColor(frame, blur, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(blur, blur, blurSize, 0);
        cv::Laplacian(blur, laplacian, CV_16S, 3);

        // Convert and scale the output of the laplacian transformation to a greyscale image  
        laplacian.convertTo(endpoint, CV_8UC(laplacian.channels()), 0.5, 0.5 * 256);
        
        // Apply adaptive thresholding to isolate the directional lines
        // cv::threshold(endpoint, endpoint, 150, 255, cv::THRESH_BINARY);
        cv::adaptiveThreshold(endpoint, endpoint, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 15, 15);
    }

    void connectLines(DIRECTION d, int minsize)
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(endpoint, labels, stats, centroids, 8);
        
        // connectedComponentswithStats yields every seperated component with information on each of them, such as size
        // the following part is just taking out the background which is also considered a component, but most of the time we don't want that.
        // ;//[1:, -1]; nb_components = nb_components - 1

        // minimum size of particles we want to keep (number of pixels)
        // here, it's a fixed value, but you can set it as you want, eg the mean of the sizes or whatever

        cv::ConnectedComponentsTypes stat = (d == DIRECTION::VERTICAL) ? cv::CC_STAT_HEIGHT : cv::CC_STAT_WIDTH;

        // #2
        endpoint = cv::Mat(endpoint.size(), CV_8U, cv::Scalar(0));

        for (int i = 1; i < ncomponents; i++) {
            cv::Mat interm;
            int size = stats.at<int>(i, stat);

            if (size >= minsize)
            {
                // totAdded++;
                cv::compare(labels, i, interm, cv::CMP_EQ);
                cv::bitwise_or(interm, endpoint, endpoint);
            }
        }
        // labels.convertTo(endpoint, CV_8UC(labels.channels()), 255.0 / (double)ncomponents);
    };

    void componentToRGB()
    {
        cv::Mat labels, stats, centroids;
        int ncomponents = cv::connectedComponentsWithStats(endpoint, labels, stats, centroids, 8);        
    }

private:
    cv::Mat frame;  // The unchanged input frame
    cv::Mat endpoint;  // The result of the last processing step

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
    LightLineProcessor* proc = LightLineProcessor::Get();
    proc->addFrame(frame);

    // parameters for the realtime vis
    int optype = 0;
    int strength = 31;
    int minsize = 30;
    std::vector<int> opkeys = {-1, 32, 82, 84};


    while (true)
    {
        // capture a new frame if we're processing a live feed
        if (!isStatic) cap.read(frame);
        proc->addFrame(frame);

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


        if (optype == 1)
        {
            proc->isolateLineDirection(DIRECTION::VERTICAL, strength);
            proc->connectLines(DIRECTION::VERTICAL, minsize);
        }
        else if (optype == 2)
        {
            proc->isolateLineDirection(DIRECTION::HORIZONTAL, strength);
            proc->connectLines(DIRECTION::HORIZONTAL, minsize);
        }

        proc->draw();
    
        if (std::find(opkeys.begin(), opkeys.end(), key) == opkeys.end()) break;
    }

    // cv::cvtColor(frame, frame, COLOR_BGR2GRAY);
    // Mat kernel = (Mat_<int>(3, 3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
    // cv::filter2D(frame, frame, CV_64F, kernel);

    return 0;
}