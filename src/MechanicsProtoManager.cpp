#include "stdafx.h"
#include "MechanicsProtoManager.h"
#include "XMLDeserializer.h"

EObjPoint ParseObjPoint( const std::string & name )
{
	if ( name == "GunPoint" )
		return OBJPOINT_GUN;
	if ( name == "BombPoint" )
		return OBJPOINT_BOMB;

	return OBJPOINT_GUN;
}

class MechanicDeserializer : public XMLNamedObjectDeserializer<SMechanicsProto, std::string>
{
	class WeaponDeserializer : public XMLNamedObjectDeserializer<SWeaponProto, int>
	{
		void Bind( SWeaponProto & object )
		{
			Attrib_Value( "Object", false, object.m_object );
			Attrib_Value( "AI", true, object.m_ai );
			Attrib_Value( "Cooldown", false, object.m_cooldown );
			Attrib_Value( "AngleLimit", true, object.m_angleLimit );
			Attrib_Value( "HeightLimit", true, object.m_heightLimit );
			Attrib_ConvertedValue( "Point", false, &ParseObjPoint, object.m_point );
			Attrib_SetterValue<SWeaponProto, std::string>( "Ammo", false, object, &SWeaponProto::SetAmmo );
			Attrib_SetterValue<SWeaponProto, std::string>( "Date", true, object, &SWeaponProto::SetDate );
		}

	public:
		WeaponDeserializer()
			: XMLNamedObjectDeserializer<SWeaponProto, int>( "Weapon", true, "ID" )
		{}

	} m_weaponDes;
private:
	void Bind( SMechanicsProto & object )
	{
		Attrib_Value( "Type", false, object.m_type );
		Attrib_Value( "DeathObject", true, object.m_deathObject );
		Attrib_Value( "DeathAI", true, object.m_deathAI );
		Attrib_Value( "Owner", true, object.m_owner );
		Attrib_Value( "Score", true, object.m_score );
		Attrib_Value( "StatType", true, object.m_statType );

		m_weaponDes.SetReceiver( object, &SMechanicsProto::AddWeapon );
		m_weaponDes.SetGetter( object, &SMechanicsProto::GetWeapon );
	}

public:
	MechanicDeserializer()
		: XMLNamedObjectDeserializer<SMechanicsProto, std::string>( "Mechanic", true, "Name")
	{
		SubDeserializer( m_weaponDes );
	}
};

class MechanicsDeserializer : public RootXMLDeserializer
{
public:
	MechanicDeserializer m_mechDes;

	MechanicsDeserializer()
		: RootXMLDeserializer( "Mechanics" )
	{
		SubDeserializer( m_mechDes ); 
	}
};

void MechanicsProtoManager::LoadResources()
{
	MechanicsDeserializer root;
	root.m_mechDes.SetReceiver<MechanicsProtoManager>( *this, &MechanicsProtoManager::AddResource );
	root.m_mechDes.SetGetter<MechanicsProtoManager>( *this, &MechanicsProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/gm.xml" );	
}

MechanicsProtoManager & ModifyMechanics()
{
	static MechanicsProtoManager m;
	return m;
}

const MechanicsProtoManager & GetMechanics()
{
	return ModifyMechanics();
}


void LoadMechanics()
{
	ModifyMechanics().LoadResources();
}
