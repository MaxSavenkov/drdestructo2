#pragma once

/*
	Very basic Image interface for Engine's user.
	All significant operations are to be performed
	through IRender interface, so there is nothing
	important here (it's not conveinient, it's not
	right, but it's the way it's made)
*/
class IImage
{
public:
	virtual ~IImage(){}
	virtual int GetWidth() const = 0;	
	virtual int GetHeight() const = 0;	
};
