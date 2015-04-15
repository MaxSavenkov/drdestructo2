#pragma once
#include "FastDelegate/FastDelegate.h"
#include "ILogger.h"
#include "Vars.h"
#include "Engine/ISystem.h"


//----------------------------------------------------------------

/*
	This is probably the most complex piece of code in
	the whole project. It is, also, may be somewhat
	unnecessary.

	The idea was to abstract away XML parsing and create
	a set of classes which would make creating objects
	from XML descriptions easy.

	Along the way, two important features were added:
	support for global variables and inheritance. This
	may make it worthwhile for this project, but I
	suspect that this code is of no use to anyone else.

	Still, here's what I was trying to do: encode a scheme
	of XML file in C++ and bind parts of C++ object to
	elements of that scheme.

	The result is a bit cumbersome. Read on, if you're
	interested in details...
*/

/*
	This is a base class for deserializers. Deserializer is a class that
	processes a piece of XML in a certain way. Base deserializer is
	abstract, but support storing children deserializers.
*/
class BaseXMLDeserializer
{
	  // Children deserializers
	typedef std::vector< BaseXMLDeserializer* > DeserializersVector;
	DeserializersVector m_subDeserializers;

protected:
	  // Tag, for which this deserializer is used
	std::string m_tag;
	  // If true, should return true from Deserialize even if there are errors
	  // or value is not present.
	bool m_optional;

protected:
	  // Registers children deserializer
	void SubDeserializer( BaseXMLDeserializer & d )
	{
		m_subDeserializers.push_back( &d );
	}
 
	  // Tries to apply all child deserializers to passed sub-element
	bool DeserializeChildren( const TiXmlElement *pSubElement )
	{
		for ( DeserializersVector::iterator iter = m_subDeserializers.begin(); iter != m_subDeserializers.end(); ++iter )
			(*iter)->Deserialize( pSubElement );

		return true;
	}

public:
	BaseXMLDeserializer( const std::string & tag, bool optional )
		: m_tag( tag )
		, m_optional( optional )
	{}

	virtual bool Deserialize( const TiXmlElement *pElement ) = 0;
};

//----------------------------------------------------------------

/*
	This function tries to read attribute with specified name.
	If variable name with $ is specified as value, returns
	value of that variable.
*/
template<typename TValue>
bool GetAttributeValue( const std::string & name, const TiXmlElement *pElement, TValue & ref )
{
	std::string strVal;
	pElement->QueryValueAttribute<std::string>( name, &strVal );
	if ( strVal.empty() || strVal[0] != '$' )
	{
		if ( pElement->QueryValueAttribute<TValue>( name, &ref ) == TIXML_SUCCESS )
			return true;

		//GetLog().Log( 1, LM_FILE, LL_WARNING, "Can read attribute value %s", name.c_str() );
		return false;
	}
	else
	{
		GlobalVariablesStorage::SVar<TValue> *pVar = GetVars().GetVar<TValue>( strVal.substr( 1 ) );
		if ( !pVar )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Can read attribute value %s: variable %s not found", name.c_str(), strVal.c_str() );
			return false;
		}

		ref = pVar->m_value;
		return true;
	}
}

template<> inline
bool GetAttributeValue( const std::string & name, const TiXmlElement *pElement, std::string & ref )
{
	std::string strVal;
	pElement->QueryValueAttribute<std::string>( name, &strVal );
	if ( strVal.empty() || strVal[0] != '$' )
	{
		if ( pElement->QueryStringAttribute( name.c_str(), &ref ) == TIXML_SUCCESS )
			return true;
        
		//GetLog().Log( 1, LM_FILE, LL_WARNING, "Can read attribute value %s", name.c_str() );
		return false;
	}
	else
	{
		GlobalVariablesStorage::SVar<std::string> *pVar = GetVars().GetVar<std::string>( strVal.substr( 1 ) );
		if ( !pVar )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Can read attribute value %s: variable %s not found", name.c_str(), strVal.c_str() );
			return false;
		}
        
		ref = pVar->m_value;
		return true;
	}
}

