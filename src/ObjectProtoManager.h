#pragma once
#include "PrototypeManager.h"

struct SGameObjectProto
{
	std::string m_graphics;
	std::string m_physics;
	std::string m_controls;
	std::string m_mechanics;
	std::string m_sounds;
};

class GameObjectProtoManager : public PrototypeManager<SGameObjectProto>
{
public:
	void LoadResources();
};

const GameObjectProtoManager & GetGameObjects();
void LoadGameObjects();
