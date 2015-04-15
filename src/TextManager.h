#pragma once
#include "ResourceManager.h"
#include "ResourceHandle.h"
#include "Engine/IUTFString.h"

class Text
{
public:
	UTFString m_text;
};

typedef Handle<Text> TextHandle;

struct Empty{};

class TextManager : public ResourceManager< Text*, Empty, 0, TextHandle >
{
public:
	Text *Create( const Empty & params ){ return 0; }
	Text *Load( const std::string & path );
	void Delete( Text *res );
	//void ReloadAllResources();
};


TextManager & GetTextManager();