//----------------------------------------------------------------

/*
	Interface for registering bindings for XML tag attributes.
*/
class IXMLAttrib
{
protected:
	std::string m_name;
	bool m_optional;

public:
	IXMLAttrib( const std::string & name, bool optional )
		: m_name( name )
		, m_optional( optional )
	{}

	virtual ~IXMLAttrib()
	{}

	virtual bool Load( const TiXmlElement *pElement ) = 0;

	bool IsOptional() const { return m_optional; }
	const char *GetName() const { return m_name.c_str(); }
};

//----------------------------------------------------------------

/*
	Simplest kind of attribute: just a reference to a field
	into which a value should be read.
*/
template<typename TValue>
class XMLSchemaValueAttrib : public IXMLAttrib
{
	TValue & m_valueRef;
public:
	XMLSchemaValueAttrib( const std::string & name, bool optional, TValue & valueRef )
		: IXMLAttrib( name, optional )
		, m_valueRef( valueRef )
	{}

	bool Load( const TiXmlElement *pElement )
	{
		return GetAttributeValue( m_name, pElement, m_valueRef );
	}
};

//----------------------------------------------------------------

/*
	Attribute which should be set via a setter function
	(useful when you want to convert units, or do some
	other operations on read data before writing it into
	field)
*/
template<typename TObject, typename TValue>
class XMLSchemaSetterValueAttrib : public IXMLAttrib
{
public:
	typedef bool (TObject::*TSetter)( const TValue & );
	TSetter m_setter;
	TObject & m_object;

public:
	XMLSchemaSetterValueAttrib( const std::string & name, bool optional, TObject & object, const TSetter & setter )
		: IXMLAttrib( name, optional )
		, m_object( object )
		, m_setter( setter )
	{}

	bool Load( const TiXmlElement *pElement )
	{
		TValue value;
		if ( !GetAttributeValue( m_name, pElement, value ) )
			return false;

		return (m_object.*m_setter)( value );
	}
};

//----------------------------------------------------------------

/*
	Converts read value via supplied conversion function
	before writing it to a field. Useful when you convert
	from string values into enums ("ENEMY" -> GM_ENEMY)
*/
template<typename TValue>
class XMLSchemaConvertedValueAttrib : public IXMLAttrib
{
	TValue & m_value;

public:
	typedef fastdelegate::FastDelegate< TValue ( const std::string & ) > ConversionDelegate;

private:
	ConversionDelegate m_converter;

public:
	XMLSchemaConvertedValueAttrib( const std::string & name, bool optional, const ConversionDelegate & converter, TValue & value )
		: IXMLAttrib( name, optional )
		, m_value( value )
		, m_converter( converter )
	{}

	bool Load( const TiXmlElement *pElement )
	{
		std::string strValue = "";
		if ( !GetAttributeValue( m_name, pElement, strValue ) )
			return false;

		m_value = m_converter( strValue );

		return true;
	}
};

//----------------------------------------------------------------

/*
	Deserializer which deserializes attributes from a tag
*/
class XMLAttribsDeserializer
{
	  // List of attributes to be deserialized
#ifndef HAS_STD_SHARED_PTR
	typedef std::vector<boost::shared_ptr<IXMLAttrib> > AttribVector;
#else
	typedef std::vector<std::shared_ptr<IXMLAttrib> > AttribVector;
#endif
	AttribVector m_attributes;

	  // Tries to add an attribute to list, replacing previous registred
	  // copy with the same name.
	void ReplaceAttrib( const std::string & name, IXMLAttrib *pAttrib )
	{
		for ( AttribVector::iterator iter = m_attributes.begin();
			  iter != m_attributes.end();
			  ++iter )
		{
			if ( (*iter)->GetName() == name )
			{
				(*iter).reset( pAttrib );
				return;
			}
		}

#ifndef HAS_STD_SHARED_PTR
		m_attributes.push_back( boost::shared_ptr<IXMLAttrib>( pAttrib ) );
#else
		m_attributes.push_back( std::shared_ptr<IXMLAttrib>( pAttrib ) );
#endif
	}

public:
	template <typename TValue>
	void Attrib_Value( const std::string & name, bool optional, TValue & ref)
	{
		ReplaceAttrib( name, new XMLSchemaValueAttrib<TValue>( name, optional, ref ) );
	}

