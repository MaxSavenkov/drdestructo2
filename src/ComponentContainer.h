#pragma once
#include "ComponentID.h"

/*
	Base component container/pool. Implemented as a vector
	of objects + lists of free objects for index reuse and
	a list of active objects for iteration
*/
template<typename TComp>
class CompList
{
	/*
		Component + it's status
	*/
	struct SCompHolder
	{
		SCompHolder()
			: m_active( false )
		{}

		bool m_active;
		TComp m_comp;
	};

	typedef std::vector<SCompHolder> TCompVector;
	TCompVector m_components;

	typedef std::vector<int> TFreeVector;
	TFreeVector m_free;

	typedef std::list<int> TActiveList;
	TActiveList m_active;

public:
	/*
		Iterator for this container. Note that it doesn't
		conforms to STL iterator interface.
	*/
	class Iterator
	{
		friend class CompList<TComp>;

		CompList<TComp> & list;
		TActiveList::iterator iter;
	public:
		Iterator operator ++()
		{
			if ( iter != list.m_active.end() )
			{
				++iter;
				return *this;
			}

			return *this;
		}

		TComp* operator *()
		{
			if ( iter == list.m_active.end() )
				return 0;

			const int index = *iter;
			if ( index < 0 || index >= (int)list.m_components.size() )
				return 0;

			return &list.m_components[ index ].m_comp;
		}

		bool AtEnd() const
		{
			return iter == list.m_active.end();
		}

	private:
		Iterator( CompList<TComp> & _list )
			: list( _list )
		{
			iter = list.m_active.begin();
		}
	};

	/*
		Constant iterator. You can probably do this
		without copy&paste, but I was too lazy to
		think about it at the moment.
	*/
	class ConstIterator
	{
		friend class CompList<TComp>;

		const CompList<TComp> & list;
		TActiveList::const_iterator iter;
	public:
		ConstIterator operator ++()
		{
			if ( iter != list.m_active.end() )
				++iter;

			return *this;
		}

		const TComp* operator *() const
		{
			if ( iter == list.m_active.end() )
				return 0;

			const int index = *iter;
			if ( index < 0 || index >= (int)list.m_components.size() )
				return 0;

			return &list.m_components[ index ].m_comp;
		}

		bool AtEnd() const
		{
			return iter == list.m_active.end();
		}

	private:
		ConstIterator( const CompList<TComp> & _list )
			: list( _list )
		{
			iter = list.m_active.begin();
		}
	};

public:
	TComp *Insert()
	{
		SCompHolder *pHolder = 0;

		if ( !m_free.empty() )
		{
			pHolder = &m_components[ m_free.back() ];
			m_active.push_back( m_free.back() );
			m_free.pop_back();
		}
		else
		{
			m_components.push_back( SCompHolder() );
			pHolder = &m_components.back();
			pHolder->m_comp = TComp(ComponentID<TComp>( (int)m_components.size()-1, 0 ));
			m_active.push_back( (int)m_components.size()-1 );
		}

		pHolder->m_active = true;
		return &pHolder->m_comp;
	}

	virtual void Remove( const ComponentID<TComp> & id )
	{
		const int index = id.GetIndex();
		const int passport = id.GetPassport();

		if ( index < 0 || index >= (int)m_components.size() )
			return;

		if ( passport != m_components[ index ].m_comp.GetID().GetPassport() )
			return;

		m_components[ index ].m_active = false;
		m_components[ index ].m_comp = TComp( ComponentID<TComp>( index, m_components[ index ].m_comp.GetID().GetPassport()+1) );
		m_free.push_back( index );
		m_active.remove( index );
	}

	const TComp *Get( const ComponentID<TComp> & id ) const
	{
		const int index = id.GetIndex();
		const int passport = id.GetPassport();

		if ( index < 0 || index >= (int)m_components.size() )
			return 0;

		if ( passport != m_components[ index ].m_comp.GetID().GetPassport() )
			return 0;

		if ( !m_components[ index ].m_active )
			return 0;

		return &m_components[ index ].m_comp;
	}

	TComp *Get( const ComponentID<TComp> & id )
	{
		const int index = id.GetIndex();
		const int passport = id.GetPassport();

		if ( index < 0 || index >= (int)m_components.size() )
			return 0;

		if ( passport != m_components[ index ].m_comp.GetID().GetPassport() )
			return 0;

		if ( !m_components[ index ].m_active )
			return 0;

		return &m_components[ index ].m_comp;
	}

	int GetActiveCount() const
	{
		return (int)m_active.size();
	}

	void Clear()
	{
		m_components.clear();
		m_active.clear();
		m_free.clear();
	}

	Iterator GetIterator()
	{
		return Iterator( *this );
	}

	ConstIterator GetConstIterator() const
	{
		return ConstIterator( *this );
	}
};
