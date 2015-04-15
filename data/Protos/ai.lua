-- Set of AI functions for objects.
-- Most any function will actually yield (with the exception of parameter generation and other utility functions),
-- and will only continue execution the next time AIProcessor is called, or when trigger becomes true.
-- (When trigger is 0, the function will continue execution on the next AIPRocessor call)

function straight( context )
	context:Control( Context.IDLE,				Trigger_Eternal()			)
end

function straight_accel( context )
	context:Control( Context.CTRL_SPEED_UP,		Trigger_Eternal()			)
end

function height_change_up( context )
    local dx = RandomInt( 100, 900 )
    local dy = 100
      
    context:Control( Context.IDLE,				Trigger_DX( dx )			)
    context:Control( Context.CTRL_VERT_UP,		Trigger_DY( dy )			)
    context:Control( Context.IDLE,				Trigger_Eternal()			)
end

function height_change_down( context )
    local dx = RandomInt( 100, 900 )
    local dy = 100
            
    context:Control( Context.IDLE,				Trigger_DX( dx )			)
    context:Control( Context.CTRL_VERT_DOWN,	Trigger_DY( dy )			)
    context:Control( Context.IDLE,				Trigger_Eternal()			)
end

function plane_dead( context, deadObject )
    local timer = RandomFloat( 1, 3 )

	context:Control( Context.IDLE,				Trigger_Timer( timer )			)
	
	local params = context:CreateObjectParams()
	params.ai = ""
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 200
	context:CreateObject( deadObject, params, true )
end

function victim_prop_dead( context )
	plane_dead( context, "victim_prop_falling" )
end

function victim_jet_dead( context )
	plane_dead( context, "victim_jet_falling" )
end  

function enemy_prop_dead( context )
	plane_dead( context, "enemy_prop_falling" )
end

function enemy_jet_dead( context )
	plane_dead( context, "enemy_jet_falling" )
end

function explosion_on_target( context )
    context:Control( Context.IDLE,				Trigger_Timer( 1 )			)
    context:DestroySelf()
end

function skymine( context )
	local dy = RandomInt( 100, 300 )
	local time = RandomFloat( 3, 6 )

    context:Control( Context.CTRL_VERT_DOWN,			Trigger_DY( dy )			)
    context:Control( Context.IDLE,						Trigger_Timer( time )		)
    context:Control( Context.CTRL_VERT_UP,				Trigger_Eternal()			)
end

function copter( context )	
	local time = RandomFloat( 1, 3 )
	context:Control( Context.IDLE, Trigger_Timer( time ) )
	
	local nextAction = RandomInt( 0, 100 )
	
	if nextAction < 50 then
		context:Control( Context.CTRL_FIRE1, Trigger_OneTime() )
	elseif nextAction < 75 then
		context:Control( Context.CTRL_STOP, Trigger_OneTime() )
		context:Control( Context.CTRL_FLIP, Trigger_OneTime() )
		context:Control( Context.IDLE, Trigger_Timer( 1 ) )
		context:Control( Context.CTRL_GO, Trigger_OneTime() )
		local time = RandomFloat( 1, 6 )
		context:Control( Context.IDLE, Trigger_Timer( time ) )		
	end
end

function copter_dart_ai( context )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_DY(50) )
	context:Control( Context.CTRL_SPEED_UP, Trigger_Eternal() )
end

function enemy_copter_dead( context )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_OneTime() )
	while true do
		context:Control( Context.CTRL_FLIP, Trigger_OneTime() )
		context:Control( Context.IDLE, Trigger_Timer( 0.5 ) )
		context:Control( Context.CTRL_GO, Trigger_OneTime() )
	end
end

function two_seater_morph1( context )
	context:Control( Context.IDLE, Trigger_Timer( 0.3 ) )
	
	local params = context:CreateObjectParams()
	params.ai = "two_seater_morph2"
	context:CreateObject( "two_seater_morph", params, true )
end

function two_seater_morph2( context )
	local time = RandomFloat( 1, 2 )
	local types = { "victim_prop", "victim_jet", "enemy_prop", "enemy_jet" }
	local type = types[ RandomInt( 1, 4 ) ]
	
	context:Control( Context.IDLE, Trigger_Timer( time ) )
	
	local params = context:CreateObjectParams()
	params.ai = "straight"
	
	context:CreateObject( type, params, true )
end

