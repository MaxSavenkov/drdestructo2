Source ReadMe file for Return of Dr. Destructo v1.0.
-------------------------------------------------

Content:

1. Foreword
2. Dependencies
3. Building Return of Dr. Destructo
4. General notes on architecture & code organization

-------------------------------------------------

1. Foreword

I decided to make my remake of Island of Dr. Destructo open-source for several reasons. One of them is to make sure the game could be ported
anywhere by anyone. My main development platform for now is Windows and I barely have time and willpower to make it compile on Linux and MacOS X,
much less on something more esoteric.

However, I would not recommend to use this code as a learning example. It's very far from perfect, since I cut corners whenever I could, simply
because I wanted to see the game complete, and writing better code sometimes take too much time when you can hack something together and get
instant results.

You may still find some insights here, though, but be sure to read section 4.

-------------------------------------------------

2. Dependencies

Return of Dr. Destructo depends on the following external libraries:

* Allegro 5.1.x (http://www.allegro.cc/). NOT Allegro 4.4, which is included in most Ubuntu-related repositories!
* TinyXML 2.6.2 (http://www.grinninglizard.com/tinyxml/). WARNING! The game will use HACKED header files to allow correct reading of strings with spaces! Files included with the game.
* Lua 5.1 (http://www.lua.org/)
* Luabind 0.9.1 (http://www.rasterbar.com/products/luabind.html). WARNING! Do not use DLL version! Only STATICALLY linked version works!
* Boost 1.44 (http://www.boost.org/). Necessary files are included with the game code, no need to download anything.
* Google Breakpad (Windows build only)

-------------------------------------------------

3. Building Return of Dr. Destructo

Since 1.0, Return of Dr. Destructo is built using CMake. Currently, builds for Windows, Linux and MacOS X are fully supported.
Be aware, that if you are building on MacOS X with "Xcode" generator, you may need to copy Data folder by hand into configuration's
folder.

The game builds most of its dependencies itself, but expects Allegro to be pre-built by user.

-------------------------------------------------

4. General notes on architecture & code organization

Code organization:

The code lacks any filesystem-level organization, since I do my coding in Visual Studio and prefer to organize it
in IDE. The only non-dependency folder in the source tree is Engine, where my Allegro wrapper resides (it's
not an actual game engine).

If you open the solution, however, you will find that code is organized in the following filters:
Engine - aforementioned wrapper
Game - the rest of the code
	Components	- classes, containers and utility structures related to GameObject Components (see below)
	Menu		- base menu class, helpers and some concrete menus
	Processors	- classes that process Components
	Prototypes	- classes that store base information for creating Components, deserializers that read them from XML files and Managers that store them
	States		- classes that encapsulated concrete game states

-------------------------------------------------

General notes:

I have mixed feelings about this project's code & architecture. On one hand, it was mostly written hastily, and I cut a lot of corners,
so it's not a very clean piece of code. On the other, it's been more than three years since I wrote the first line of code, and I still
can navigate it quite reliably and add small new features or fix bugs without descending into cursing. Which is more than I can say about
some other projects I have worked on.

Possibly, it is saved by its small size and short features list. Still, it's a complete, working game.

-------------------------------------------------

Architecture:

This project uses a mix of Object Oriented Programming paradigm, Component model and plain old "add hacks until it works" approach.

I use a variant of Component model here that's might be slightly unusual for many. Unlike Unity, or other Component-based engines,
my Components are simple data structures. They don't do their own processing, but only store relevant data. The real processing code,
instead, is separated into Processor classes, which take a list of Components of one type and proceed to do all calculations, rendering,
etc.

There is just one advantage of this mode. When a processing code needs to operate on more than one Component, for some reason, it can
easily access it. If this code was instead written inside the Component's own method, it would require this component to know about
other components. In present approach, only Processors know about different Components, but Components themselves are isolated. Not much
of an advantage, I must admit, but it's one possible solution to a question of inter-Component communication that is important for application
of Component model.

-------------------------------------------------

On Components:

GameObject is the main Component, that only acts as an unifying container for other Components and has no other data of its own. If you
read Game Programmin Patterns, you must remember this approach as one of possible variations. Unfortunately, I kind of botched it here,
because GameObject do not store references to Components, but only their IDs, which leads to a very verbose way of accessing a Component:

	const GameObject *pObj = context.GetOjectsStorage().GetGameObject( objectID );
	const PhysicComponent *pMePhys = context.GetOjectsStorage().GetPhysicComponent( pObj->GetPhysicComponent() );

Now, I would have written it differently.

The rest of components are:

ControlComponent  - stores things related to controlling a game object, like player's inputs or AI state
GraphicComponent  - stores references to textures and animation frames etc.
MechanicComponent - stores logical data, like weapons' states, logical states (stall etc.)
PhysicComponent   - stores location, speed and other physical state of an object
SoundComponent    - stores currently everything that is needed to play sounds

Most of the time, the code does not create separate Components directly, but just makes a request to create a GameObject from Prototype
(this is not a Prototype in Programming Patterns terms, but rather a mix between Flyweight and something else).

However, there are a few cases when I call CreateGraphCompImmediate to create a GraphicComponent that is not a part of any GameObject, but
is drawn as a part of the scene.

-------------------------------------------------

On Game States:

That's all for Component model. The rest of the game is more OOP than anything. The flow of the game itself is described through GameStates,
classes that represent different screens and states encountered in it. The whole thing is a state machine, of the type called Pushdown Automata
by Game Programming Patterns book. There is a stack of states, onto which a state could be put. Only the top state gets to process user's input
and time updates, but all states are rendered. Unlike some implementations, you can not only remove a state from the top of the stack. When you
remove a state, all states above it are also removed, as they are considered to be children of bottom-most state.

-------------------------------------------------

On AI code:

A special mention should be made of AI code. The main idea for enemies AI was to write a script that consist of a series of commands that look
like this:

DO something UNTIL condition

For example:

DO fly forward at a constant speed UNTIL delta X from starting point >= 500
DO fly up AND forward at a constant speed UNTIL delta Y from starting point >= 50
DO fly forward at a constant speed UNTIL forever

This simple pseudi-script describes a behaviour of an enemy plane that flies level for a bit, than changes height, than flies level again. There
are more complex behaviours in the game.

To implement this, I used Lua's ability to yield control back to calling C++ function. If you take a look at ai.lua, you will see scripts like that:

function height_change_up( context )
    local dx = RandomInt( 100, 900 )
    local dy = 100
      
    context:Control( Context.IDLE,				Trigger_DX( dx )			)
    context:Control( Context.CTRL_VERT_UP,		Trigger_DY( dy )			)
    context:Control( Context.IDLE,				Trigger_Eternal()			)
end

This is actually a representation of the pseudo-script above. The context:Controll call applies a control to current object, and yields until the specified
condition (Trigger_*) is satisfied. If the end of the function is ever reached, it will be re-run the next frame from the beginning, which allows me to
create repeating behaviour patterns with randomization for each iteration.

To achieve this, I use "yield" policy when registring C++ function in Luabind (.def( "Control", &IScriptContext::Control, yield ) for example).

Yielding saves the full Lua state, and it cannot be reused to run another function without corrupting it, so I use Lua threads to run several AI scripts at
once, which allow reuse of the main Lua state by creating lightweight state copies.

A note: if you ever want to to make calls from Lua to C++, try to avoid Luabind. It's a pain to compile and use, and no longer actively developed. Also,
it has problems with compilation on modern C++11 compilers and requires Boost.

-------------------------------------------------

On Deserializers:

In this game, I have once again tried to tackle the hairy topic of object deserialization in C++. The end result of this was a set of XMLDeserializer
classes. They allow you to describe objects contained in an XML document via additional classes and deserialize them into containers. This system is
neither easy to use, nor easy to understand, because the task proved to be much more complex than I expected. I already have another, better version
of deserialization code that I used in another game, but I did not want to rewrite all deserialization code here to use it.

So, here are some basics. To start deserializing a document, you need to create a descendant of RootXMLDeserializer. It should name the root element used
by your XML document, and list sub-deserializers for all entities included in it. Sub-deserializers are declared in the same recursive manner.

To bind an attribute of an XML tag to field in object, you need to use Attrib_* functions. This is done in overloaded Bind method, into which a newly created
object of a given type is passed. Let's have a look at an example from GraphicsProtoManager.cpp.

Here's a root deserializer:

class GraphicsDeserializer : public RootXMLDeserializer
{
public:
	GraphicDeserializer m_graphicDes;

	GraphicsDeserializer()
		: RootXMLDeserializer( "Graphics" )
	{
		SubDeserializer( m_graphicDes ); 
	}
};

It declares that it is going to work with documents that have root element named <Graphics>, and that there is only one sub-deserializer. The code for
sub-deserializer looks like this:

class GraphicDeserializer : public XMLNamedObjectDeserializer<SGraphicsProto, std::string>
{
	class AnimDeserializer : public XMLNamedObjectDeserializer<SAnimProto, AnimationID>
	{
		class FrameDeserializer : public XMLObjectDeserializer<SAnimFrame>
		{
		public:
			FrameDeserializer() : XMLObjectDeserializer<SAnimFrame>( "Frame", false )
			{}

			void Bind( SAnimFrame & object )
			{
				Attrib_Value( "X", false, object.x );
				Attrib_Value( "Y", false, object.y );
				Attrib_Value( "W", true, object.w );
				Attrib_Value( "H", true, object.h );
				Attrib_Value( "FlipH", true, object.flipH );
				Attrib_Value( "FlipV", true, object.flipV );
				Attrib_SetterValue<SAnimFrame, int>( "X2", true, object, &SAnimFrame::SetX2 );
				Attrib_SetterValue<SAnimFrame, int>( "Y2", true, object, &SAnimFrame::SetY2 );
			}
		}m_frameDes;

	public:
		AnimDeserializer()
			: XMLNamedObjectDeserializer<SAnimProto, AnimationID>( "Animation", false, "ID" )
		{
			SubDeserializer( m_frameDes );
		}

		void Bind( SAnimProto & object )
		{
			Attrib_Value( "FPS", false, object.m_fps );
			Attrib_Value( "Dir", true, object.m_dir );
			Attrib_Value( "Reverse", true, object.m_reverse );
			Attrib_Value( "FlipV", true, object.m_flipV );
			Attrib_Value( "FlipH", true, object.m_flipH );
			Attrib_Value( "OneShot", true, object.m_oneShot );
			Attrib_Value( "SoundEvent", true, object.m_soundEvent );
			m_frameDes.SetReceiver( object, &SAnimProto::AddFrame );
		}
	};

private:
	XMLDataDeserializer m_imgDes;
	XMLDataDeserializer m_bgDes;
	XMLDataDeserializer m_capsDes;
	AnimDeserializer m_animDes;

	void Bind( SGraphicsProto & object )
	{
		Attrib_Value( "Layer", false, object.m_layerID );
		m_animDes.SetReceiver( object, &SGraphicsProto::SetAnim );
		m_animDes.SetGetter<SGraphicsProto>( object, &SGraphicsProto::GetAnim );
		m_imgDes.Attrib_Value( "Path", false, object.m_image );
		m_bgDes.Attrib_Value( "Path", false, object.m_imageBg );
		m_capsDes.Attrib_SetterValue<SGraphicsProto, int>( "ID", false, object, &SGraphicsProto::SetCaps );
	}

public:
	GraphicDeserializer()
		: XMLNamedObjectDeserializer<SGraphicsProto, std::string>( "Graphic", true, "Name")
		, m_imgDes( "Image", false )
		, m_bgDes( "Bg", true )
		, m_capsDes( "Caps", false )
	{
		SubDeserializer( m_imgDes ); 
		SubDeserializer( m_bgDes ); 
		SubDeserializer( m_animDes ); 
		SubDeserializer( m_capsDes ); 
	}
};

It's a bit too much to take in all at once, so let's walk this through. In GraphicDeserializer(), we declare this
sub-deserializer to work with tags named <Graphic>, for which identifying tag is Name. Identifying tags are used
for inheritance, so that when you write <Graphics Name="Descendant" INHERIT="Ancestor"> all data from Graphics
object named "Ancestor" would be copied into a newly created object named "Descendant" before any further actions.

We then declare some more sub-deserializers for <Image>, <Bg> and <Caps> tags.

Bind method of GraphicDeserializer does some more work. When it is called, a new SGraphicsProto structure is
already allocated and passed to it, so we may work with it.

First, we declare that "Layer" attribute from the <Graphic> element we are currently reading should be put into
object.m_layerID. The second parameter to Attrib_Value (false) indicates that this is a non-optional element, and
if it is missing, an error will be logged.

Further along, we call SetReceiver and SetGetter function for m_animDes, the animation deserializer. Receiver function
will be used by m_animDes to add new animations to our object. Getter function will be used to retrieve previously
deserialized animations by name, in case we encounter an INHERIT tag.

Then, we bind some attributes for img, bg and caps deserializers.

The rest of the class proceeds recursively in the same manner. If you still don't understand what's going on, I don't
blame you - this is unnecesarily complex!

-------------------------------------------------
