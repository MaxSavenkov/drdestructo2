#pragma once

class IRender;
class GraphicComponent;

  // Processes Graphic components
class GraphicsProcessor
{
public:
	void Update( float dt, GraphicComponent & comp );
	void Render( IRender & render, GraphicComponent & comp );
};
