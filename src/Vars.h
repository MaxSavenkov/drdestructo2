#pragma once

  // Class which stores global configurable game variables
class GlobalVariablesStorage
{
	enum EType
	{
		INT,
		FLOAT,
		STRING,
	};

public:
	struct IVar
	{
		virtual ~IVar(){}
	};

	template<typename T>
	struct SVar : public IVar
	{
		T m_value;
	};
private:

	std::map<std::string, IVar*> m_vars;

	GlobalVariablesStorage( const GlobalVariablesStorage & );
	void operator = ( const GlobalVariablesStorage & ) const;

	EType ParseType( const std::string & type );

	template<typename T>
	void SetVariable( const std::string & name, const T & value );

public:
	GlobalVariablesStorage()
	{}

	~GlobalVariablesStorage()
	{
		for ( std::map<std::string, IVar*>::iterator iter = m_vars.begin(); iter != m_vars.end(); ++iter )
			delete iter->second;

		m_vars.clear();
	}

	template<typename T>
	SVar<T> *RegisterVar( const std::string & name, const T & defaultValue = T() )
	{
		if ( m_vars.find( name ) != m_vars.end() )
		{
			assert( false && "Variable registred twice!" );
			return 0;
		}

		SVar<T> *pVar = new SVar<T>();
		pVar->m_value = defaultValue;
		m_vars[ name ] = pVar;
		return pVar;	
	}

	template<typename T>
	SVar<T> *GetVar( const std::string & name )
	{
		std::map<std::string, IVar *>::iterator iter = m_vars.find( name );
		if ( iter != m_vars.end() )
		{
			return (SVar<T>*)iter->second;
		}

		return 0;
	}

	void Load( const std::string & path );
};

GlobalVariablesStorage & GetVars();

  // Use this class to declare a new global variable.
  // Will assert if variable with this name is already registred
template<typename T>
class RegisterVariable
{
	GlobalVariablesStorage::SVar<T> *m_pVar;
public:
	RegisterVariable( const std::string & name, const T & defValue = T() )
	{
		m_pVar = GetVars().RegisterVar( name, defValue );
	}

	operator T() const { return m_pVar->m_value; }
	operator const T & () const { return m_pVar->m_value; }
	operator const T & () { return m_pVar->m_value; }
	RegisterVariable & operator = ( const T & v ) { m_pVar->m_value = v; return *this; }
};

  // Use this class to have access to a global variable in particular cpp file.
  // Will assert if variable has not been registred!
template<typename T>
class UseVariable
{
	GlobalVariablesStorage::SVar<T> *m_pVar;
	std::string m_name;
public:
	UseVariable( const std::string & name )
		: m_pVar( 0 )
		, m_name( name )
	{	
	}

	inline void GetVar() { if ( !m_pVar ) m_pVar = GetVars().GetVar<T>( m_name ); assert( m_pVar && "Used variable not set!" ); }

	operator T () const { GetVar(); return m_pVar->m_value; }
	operator const T & () const { GetVar(); return m_pVar->m_value; }
	operator T & ()  { GetVar(); return m_pVar->m_value; }
	UseVariable & operator = ( const T & v ) { m_pVar->m_value = v; return *this; }
};

typedef RegisterVariable<int> RegisterIntVar;
typedef RegisterVariable<float> RegisterFloatVar;
typedef RegisterVariable<std::string> RegisterStringVar;

typedef UseVariable<int> UseIntVar;
typedef UseVariable<float> UseFloatVar;
typedef UseVariable<std::string> UseStringVar;
