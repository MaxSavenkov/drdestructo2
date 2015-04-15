#include <vector>
#include <map>
#include <string>
#include <list>
#include <bitset>
#include <algorithm>
#include <set>
#include <deque>

#undef int8_t
#undef int_least8_t
#undef int_fast8_t
#undef uint8_t
#undef uint_least8_t
#undef uint_fast8_t
#undef int16_t
#undef int_least16_t
#undef int_fast16_t
#undef uint16_t
#undef uint_least16_t
#undef uint_fast16_t
#undef int32_t
#undef int_least32_t
#undef int_fast32_t
#undef uint32_t
#undef uint_least32_t
#undef uint_fast32_t
#undef intmax_t
#undef uintmax_t
#undef int64_t
#undef int_least64_t
#undef int_fast64_t
#undef uint64_t
#undef uint_least64_t
#undef uint_fast64_t

#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_real.hpp"
#include "boost/random/uniform_int.hpp"
//#ifndef ALLEGRO_ANDROID
#ifndef HAS_STD_SHARED_PTR
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include "boost/unordered_map.hpp"
#else
#include <unordered_map>
#endif

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml/tinyxml.h"

extern "C"
{
#include "lua/lua.h"
# include "lauxlib.h"
# include "lualib.h"
}
#include "luabind/luabind.hpp"

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#ifdef WIN32
#include <allegro5/allegro_direct3d.h>
#endif
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#ifdef WIN32
//#include <allegro5/allegro_native_dialog.h>
#endif

#ifndef WIN32
#define _snprintf snprintf
#endif
