#ifndef SL_ACQUISITION_H
#define SL_ACQUISITION_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "Util.h"



/**
 * Abstract class that provides an interface for reading frames from devices and files.
 * 
 * Before reading files, open() can either be explicitly called, or called implicitly from captureFrame()
 * Similarly, close() can be called after reading frames, but if it is not called it will be called on destruction
 * captureFrame(Mat) will read a single frame into the passed Mat
 */
class CameraInterface
{
    virtual void captureFrame(cv::Mat& frame) = 0;
    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool getIsOpen() = 0;

protected:
    bool isOpen;
};



class OCVCameraInterface : public CameraInterface
{
public:
    OCVCameraInterface(std::string filename) : isLiveCam(false), filename(filename) {}
    OCVCameraInterface(std::size_t camera) : isLiveCam(true), camera(camera) {}

    cv::Mat frame;

    bool isStatic = false;
    bool isPaused = false;

    void open()
    {
        if (!isOpen)
        {
            if (isLiveCam) cap.open(camera, cv::CAP_ANY);
            else cap.open(filename);
        }
    }

    void close()
    {
        cap.release();
    }

    void captureFrame(cv::Mat& frame)
    {
        cap.read(frame);
    }

    bool getIsOpen()
    {
        return cap.isOpened(); 
    }

private:
    bool isLiveCam = true;
    bool isOpen = false;
    std::size_t camera;
    std::string filename;

    cv::VideoCapture cap;
};


class FLIRCameraInterface : public CameraInterface
{
public:
    
    FLIRCameraInterface(const std::size_t index) : index(index)
    {
        system = Spinnaker::System::GetInstance();
        cameras = system->GetCameras();
        cam = cameras.GetByIndex(index);
    }
    
    ~FLIRCameraInterface()
    {
        if (isOpen) close();

        cam = nullptr;
        cameras.Clear();
        system->ReleaseInstance();
    }

    void open()
    {
        if (!isOpen)
        {
            Spinnaker::GenApi::INodeMap& nodeMapTLDevice = cam->GetTLDeviceNodeMap();

            cam->Init();

            // Retrieve GenICam nodemap
            Spinnaker::GenApi::INodeMap& nodeMap = cam->GetNodeMap();

            // Todo: looks like this double set and check is done OFTEN, if we want a super generic api we should abstract it

            // Retrieve enumeration node from nodemap
            Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");

            if (!Spinnaker::GenApi::IsAvailable(ptrAcquisitionMode) || !Spinnaker::GenApi::IsWritable(ptrAcquisitionMode))
            {
                std::cerr << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << std::endl;
                // TODO: actual exception throwing here, and more error handling (check examples)
                return;
            }
            // Retrieve entry node from enumeration node
            Spinnaker::GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");

            if (!Spinnaker::GenApi::IsAvailable(ptrAcquisitionModeContinuous) || !Spinnaker::GenApi::IsReadable(ptrAcquisitionModeContinuous))
            {
                std::cerr << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << std::endl;
                return;
            }

            // Retrieve integer value from entry node
            const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

            // Set integer value from entry node as new value of enumeration node
            ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);



            // set read mode to get the latest frame
            Spinnaker::GenApi::INodeMap& sNodeMap = cam->GetTLStreamNodeMap();
            Spinnaker::GenApi::CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
            Spinnaker::GenApi::CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();

            ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestOnly");
            ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());

            isOpen = true;
        }

        cam->BeginAcquisition();
    }


    void close()
    {
        cap->Release();
        cap = nullptr;
        cam->EndAcquisition();
    }


    void captureFrame(cv::Mat& frame)
    {
        if (!isOpen) open();
        assert((isOpen));
        cap = nullptr;

        // maybe getSerial();
        // gcstring deviceSerialNumber("");
        // CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
        // if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
        // {
        //     deviceSerialNumber = ptrStringSerial->GetValue();

        //     cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        // }


        // *** NOTES ***
        // Capturing an image houses images on the camera buffer. Trying
        // to capture an image that does not exist will hang the camera.
        cap = cam->GetNextImage(100);

        if (cap->IsIncomplete())
        {
            std::cerr << "Image incomplete: " << Spinnaker::Image::GetImageStatusDescription(cap->GetImageStatus()) << "..." << std::endl;
        }

        const std::size_t width = cap->GetWidth();
        const std::size_t height = cap->GetHeight();

        try {
            //  = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);

            // ostringstream filename;

            // filename << "Acquisition-";
            // if (!deviceSerialNumber.empty())
            // {
            //     filename << deviceSerialNumber.c_str() << "-";
            // }
            // filename << imageCnt << ".jpg";

            // convertedImage->Save(filename.str().c_str());

            // cout << "Image saved at " << filename.str() << endl;
            SpinImgToMat(cap, frame);
            
            // cap = cap->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::NEAREST_NEIGHBOR);


            // unsigned int XPadding = cap->GetXPadding();
            // unsigned int YPadding = cap->GetYPadding();
            // unsigned int rowsize = cap->GetWidth();
            // unsigned int colsize = cap->GetHeight();

            // frame = cv::Mat(colsize + YPadding, rowsize + XPadding, CV_8UC3, cap->GetData(), cap->GetStride());


            // cv::imshow("test", frame);
            // cv::waitKey(1);

            // frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(255,0,255));
        }
        catch (Spinnaker::Exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    
    bool getIsOpen()
    {
        return (index < cameras.GetSize()) && isOpen;
    }


private:
    static void SpinImgToMat(Spinnaker::ImagePtr& img, cv::Mat& mat)
    {
        img = img->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::NEAREST_NEIGHBOR);

        unsigned int XPadding = img->GetXPadding();
        unsigned int YPadding = img->GetYPadding();
        unsigned int rowsize = img->GetWidth();
        unsigned int colsize = img->GetHeight();
        
        // img->DeepCopy();


        mat = cv::Mat(colsize + YPadding, rowsize + XPadding, CV_8UC3, img->GetData(), img->GetStride());
    }

    std::size_t index;

    Spinnaker::ImagePtr cap;
    Spinnaker::CameraPtr cam;
    Spinnaker::CameraList cameras;
    Spinnaker::SystemPtr system;

    bool isOpen = false;
};

#endif