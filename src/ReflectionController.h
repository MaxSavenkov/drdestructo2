#pragma once
#include "IRender.h"

struct SReflectionControllerParams
{
	SReflectionControllerParams()
		: m_active( false )
		, m_reflectionPercent( 0.25f )
		, m_beforeAfterY( 0 )
	{}

	bool m_active;
	float m_reflectionPercent;
	int m_beforeAfterY;
};

class GraphicComponent;

/*
	This class draws reflection of sky in "water"
*/
class ReflectionController
{
	bool m_active;
	float m_reflectionPercent;
	int m_beforeAfterY;

	ImageHandle m_image;

public:
	ReflectionController()
		: m_active( false )
		, m_reflectionPercent( 0.25f )
		, m_beforeAfterY( 0 )
	{}

	void Reset() { m_active = false; }
	void Setup( const SReflectionControllerParams & params );
	void Update( IRender & render, ImageHandle bgImg );
	void RenderBefore( IRender & render, GraphicComponent & comp );
	void RenderAfter( IRender & render, GraphicComponent & comp );
};