	template <typename TValue>
	void Attrib_ConvertedValue( const std::string & name, bool optional, const typename XMLSchemaConvertedValueAttrib<TValue>::ConversionDelegate & converter, TValue & ref )
	{
		ReplaceAttrib( name, new XMLSchemaConvertedValueAttrib<TValue>( name, optional, converter, ref ) );
	}

	template<typename TObject, typename TValue>
	void Attrib_SetterValue( const std::string & name, bool optional, TObject & object, const typename XMLSchemaSetterValueAttrib<TObject, TValue>::TSetter & setter )
	{
		ReplaceAttrib( name, new XMLSchemaSetterValueAttrib<TObject, TValue>( name, optional, object, setter ) );
	}

	bool LoadAttribs( const TiXmlElement *pElement, bool inherited )
	{
		for( AttribVector::iterator iter = m_attributes.begin(); iter != m_attributes.end(); ++iter )
			if ( !(*iter)->Load( pElement ) && !(*iter)->IsOptional() && !inherited )
				return false;

		return true;
	}
};

//----------------------------------------------------------------

/*
	Deserializer which searches for a root element in document and then
	passes control to children deserializers.
*/
class RootXMLDeserializer : public BaseXMLDeserializer, public XMLAttribsDeserializer
{
public:
	RootXMLDeserializer( const std::string & name )
		: BaseXMLDeserializer( name, false )
	{}

	bool Deserialize( const TiXmlElement *pRoot )
	{
		if ( !pRoot || pRoot->Value() != m_tag )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Root element %s not found", m_tag.c_str() );
			return false;
		}

		if ( !LoadAttribs( pRoot, false ) )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Failed to load %s attribs", m_tag.c_str() );
			return false;
		}

		if ( !DeserializeChildren( pRoot ) )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Failed to load %s children", m_tag.c_str() );
			return false;
		}

		return true;
	}
};

//----------------------------------------------------------------

/*
	A wrapper for Root deserializer, which accepts file name,
	opens file and deserializes it.
*/
class XMLDeserializer
{
	RootXMLDeserializer & m_rootDeserializer;

public:
	XMLDeserializer( RootXMLDeserializer & rootDeserializer )
		: m_rootDeserializer( rootDeserializer )
	{}

	bool Deserialize( const std::string & fileName )
	{
		ISystem & sys = GetSystem();
		std::vector<char> data;

		if ( !sys.LoadFile( true, fileName.c_str(), data ) )
			return false;
		
		TiXmlDocument doc;
		doc.Parse( &data[0], 0, TIXML_ENCODING_UTF8 );
		if ( doc.Error() )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Failed to load XML file %s", fileName.c_str() );
			return false;
		}

		return Deserialize( doc );
	}

	bool Deserialize( const char *data, int len )
	{
		TiXmlDocument doc;
		doc.Parse( data );
		if ( doc.Error() )
		{
			GetLog().Log( 1, LM_FILE, LL_WARNING, "Failed to parse data" );
			return false;
		}

		return Deserialize( doc );
	}

	bool Deserialize( TiXmlDocument & doc )
	{
		return m_rootDeserializer.Deserialize( doc.RootElement() );
	}
};

//----------------------------------------------------------------

