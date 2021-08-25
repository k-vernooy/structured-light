#ifndef SL_PINHOLE_H
#define SL_PINHOLE_H

#include "StructuredLight.h"


class Pinhole
{
    // represent matrices of a pinhole obj
};

class Projector : public Pinhole
{
public:
    StructuredLight projection;
};

class Camera : public Pinhole
{
    
};

#endif