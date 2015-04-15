#include "stdafx.h"
#include "BitmapManager.h"
#include "IRender.h"
#include "AllegroImage5.h"
#ifdef WIN32
//Uncomment to debug
//#include "allegro5/internal/aintern_bitmap.h"
#endif

IImage *BitmapManager::Load( const std::string & path )
{
	ALLEGRO_BITMAP *bmp = al_load_bitmap( path.c_str() );
	if ( bmp )
	{
		al_convert_mask_to_alpha( bmp, al_map_rgb(255,0,255));
		return new AllegroImage5( bmp );
	}
	else
	{
		assert( false );
		return 0;
	}
}

IImage *BitmapManager::Create( const BitmapParams & params )
{
	const int oldFlags = al_get_new_bitmap_flags();
	int newFlags = oldFlags;

	if ( params.noPreserve )
		newFlags |= ALLEGRO_NO_PRESERVE_TEXTURE;

	if ( params.memory )
	{
		newFlags |= ALLEGRO_MEMORY_BITMAP;
		newFlags &= ~ALLEGRO_VIDEO_BITMAP;
	}

	al_set_new_bitmap_flags( newFlags );
	ALLEGRO_BITMAP *bmp = al_create_bitmap( params.w, params.h );		
	al_set_new_bitmap_flags( oldFlags );

	if ( bmp )
	{
		AllegroImage5 *img = new AllegroImage5( bmp );
		m_contexts[ (intptr_t)img ] = params.context;
		return img;
	}

	return 0;
}

void BitmapManager::Delete( IImage *res )
{
	delete res;
}

void BitmapManager::ReloadAllResources()
{
	for ( ResVector::iterator iter = m_resources.begin();
		iter != m_resources.end();
		++iter )
	{
		if ( !iter->m_res )
			continue;

		bool reloaded = false;
		const int index = (iter-m_resources.begin());
		for ( NameMap::const_iterator iter2 = m_nameMap.begin(); iter2 != m_nameMap.end(); ++iter2 )
		{
			if ( iter2->second.m_index == index && iter2->second.m_passport == iter->m_passport )
			{
				al_destroy_bitmap( ((AllegroImage5*)iter->m_res)->GetBitmap() );
				ALLEGRO_BITMAP *pNew = al_load_bitmap( iter2->first.c_str() );
				iter->m_res = new AllegroImage5( pNew );
				reloaded = true;
				break;
			}
		}

		if ( !reloaded )
		{
			ALLEGRO_BITMAP *pNew = al_clone_bitmap( ((AllegroImage5*)iter->m_res)->GetBitmap() );
			al_destroy_bitmap( ((AllegroImage5*)iter->m_res)->GetBitmap() );
			iter->m_res = new AllegroImage5( pNew );
		}
	}
}

// For debugging. Change path to your own if you need this too
//#define ASSERT
//#include "c:/alleg/5.0.5/src/win/d3d.h"
//int BitmapManager::CountModified()
//{
//	int modified = 0;
//	for ( ResVector::iterator iter = m_resources.begin();
//		iter != m_resources.end();
//		++iter )
//	{
//		ALLEGRO_BITMAP *pAlB = ((AllegroImage5*)iter->m_res)->GetBitmap();
//		ALLEGRO_BITMAP_D3D *pD3DBmp = (ALLEGRO_BITMAP_D3D *)pAlB;
//		const int index = (iter-m_resources.begin());
//		if ( pD3DBmp->modified && pAlB->preserve_texture )
//		{
//			const std::string & context = m_contexts[ (int)(iter->m_res) ];
//			std::string name = "";
//			for ( NameMap::const_iterator it = m_nameMap.begin(); it != m_nameMap.end(); ++it )
//			{
//				if ( it->second.m_index == (iter-m_resources.begin()) )
//				{
//					name = it->first;
//					break;
//				}
//			}
//			++modified;
//		}
//	}
//	return modified;
//}

BitmapManager & GetBitmapManager()
{
	static BitmapManager man;
	return man;
}