/*
	This deserializer tries to create an object from XML and
	pass it to receiver via Receiver function. You should note
	that this class will ITSELF allocate a new object!
*/
template<typename TObject>
class XMLObjectDeserializer : public BaseXMLDeserializer, public XMLAttribsDeserializer
{
public:
	typedef fastdelegate::FastDelegate< void ( const TObject & ) > TReceiveDelegate;
	void SetReceiver( const TReceiveDelegate & receiver ) { m_receiver = receiver; }
	template<typename TReceiver> void SetReceiver( TReceiver & object, void (TReceiver::*func)( const TObject & ) ) { m_receiver.bind( &object, func ); }

private:
	TReceiveDelegate m_receiver;

private:

	  // Implement this function to bind object's fields to XML attributes
	virtual void Bind( TObject & object ) = 0;

public:
	XMLObjectDeserializer( const std::string & tag, bool optional, const TReceiveDelegate & receiver )
		: BaseXMLDeserializer( tag, optional )
		, m_receiver( receiver )
	{}

	XMLObjectDeserializer( const std::string & tag, bool optional )
		: BaseXMLDeserializer( tag, optional )
	{}

	bool Deserialize( const TiXmlElement *pElement )
	{
		if ( !pElement )
			return false;

		const TiXmlElement *pSubElement = 0;
		while( ( pSubElement ? pSubElement = pSubElement->NextSiblingElement( m_tag ) : pSubElement = pElement->FirstChildElement( m_tag ) ) != 0 )
		{
			TObject object;
			Bind( object );

			if ( !LoadAttribs( pSubElement, false ) )
				continue;

			if ( !DeserializeChildren( pSubElement ) )
				continue;

			if ( m_receiver )
				m_receiver( object );
		}

		return true;
	}
};

//----------------------------------------------------------------

/*
	This deserializer tries to create a named object from XML and
	pass it to receiver via Receiver function. You should note
	that this class will ITSELF allocate a new object!

	Since object is named, it can use inheritance: copy all attributes
	from another object with specified name. A Getter functions, which
	can retrieve previously created objects by name should be supplied
	to support this.
*/
template<typename TObject, typename TID>
class XMLNamedObjectDeserializer : public BaseXMLDeserializer, public XMLAttribsDeserializer
{
	std::string m_idAttrib;

	struct SInheritor
	{
		SInheritor( const TiXmlElement *_pElement )
			: pElement( _pElement )
		{}

		const TiXmlElement *pElement;
	};

	std::vector<SInheritor> m_inheritors;

public:
	typedef fastdelegate::FastDelegate< void ( const TID &, const TObject & ) > TReceiveDelegate;
	typedef fastdelegate::FastDelegate< const TObject* ( const TID & ) > TGetDelegate;

	void SetReceiver( const TReceiveDelegate & receiver ) { m_receiver = receiver; }
	template<typename TReceiver> void SetReceiver( TReceiver & object, void (TReceiver::*func)( const TID &, const TObject & ) ) { m_receiver.bind( &object, func ); }
	
	void SetGetter( const TGetDelegate & getter ) { m_getter = getter; }
	template<typename TGetter> void SetGetter( TGetter & object, const TObject* (TGetter::*func)( const TID & )const ) { m_getter.bind( &object, func ); }

private:
	TReceiveDelegate m_receiver;
	TGetDelegate m_getter;

private:
	virtual void Bind( TObject & object ) = 0;	

private:
	bool LoadObject( TObject & object, const TiXmlElement *pElement, bool inherited )
	{
		TID id;
		if ( !GetAttributeValue( m_idAttrib, pElement, id ) )
			return false;

		Bind( object );

		if ( !LoadAttribs( pElement, inherited ) )
			return false;

		if ( !DeserializeChildren( pElement ) )
			return false;

		if ( m_receiver )
			m_receiver( id, object );

		return true;
	}

public:
	XMLNamedObjectDeserializer( const std::string & tag, bool optional, const std::string & idAttrib, const TReceiveDelegate & receiver, const TGetDelegate & getter )
		: BaseXMLDeserializer( tag, optional )
		, m_idAttrib( idAttrib )
		, m_receiver( receiver )
		, m_getter( getter )
	{}

