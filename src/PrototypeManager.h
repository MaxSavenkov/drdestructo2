#pragma once
#include "ResourceHandle.h"

template< typename TResource > class PrototypeManager;

  // Reference to resource. Not really used anywhere, but should be?
template< typename TResource >
class ResourceRef
{
	const PrototypeManager<TResource> *m_pManager;
	Handle<TResource> m_handle;
	mutable const TResource *m_pResource;

private:
	template<typename> friend class PrototypeManager;	
	ResourceRef( const PrototypeManager<TResource> & manager, const Handle<TResource> & handle )
		: m_pManager( &manager )
		, m_handle( handle )
	{
		m_pResource = manager.GetResource( handle );
	}

public:
	ResourceRef()
		: m_pManager( 0 )
		, m_pResource( 0 )
	{}

	inline void Reacquire() const
	{
		if ( m_pManager )
			m_pResource = m_pManager->GetResource( m_handle );
	}

	inline const TResource *Get() const
	{
		if ( m_pManager && !m_pManager->IsValidHandle( m_handle ) )
			Reacquire();

		return m_pResource;
	}

	inline const TResource *Get()
	{
		if ( m_pManager && !m_pManager->IsValidHandle( m_handle ) )
			Reacquire();

		return m_pResource;
	}

	inline operator const TResource*() const { return Get(); }
	//inline operator TResource*() { return Get(); }

	bool IsValid() const
	{
		return m_pManager && m_pManager->IsValidHandle( m_handle );
	}
};

  // A version of ResourceManager, geared toward storing object prototypes
  // It never unloads them (as prototypes are quite small)
template< typename TResource >
class PrototypeManager
{
public:
	typedef Handle<TResource> THandle;
	typedef ResourceRef<TResource> Ref;

private:
protected:
	struct ResourceInfo
	{
		TResource m_res;
		unsigned int m_passport;
	};

	typedef std::vector< ResourceInfo > ResVector;
	ResVector m_resources;
	typedef std::map< std::string, unsigned int > NameMap;
	NameMap m_nameMap;

	unsigned int MakeFreeIndex()
	{
		ResourceInfo resInfo;
		resInfo.m_passport = 1;
		m_resources.push_back( resInfo );
		return (unsigned int)m_resources.size() - 1;
	}
public:	
	PrototypeManager()
	{}

	virtual ~PrototypeManager()
	{
		for ( typename ResVector::iterator iter = m_resources.begin();
			  iter != m_resources.end();
			  ++iter)
		{
			Delete( iter->m_res );
		}

		Clear();
	}

	Ref GetResourceRef( const std::string & path ) const
	{
		const NameMap::const_iterator iter = m_nameMap.find( path );
		if ( iter != m_nameMap.end() )
		{
			return Ref( *this, THandle( iter->second, m_resources[ iter->second ].m_passport ) );
		}

		return Ref();
	}

	const TResource *GetResource( const std::string & path ) const
	{
		const NameMap::const_iterator iter = m_nameMap.find( path );
		if ( iter != m_nameMap.end() )
		{
			return &m_resources[ iter->second ].m_res;
		}

		return 0;
	}

	const TResource *GetResource( THandle handle ) const
	{
		unsigned int index = handle.GetIndex();
		unsigned int passport = handle.GetPassport();

		if ( index >= (int)m_resources.size() )
			return 0;

		if ( m_resources[ index ].m_passport != passport )
			return 0;

		return &m_resources[ index ].m_res;
	}

	void AddResource( const std::string & path, const TResource & res )
	{
		unsigned int index = MakeFreeIndex();
		m_resources[ index ].m_res = res;
		m_resources[ index ].m_passport = 0;
		m_nameMap.insert( std::make_pair( path, index ) );
	}

	void Clear()
	{
		for( typename ResVector::iterator iter = m_resources.begin();
			 iter != m_resources.end();
			 ++iter )
		{
			Delete( iter->m_res );
		}

		m_resources.clear();
		m_nameMap.clear();		
	}

	bool IsValidHandle( const THandle & handle ) const
	{
		unsigned int index = handle.GetIndex();
		unsigned int passport = handle.GetPassport();

		if ( index >= (int)m_resources.size() )
			return false;

		if ( m_resources[ index ].m_passport != passport )
			return false;

		return true;
	}

	virtual void Delete( TResource res ){}
	virtual void LoadResources() = 0;
};
