#ifndef SL_ACQUISITION_H
#define SL_ACQUISITION_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "Util.h"




class CameraInterface
{
    virtual void captureFrame(cv::Mat& frame) = 0;
    virtual bool getIsOpen() = 0;

private:
};


// class OCVCameraInterface : public CameraInterface
// {

// };


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
        cam = nullptr;
        cameras.Clear();
        system->ReleaseInstance();
    }

    // void open(), close();
    // bool isOpen;

    void captureFrame(cv::Mat& frame)
    {
        Spinnaker::GenApi::INodeMap& nodeMapTLDevice = cam->GetTLDeviceNodeMap();

        // Initialize camera, might want to move into a separate function for client and add isActive
        cam->Init();

        // Retrieve GenICam nodemap
        Spinnaker::GenApi::INodeMap& nodeMap = cam->GetNodeMap();

        // Retrieve enumeration node from nodemap
        Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
        if (!Spinnaker::GenApi::IsAvailable(ptrAcquisitionMode) || !Spinnaker::GenApi::IsWritable(ptrAcquisitionMode))
        {
            // cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << endl << endl;
            // return -1;
        }

        // Retrieve entry node from enumeration node
        Spinnaker::GenApi::CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");

        if (!Spinnaker::GenApi::IsAvailable(ptrAcquisitionModeContinuous) || !Spinnaker::GenApi::IsReadable(ptrAcquisitionModeContinuous))
        {
            // cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << endl << endl;
            // return -1;
        }

        // Retrieve integer value from entry node
        const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

        // Set integer value from entry node as new value of enumeration node
        ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);



        cam->BeginAcquisition();


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
        Spinnaker::ImagePtr cap = cam->GetNextImage(1000);

        if (cap->IsIncomplete())
        {
            std::cerr << "Image incomplete: " << Spinnaker::Image::GetImageStatusDescription(cap->GetImageStatus()) << "..." << std::endl;
        }

        const std::size_t width = cap->GetWidth();

        const std::size_t height = cap->GetHeight();

        try {

            //
            // Convert image to mono 8
            //
            // *** NOTES ***
            // Images can be converted between pixel formats by using
            // the appropriate enumeration value. Unlike the original
            // image, the converted one does not need to be released as
            // it does not affect the camera buffer.
            //
            // When converting images, color processing algorithm is an
            // optional parameter.
            //
            // ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);


            // ostringstream filename;

            // filename << "Acquisition-";
            // if (!deviceSerialNumber.empty())
            // {
            //     filename << deviceSerialNumber.c_str() << "-";
            // }
            // filename << imageCnt << ".jpg";

            // convertedImage->Save(filename.str().c_str());

            // cout << "Image saved at " << filename.str() << endl;
            int result = 0;
            Spinnaker::ImagePtr convertedImage = cap->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::NEAREST_NEIGHBOR);

            unsigned int XPadding = convertedImage->GetXPadding();
            unsigned int YPadding = convertedImage->GetYPadding();
            unsigned int rowsize = convertedImage->GetWidth();
            unsigned int colsize = convertedImage->GetHeight();

            //image data contains padding. When allocating Mat container size, you need to account for the X,Y image data padding. 
            cv::Mat cvimg = cv::Mat(colsize + YPadding, rowsize + XPadding, CV_8UC3, convertedImage->GetData(), convertedImage->GetStride());
            cv::imshow("current Image", cvimg);
            cv::waitKey(1);
            
            cap->Release();
        }
        catch (Spinnaker::Exception& e)
        {
            std::cout << "Error: " << e.what() << std::endl;
        }

        cam->EndAcquisition();
    }

    
    bool getIsOpen()
    {
        // check whether requested camera is within enumerated cameras
        return (index < cameras.GetSize());
    }


private:
    std::size_t index;
    Spinnaker::CameraPtr cam;
    Spinnaker::CameraList cameras;
    Spinnaker::SystemPtr system;
};

#endif