	XMLNamedObjectDeserializer( const std::string & tag, bool optional, const std::string & idAttrib )
		: BaseXMLDeserializer( tag, optional )
		, m_idAttrib( idAttrib )
	{}

	bool Deserialize( const TiXmlElement *pElement )
	{
		const TiXmlElement *pSubElement = 0;
		while( ( pSubElement ? pSubElement = pSubElement->NextSiblingElement( m_tag ) : pSubElement = pElement->FirstChildElement( m_tag ) ) != 0 )
		{
			TID inheritID;
			if ( GetAttributeValue( "INHERIT", pSubElement, inheritID ) )
			{
				m_inheritors.push_back( pSubElement );
				continue;
			}

			TObject object;
			LoadObject( object, pSubElement, false );
		}

		if ( m_getter )
		{
			for ( typename std::vector<SInheritor>::iterator iter = m_inheritors.begin(); iter != m_inheritors.end(); ++iter )
			{
				pSubElement = iter->pElement;
				TID inheritID;
				GetAttributeValue( "INHERIT", pSubElement, inheritID );
				const TObject *pObject = m_getter( inheritID );
				if ( pObject )
				{
					TObject newObject = *pObject;
					LoadObject( newObject, pSubElement, true );
				}
				else
				{
					std::string inheritIDStr;
					GetAttributeValue( "INHERIT", pSubElement, inheritIDStr );
					GetLog().Log( 1, LM_FILE, LL_WARNING, "Inherited object %s not found", inheritIDStr.c_str() );
				}
			}
		}

		m_inheritors.clear();

		return true;
	}
};

//----------------------------------------------------------------

/*
	This class deserializes a piece of XML without creating any objects.
*/
class XMLDataDeserializer : public BaseXMLDeserializer, public XMLAttribsDeserializer
{
public:
	typedef fastdelegate::FastDelegate<bool ()> DataFinisherDelegate;
	DataFinisherDelegate m_dataFinisher;

public:
	XMLDataDeserializer( const std::string & tag, bool optional, DataFinisherDelegate finisher = 0 )
		: BaseXMLDeserializer( tag, optional )
		, m_dataFinisher( finisher )
	{}
	
	bool Deserialize( const TiXmlElement *pElement )
	{
		const TiXmlElement *pSubElement = 0;
		while( ( pSubElement ? pSubElement = pSubElement->NextSiblingElement( m_tag ) : pSubElement = pElement->FirstChildElement( m_tag ) ) != 0 )
		{
			if ( !LoadAttribs( pSubElement, false ) )
				return false;

			if ( !DeserializeChildren( pSubElement ) )
				return false;

			if ( m_dataFinisher )
			{
				if ( !m_dataFinisher() )
				{
					GetLog().Log( 1, LM_FILE, LL_WARNING, "Data finisher failed for %s", m_tag.c_str() );
					return false;
				}
			}
		}

		return true;
	}

	void Finisher( DataFinisherDelegate finisher )
	{
		m_dataFinisher = finisher;
	}
};

//----------------------------------------------------------------

/*
	This class allows you, when all else fails, to insert your
	own deserialization function in the middle of this structure
	of deserializers.
*/
class XMLCustomDeserializer : public BaseXMLDeserializer
{
	virtual bool CustomDeserialize( const TiXmlElement *pElement ) = 0;

public:
	XMLCustomDeserializer( const std::string & tag, bool optional )
		: BaseXMLDeserializer( tag, optional )
	{}

	bool Deserialize( const TiXmlElement *pElement )
	{
		const TiXmlElement *pSubElement = 0;
		while( ( pSubElement ? pSubElement = pSubElement->NextSiblingElement( m_tag ) : pSubElement = pElement->FirstChildElement( m_tag ) ) != 0 )
		{
			if ( !CustomDeserialize( pSubElement ) && !m_optional )
			{
				GetLog().Log( 1, LM_FILE, LL_WARNING, "Failed to deserialize %s", m_tag.c_str() );
				return false;
			}
		}

		return true;
	}
};

//----------------------------------------------------------------
