#pragma once
#include "ResourceManager.h"
#include "ResourceHandle.h"
#include "IImage.h"

typedef Handle<IImage*> ImageHandle;

struct BitmapParams
{
	int w, h;
	bool noPreserve;
	std::string context;
	bool memory;
};

class BitmapManager : public ResourceManager< IImage*, BitmapParams, 0, ImageHandle >
{
	std::map<intptr_t, std::string> m_contexts;
public:
	IImage *Load( const std::string & path );
	IImage *Create( const BitmapParams & params );
	void Delete( IImage *res );
	void ReloadAllResources();
	  // Debug function
	//int CountModified();
};

BitmapManager & GetBitmapManager();