function bomber( context )
	local time = RandomFloat( 1, 3 )
	context:Control( Context.IDLE, Trigger_Timer( time ) )
	
	local nextAction = RandomInt( 0, 100 )
	if nextAction < 18 then
		context:Control( Context.CTRL_VERT_DOWN, Trigger_DY(50) )		
	elseif nextAction < 36 then
		context:Control( Context.CTRL_VERT_UP, Trigger_DY(50) )
	elseif nextAction < 85 then
		local n = RandomInt( 2, 5 )
		for i = 1,n do
			context:Control( Context.CTRL_FIRE1, Trigger_OneTime() )
			context:Control( Context.IDLE, Trigger_Timer( 0.1 ) )
		end
	end		
end

function enemy_bomb_ai( context )
	local time = RandomFloat( 2, 3 )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_Timer(time) )
	
	local params = context:CreateObjectParams()
	params.ai = "bomb_explosion"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "bomb_explosion", params, true )	
end

function bomb_explosion( context )
	context:Control( Context.IDLE, Trigger_Timer(0.5) )
	context:DestroySelf()
end

function player_bomb_ai( context )
	local time = RandomFloat( 2, 3 )
	context:Control( Context.CTRL_SPEED_UP, Trigger_Timer(1.0) )
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_Eternal() )
end

function enemy_bomber_dead( context )
	context:Control( Context.CTRL_VERT_UP, Trigger_Timer(0.2) )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_OneTime() )
	while true do
		context:Control( Context.CTRL_FLIP, Trigger_OneTime() )
		context:Control( Context.IDLE, Trigger_Timer( 0.1 ) )
		context:Control( Context.CTRL_GO, Trigger_OneTime() )
	end	
end

function ufo_spawn( context )
	context:Control( Context.IDLE, Trigger_Timer( 0.3 ) )
	
	local params = context:CreateObjectParams()
	params.ai = "ufo_live"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "ufo", params, true )	
end

function ufo_live( context )
	local nextAction = RandomInt( 1, 2 )

	if nextAction == 1 then
		local repeats = RandomInt( 4, 7 )
		for i = 1, repeats do
			context:Control( Context.CTRL_VERT_DOWN, Trigger_Timer(0.3) )
			context:Control( Context.CTRL_VERT_UP, Trigger_Timer(0.3) )
		end
	else
		local time = RandomFloat( 0.8, 1.5 )
		context:Control( Context.CTRL_SPEED_UP, Trigger_Timer(time) )
	end
	
	local nextAIs = { "ufo_despawn", "ufo_end" }
	local nextAI = nextAIs[ RandomInt( 1, 2 ) ]
	local params = context:CreateObjectParams()
	params.ai = nextAI
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "ufo_despawn", params, true )
end

function ufo_despawn( context )
	context:Control( Context.IDLE, Trigger_Timer( 0.4 ) )
	context:Control( Context.CTRL_RANDOM_TELEPORT, Trigger_OneTime() )
	
	local params = context:CreateObjectParams()
	params.ai = "ufo_spawn"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "ufo_spawn", params, true )			
end

function ufo_end( context )
	context:Control( Context.IDLE, Trigger_Timer( 0.4 ) )
	context:DestroySelf()
end

function ufo_death( context )
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )
	context:Control( Context.IDLE, Trigger_Timer(0.2) )
	
	local params = context:CreateObjectParams()
	params.ai = "ufo_dead"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "ufo_dead", params, true )				
end

function ufo_dead( context )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_OneTime() )
end

function hyperjet_slow( context )
	local dx = RandomInt( 20, 400 )
	context:Control( Context.IDLE, Trigger_DX(dx) )
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )
	
	local params = context:CreateObjectParams()
	params.ai = "hyperjet_stop"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "hyperjet_stop", params, true )	
end

function hyperjet_stop( context )
	context:Control( Context.IDLE, Trigger_Timer(2) )
	
	local params = context:CreateObjectParams()
	params.ai = "hyperjet_fast"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "hyperjet_fast", params, true )	
end

function hyperjet_fast( context )
	context:Control( Context.CTRL_SPEED_UP, Trigger_Eternal() )
end

function hyperjet_dead1( context )
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )
	context:Control( Context.CTRL_VERT_UP, Trigger_Timer(0.2) )
	
	local params = context:CreateObjectParams()
	params.ai = "hyperjet_dead2"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "hyperjet_dead2", params, true )			
end

function hyperjet_dead2( context )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_Eternal() )
end

function paranoia_dead( context )
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_Eternal() )
end

