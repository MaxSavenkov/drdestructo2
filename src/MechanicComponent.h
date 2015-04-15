#pragma once
#include "BaseComponent.h"

/*
	IDs of special points on object's sprite
*/
enum EObjPoint
{
	OBJPOINT_GUN,
	OBJPOINT_BOMB,
};

/*
	Type of ammunition for a weapon
*/
enum EAmmoType
{
	AMMO_FINITE,
	AMMO_INFINITE,
	AMMO_ONE_PER_LEVEL,
};

// Maximum number of weapons
static const int MAX_WEAPONS = 10;

// How long player's object remains invulnerable after spawn
const float INVUL_TIMER = 2.0f;

// Used for swapping ammo for celebrations :)
struct SWeaponDate
{
	int m_month;
	int m_day;
};

/*
	Description and current parameters of a weapon
*/
struct SWeapon
{
	SWeapon()
		: m_cooldown( 0.5f )
		, m_currentCooldown( 0.0f )
		, m_angleLimit( 180 )
		, m_heightLimit( 0 )
		, m_point( OBJPOINT_GUN )
		, m_ammoType( AMMO_INFINITE )
		, m_ammoMax( 0 )
		, m_currentAmmo( 0 )
	{}

	  // Object of a bullet
	std::string m_object;
	  // AI of a bullet
	std::string m_ai;
	  // Cooldown between salvos
	float m_cooldown;
	  // Current cooldown value. Could be ignored!
	float m_currentCooldown;
	  // Weapon will not work if angle between plane and horizont is greate that this (for bombs)
	float m_angleLimit;
	  // Weapon will not work if plane height is less than this (for bombs)
	int m_heightLimit;
	  // Point at which bullet is spawned
	EObjPoint m_point;
	  // How much ammo is in this weapon (finite/infinite)
	EAmmoType m_ammoType;
	  // Number of ammo
	int m_ammoMax;
	  // Current ammo count
	int m_currentAmmo;

	SWeaponDate m_date;
};

/*
	Component that holds game-mechanic related information and state of object.
	It ended up being kind of a dump for all the properties which didn't fit
	anywhere else.
*/
class MechanicComponent : public SubComponent<MechanicComponent, GameObject>
{
public:
	MechanicComponent()
	{}

	MechanicComponent( const MechanicCompID & id )
		: SubComponent<MechanicComponent, GameObject>( id )
		, m_type( 0 )
		, m_owner( 0 )
		, m_stalled( false )
		, m_stallTimer( 0 )
		, m_score( 0 )
		, m_statType( -1 )
	{}

	  // Object's weapons
	SWeapon m_weapons[ MAX_WEAPONS ];
	  // Type (mostly for collision detection, but could be used for any purpose :) )
	int m_type;
	  // For bullets. Could be Player or AI. If we add more than one player, this could be
	  // used to distinguish their bullets to support correct kills/score count
	int m_owner;
	  // Object that will replace this object after its death
	std::string m_deathObject;
	  // AI for death object
	std::string m_deathAI;
	  // Amount of score for destroying this object
	int m_score;
	  // Object's type for statistic purposes
	int m_statType;

	  // True if this object is stalled (only used for player's object)
	bool m_stalled;
	  // Stall timer: how: while it's > 0 player can't control his plane
	float m_stallTimer;

	  // Timer of invulenarability after respawn
	float m_invulTimer;

	void StartStall()
	{
		m_stalled = true;
		m_stallTimer = 1.0f;
	}
	void UpdateStall( float dt, bool hasControls )
	{
		if ( !m_stalled )
			return;

		if ( m_stallTimer > 0 )
		{
			m_stallTimer -= dt;
			if ( m_stallTimer < 0 )
			{
				m_stallTimer = 0;
			}
		}
		else
		{
			if ( hasControls )
				m_stalled = false;
		}
	}
	bool IsStalledPhase1() const { return m_stalled && m_stallTimer > 0.0f; }
	bool IsStalledPhase2() const { return m_stalled && m_stallTimer <= 0.0f; }
};
