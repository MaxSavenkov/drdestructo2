#pragma once
#include "PrototypeManager.h"
#include "MechanicComponent.h"

struct SWeaponProto
{
	SWeaponProto()
		: m_point( OBJPOINT_GUN )
		, m_cooldown( 0.5f )
		, m_ammoType( AMMO_INFINITE )
		, m_ammo( 0 )
		, m_angleLimit( 180 )
		, m_heightLimit( 0 )
	{}

	EObjPoint m_point;
	float m_cooldown;
	std::string m_object;
	std::string m_ai;
	EAmmoType m_ammoType;
	int m_ammo;
	float m_angleLimit;
	int m_heightLimit;
	
	SWeaponDate m_date;

	bool SetAmmo( const std::string & ammo )
	{
		if ( ammo == "INFINITE" )
		{
			m_ammoType = AMMO_INFINITE;			
		}
		else if ( ammo == "ONE_PER_LEVEL" )
		{
			m_ammoType = AMMO_ONE_PER_LEVEL;
			m_ammo = 1;
		}
		else
		{
			m_ammoType = AMMO_FINITE;
			m_ammo = atoi( ammo.c_str() );
		}

		return true;
	}

	bool SetDate( const std::string & dateStr )
	{
		sscanf( dateStr.c_str(), "%i.%i", &m_date.m_day, &m_date.m_month );
		return true;	
	}
};

struct SMechanicsProto
{
	SMechanicsProto()
		: m_type( 0 )
		, m_owner( 0 )
		, m_score( 0 )
		, m_statType( -1 )
	{}

	int m_type;
	std::string m_deathObject;
	std::string m_deathAI;
	int m_owner;
	int m_score;
	int m_statType;

	SWeaponProto m_weapons[ MAX_WEAPONS ];

	void AddWeapon( const int & id, const SWeaponProto & wpn )
	{
		if ( id < 0 || id >= MAX_WEAPONS )
			return;

		m_weapons[ id ] = wpn;
	}

	const SWeaponProto *GetWeapon( const int & id ) const
	{
		return ( id < 0 || id >= MAX_WEAPONS ) ? 0 : &m_weapons[ id ];
	}
};

class MechanicsProtoManager : public PrototypeManager<SMechanicsProto>
{
public:
	void LoadResources();
};

const MechanicsProtoManager & GetMechanics();
void LoadMechanics();
