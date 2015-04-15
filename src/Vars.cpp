#include "stdafx.h"
#include "Engine/ISystem.h"
#include "Vars.h"

GlobalVariablesStorage::EType GlobalVariablesStorage::ParseType( const std::string & type )
{
	if ( type == "INT" )
		return INT;
	if ( type == "FLOAT" )
		return FLOAT;
	return STRING;
}

template<typename T>
void GlobalVariablesStorage::SetVariable( const std::string & name, const T & value )
{
	std::map<std::string, IVar*>::iterator iter = m_vars.find( name );
	
	if ( iter == m_vars.end() )
	{
		SVar<T> *newVar = new SVar<T>();
		newVar->m_value = value;
		m_vars[ name ] = newVar;
	}
	else
	{
		SVar<T> *pVar = static_cast<SVar<T>*>(iter->second);
		if ( pVar )
			pVar->m_value = value;
	}
}

/* 
  Loads variables from file. Use the following syntax:
  <Vars>
	<Var Name="NAME" Type="TYPE" Value="VALUE"/> - To declare a single variable
	<Enum Name="NAME" Type="TYPE" StartValue="StartValue"> - To declare several variables, with specified names, with value starting from StartValue and increasing by one.
	    <VARIABLE_NAME/>
	</Enum>
	<BitEnum Name="NAME" Type="TYPE" StartValue="StartValue"> - To declare several variables, with specified names and value 1 << i, where starts from StartValue and increased by one.
		<VARIABLE_NAME/>
	</BitEnum>
  </Vars>
*/

void GlobalVariablesStorage::Load( const std::string & path )
{
	ISystem & sys = GetSystem();
	std::vector<char> buf;
	if ( !sys.LoadFile( true, path.c_str(), buf ) )
		return;

	TiXmlDocument doc;
	if ( !doc.Parse( &buf[0] ) )
		return;

	const TiXmlElement *pRoot = doc.FirstChildElement( "Vars" );
	if ( !pRoot )
		return;

	const TiXmlElement *pEnum = 0;
	while( pEnum = pEnum ? pEnum->NextSiblingElement( "Enum" ) : pRoot->FirstChildElement("Enum" ) )
	{
		std::string name;
		std::string type;
		int startValue = 0;

		// Ignore name and type for now
		pEnum->QueryIntAttribute( "StartValue", &startValue );

		const TiXmlElement *pEl = 0;
		int val = startValue;
		while( pEl = pEl ? pEl->NextSiblingElement() : pEnum->FirstChildElement() )
		{
			SetVariable( pEl->Value(), val );
			++val;
		}
	}

	const TiXmlElement *pBitEnum = 0;
	while( pBitEnum = pBitEnum ? pBitEnum->NextSiblingElement( "BitEnum" ) : pRoot->FirstChildElement("BitEnum" ) )
	{
		std::string name;
		std::string type;
		int startValue = 0;

		// Ignore name and type for now
		pBitEnum->QueryIntAttribute( "StartValue", &startValue );

		const TiXmlElement *pEl = 0;
		int val = startValue;
		while( pEl = pEl ? pEl->NextSiblingElement() : pBitEnum->FirstChildElement() )
		{
			SetVariable( pEl->Value(), 1 << val );
			++val;
		}
	}

	const TiXmlElement *pVar = 0;
	while( pVar = pVar ? pVar->NextSiblingElement( "Var" ) : pRoot->FirstChildElement("Var" ) )
	{
		std::string name;
		std::string type;
		int startValue = 0;

		if ( pVar->QueryValueAttribute( "Name", &name ) != TIXML_SUCCESS )
			continue;
		if ( pVar->QueryValueAttribute( "Type", &name ) != TIXML_SUCCESS )
			continue;

		if ( type == "INT" )
		{
			int value;
			if ( pVar->QueryValueAttribute( "Value", &value ) != TIXML_SUCCESS )
				continue;
			SetVariable( name, value );
		}
		else if ( type == "FLOAT" )
		{
			float value;
			if ( pVar->QueryValueAttribute( "Value", &value ) != TIXML_SUCCESS )
				continue;
			SetVariable( name, value );
		}
		else if ( type == "STRING" )
		{
			std::string value;
			if ( pVar->QueryValueAttribute( "Value", &value ) != TIXML_SUCCESS )
				continue;
			SetVariable( name, value );
		}
	}
}

GlobalVariablesStorage & GetVars()
{
	static GlobalVariablesStorage vars;
	return vars;
}
