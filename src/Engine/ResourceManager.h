#pragma once

  // Generic resource manager
#ifdef WIN32
template< typename TResource, typename TResourceParams, const TResource TInvalid, typename THandle >
#else
template< typename TResource, typename TResourceParams, int TInvalid, typename THandle >
#endif
class ResourceManager
{
protected:
	  // Stores resource along with it's passport - an identifier that allows index reuse
	struct ResourceInfo
	{
		TResource m_res;
		unsigned int m_passport;
	};

	  // Information about resource associated with some name vis NameMap
	struct NameInfo
	{
		NameInfo()
			: m_index( -1 )
			, m_passport( -1 )
		{}

		NameInfo( int i, int p )
			: m_index( i )
			, m_passport( p )
		{}
		int m_index;
		int m_passport;
	};

	  // All resources
	typedef std::vector< ResourceInfo > ResVector;
	ResVector m_resources;
	  // Mapping from resource names to NameInfo
	typedef std::map< std::string, NameInfo > NameMap;
	NameMap m_nameMap;
	  // List of free indices for reuse
	typedef std::vector< unsigned int > FreeList;
	FreeList m_freeList;

private:
	  // Gets resource by path/name only if it is present
	THandle GetResource( const std::string & path )
	{
		const typename NameMap::iterator iter = m_nameMap.find( path );
		if ( iter != m_nameMap.end() && iter->second.m_passport == m_resources[ iter->second.m_index ].m_passport )
		{
			return THandle( iter->second.m_index, m_resources[ iter->second.m_index ].m_passport );
		}

		return THandle::INVALID;
	}

	  // Loads resource by path/name and stores it a specified index with specified passport.
	  // Should have a valid index+passport, of course
	THandle LoadResource( const std::string & path, unsigned int index, unsigned int passport )
	{
		if ( index >= UINT_MAX || passport >= UINT_MAX )
			return THandle::INVALID;

		TResource res = Load( path );
		if ( res == TInvalid )
		{
			m_resources[ index ].m_res = TInvalid;
			m_resources[ index ].m_passport = passport;

			return THandle::INVALID;
		}
		
		m_resources[ index ].m_res = res;
		m_resources[ index ].m_passport = passport;

		m_nameMap.insert( std::make_pair( path, NameInfo( index, passport ) ) );

		return THandle( index, passport );
	}

	  // Returns an index for use for a new resource
	unsigned int MakeFreeIndex()
	{
		if ( m_freeList.empty() )
		{
			ResourceInfo resInfo;
			resInfo.m_res = TInvalid;
			resInfo.m_passport = 0;
			m_resources.push_back( resInfo );
			return (unsigned int)m_resources.size() - 1;
		}

		unsigned int index = m_freeList[ 0 ];
		m_freeList.erase( m_freeList.begin() );
		return index;
	}
public:	
	ResourceManager()
	{}

	virtual ~ResourceManager()
	{
		// We can't usually do clearing here, as
		// most Managers are singletons and destroyed
		// AFTER Allegro and other libraries which can
		// free resources are already de-initialized.
		// Short version: we can't clear anything automatically, it will crash.
	}

	  // Loads resource OR returns already loaded one
	THandle LoadResource( const std::string & path )
	{
		if ( path.empty() )
			return THandle::INVALID;

		THandle h = GetResource( path );
		if ( h != THandle::INVALID )
			return h;

		const int freeIndex = MakeFreeIndex();
		const int passport = m_resources[ freeIndex ].m_passport + 1;
		return LoadResource( path, freeIndex, passport );
	}

	  // Reloads resource if it's not present or loads it.
	THandle ReloadResource( const std::string & path )
	{
		THandle h = GetResource( path );

		unsigned int passport = 1;
		unsigned int index = UINT_MAX;

		if ( h != THandle::INVALID )
		{
			Delete( m_resources[ h.GetIndex() ].m_res );			
			index = h.GetIndex();
		}
		else
		{
			index = MakeFreeIndex();
		}

		passport = m_resources[ index ].m_passport + 1;

		return LoadResource( path, index, passport );
	}

	  // Creates a new resource via child's overridden function
	THandle CreateResource( const TResourceParams & params )
	{
		TResource res = Create( params );
		if ( res == TInvalid )
			return THandle::INVALID;

		const int index = MakeFreeIndex();
		m_resources[ index ].m_res = res;
		m_resources[ index ].m_passport = m_resources[ index ].m_passport + 1;

		return THandle( index, m_resources[ index ].m_passport );
	}

	  // Removes resource from list and frees its memory
	void UnloadResource( THandle handle )
	{
		unsigned int index = handle.GetIndex();
		unsigned int passport = handle.GetPassport();

		if ( index >= (int)m_resources.size() )
			return;

		if ( m_resources[ index ].m_passport != passport )
			return;

		Delete( ( m_resources[ index ].m_res ) );
		m_resources[ index ].m_res = TInvalid;
		++m_resources[ index ].m_passport;
		m_freeList.push_back( index );
	}

	  // Gets resource only if it's present
	TResource GetResource( THandle handle )
	{
		unsigned int index = handle.GetIndex();
		unsigned int passport = handle.GetPassport();

		if ( index >= (int)m_resources.size() )
			return TInvalid;

		if ( m_resources[ index ].m_passport != passport )
			return TInvalid;

		return m_resources[ index ].m_res;
	}

	  // Destroys all resources
	void Clear()
	{
		for( typename ResVector::iterator iter = m_resources.begin();
			 iter != m_resources.end();
			 ++iter )
		{
			if ( iter->m_res )
				Delete( iter->m_res );
		}

		m_resources.clear();
		m_freeList.clear();
		m_nameMap.clear();		
	}

	  // Functions for child managers, specific for resources
	virtual TResource Load( const std::string & path ) = 0;
	virtual void Delete( TResource res ) = 0;
	virtual TResource Create( const TResourceParams & params ) = 0;
};
