#pragma once

#include "ResourceHandle.h"

class IImage;

typedef IImage* IImagePtr;
typedef Handle<IImagePtr> ImageHandle;

class IImageManager
{
public:
	virtual IImagePtr GetImageByHandle( ImageHandle handle ) = 0;
};