function shuttle( context )
	-- 0 - initial
	-- 1 - down
	-- 2 - up
	-- 3 - side
	-- 4 - flip
	-- 5 - create satellite
	
	local nextAction = 0
	local y = context:GetSelfY()

	if y < 10 then
		nextAction = 0
	else
		local rnd = RandomInt( 0, 100 )
		if ( rnd < 50 ) then
			nextAction = 3
		elseif ( rnd < 60 ) then
			nextAction = 2
		elseif ( rnd < 70 ) then
			if y < 500 then
				nextAction = 1
			else
				nextAction = 3
			end
		elseif ( rnd < 85 ) then
			nextAction = 4
		else
			nextAction = 5
		end
	end

	if nextAction == 0 then
		local dy = RandomInt( 150, 200 )
		context:Control( Context.CTRL_VERT_DOWN, Trigger_DY(dy) )	
	elseif nextAction == 1 then
		local dy = RandomInt( 50, 100 )
		context:Control( Context.CTRL_VERT_DOWN, Trigger_DY(dy) )
	elseif nextAction == 2 then
		local dy = RandomInt( 50, 100 )
		context:Control( Context.CTRL_VERT_UP, Trigger_DY(dy) )
	elseif nextAction == 3 then
		local time = RandomFloat( 1, 3 )
		context:Control( Context.IDLE, Trigger_Timer( time ) )	
	elseif nextAction == 4 then
		context:Control( Context.CTRL_STOP, Trigger_OneTime() )
		context:Control( Context.CTRL_FLIP, Trigger_OneTime() )
		context:Control( Context.IDLE, Trigger_Timer( 1 ) )
		context:Control( Context.CTRL_GO, Trigger_OneTime() )
		local time = RandomFloat( 1, 3 )
		context:Control( Context.IDLE, Trigger_Timer( time ) )
	elseif nextAction == 5 then
		local params = context:CreateObjectParams()
		params.ai = "satellite_deploy"
		params.hasSpeed = true
		params.speedX = 0
		params.speedY = -75
		params.angle = 0
		context:CreateObject( "satellite_deploy", params, false )
		local time = RandomFloat( 0.5, 1.5 )
		context:Control( Context.IDLE, Trigger_Timer( time ) )
	end
end

function shuttle_dead( context )
	context:Control( Context.CTRL_GO, Trigger_OneTime() )
	context:Control( Context.CTRL_VERT_DOWN, Trigger_Eternal() )
	end

function satellite_deploy( context )		
	context:Control( Context.CTRL_STOP, Trigger_OneTime() )	
	context:Control( Context.CTRL_VERT_UP, Trigger_DY(30) )	
	
	local params = context:CreateObjectParams()
	params.ai = "satellite"
	params.hasSpeed = true
	params.speedX = 0
	params.speedY = 0
	params.angle = 0
	context:CreateObject( "satellite", params, true )	
end

function satellite( context )
	
	-- 1 - flip
	-- 2 - engage vert_down
	-- 3 - engage vert_up
	-- 4 - issue stop
	-- 5 - issue go
	-- 6 - idle

	local nextAction = RandomInt( 0, 100 )
	
	if nextAction < 25 then
		context:Control( Context.CTRL_FLIP, Trigger_OneTime() )
		context:Control( Context.IDLE, Trigger_Timer(0.1) )
	elseif nextAction < 35 then
		local time = RandomFloat( 1, 3 )
		context:Control( Context.CTRL_VERT_DOWN, Trigger_Timer(time) )
	elseif nextAction < 45 then
		local time = RandomFloat( 1, 3 )
		context:Control( Context.CTRL_VERT_UP, Trigger_Timer(time) )	
	elseif nextAction < 55 then
		context:Control( Context.CTRL_STOP, Trigger_OneTime() )	
	elseif nextAction < 85 then
		context:Control( Context.CTRL_GO, Trigger_OneTime() )
	elseif nextAction < 100 then
		local time = RandomFloat( 1, 3 )
		context:Control( Context.IDLE, Trigger_Timer(time) )		
	end
end

function spacejet_dead( context )
    context:Control( Context.IDLE,				Trigger_DX( 15 )			)
    context:Control( Context.CTRL_STOP,			Trigger_OneTime()			)
    context:Control( Context.CTRL_VERT_DOWN,	Trigger_OneTime()	    	)
    while true do
		--context:Control( Context.CTRL_GO,	Trigger_Timer( 0.125 )	    	)
		context:Control( Context.CTRL_GO,	Trigger_OneTime()	    	)
		context:Control( Context.IDLE,		Trigger_Timer(0.125)	    	)
		context:Control( Context.CTRL_FLIP,	Trigger_OneTime()		    	)
	end
end
