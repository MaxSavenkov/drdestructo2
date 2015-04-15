#include "stdafx.h"
#include "PhysicsProtoManager.h"
#include "XMLDeserializer.h"
#include "IRender.h"
#include "IImage.h"

bool SPhysicsProto::SetCollisionMask( const std::string & fileName )
{
	m_collisionMask = fileName;
	return true;
}

class PhysicDeserializer : public XMLNamedObjectDeserializer<SPhysicsProto, std::string>
{
	class CollisionDeserializer : public XMLDataDeserializer
	{
		class SphereDeserializer : public XMLObjectDeserializer<SSphere>
		{
			void Bind( SSphere & object )
			{
				Attrib_Value( "DX", false, object.m_dx );
				Attrib_Value( "DY", false, object.m_dy );
				Attrib_Value( "Radius", false, object.m_r );
			}

		public:
			SphereDeserializer()
				: XMLObjectDeserializer<SSphere>( "Sphere", true )
			{}
		} m_spherDes;

		class MaskDeserializer : public XMLDataDeserializer
		{
		public:
			MaskDeserializer()
				: XMLDataDeserializer( "Mask", true )
			{
			}
		} m_maskDes;

	public:
		void Bind( SPhysicsProto & object )
		{
			Attrib_Value( "Layer", false, object.m_collisionLayer );
			m_spherDes.SetReceiver( object, &SPhysicsProto::AddSphere );
			m_maskDes.Attrib_SetterValue<SPhysicsProto, std::string>( "File", false, object, &SPhysicsProto::SetCollisionMask );
		}

		CollisionDeserializer()
			: XMLDataDeserializer( "Collision", true )
		{
			SubDeserializer( m_spherDes );	
			SubDeserializer( m_maskDes );
		}
	} m_collDes;

private:
	void Bind( SPhysicsProto & object )
	{
		Attrib_Value( "MinSpeed", false, object.m_minSpeed );
		Attrib_Value( "MaxSpeed", false, object.m_maxSpeed );
		Attrib_Value( "TurnSpeed", true, object.m_turnSpeed );
		Attrib_Value( "VerticalSpeed", true, object.m_verticalSpeed );
		Attrib_Value( "Accel", true, object.m_accel );
		Attrib_Value( "FlipTimer", true, object.m_flipTimer );

		m_capsDes.Attrib_SetterValue<SPhysicsProto, int>( "ID", false, object, &SPhysicsProto::SetCaps );
		m_collDes.Bind( object );
	}

	XMLDataDeserializer m_capsDes;

public:
	PhysicDeserializer()
		: XMLNamedObjectDeserializer<SPhysicsProto, std::string>( "Physic", true, "Name")
		, m_capsDes( "Caps", true )
	{
		SubDeserializer( m_capsDes );
		SubDeserializer( m_collDes );
	}
};

class PhysicsDeserializer : public RootXMLDeserializer
{
public:
	PhysicDeserializer m_physicDes;

	PhysicsDeserializer()
		: RootXMLDeserializer( "Physics" )
	{
		SubDeserializer( m_physicDes ); 
	}
};

void PhysicsProtoManager::LoadResources()
{
	PhysicsDeserializer root;
	root.m_physicDes.SetReceiver<PhysicsProtoManager>( *this, &PhysicsProtoManager::AddResource );
	root.m_physicDes.SetGetter<PhysicsProtoManager>( *this, &PhysicsProtoManager::GetResource );
	XMLDeserializer des( root );
	des.Deserialize( "Data/Protos/physics.xml" );	
}

PhysicsProtoManager & ModifyPhysics()
{
	static PhysicsProtoManager m;
	return m;
}

const PhysicsProtoManager & GetPhysics()
{
	return ModifyPhysics();
}


void LoadPhysics()
{
	ModifyPhysics().LoadResources();
}
