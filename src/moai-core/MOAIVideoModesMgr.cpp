// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include <moai-core/MOAIVideoModesMgr.h>

//================================================================//
// lua
//================================================================//

std::vector<MOAIVideoModesMgr::Mode> MOAIVideoModesMgr::s_videoModes;

//----------------------------------------------------------------//
/**	@name	singletonHello
	@text	Prints the string 'MOAIVideoModesMgr singleton foo!' to the console.

	@out	nil
*/
int MOAIVideoModesMgr::_GetNumDisplayModes ( lua_State* L ) {

	MOAILuaState state ( L );
	lua_pushnumber ( L, s_videoModes.size() );

	return 1;
}

int MOAIVideoModesMgr::_GetDisplayMode ( lua_State* L ) {

	MOAILuaState state ( L );
	unsigned int mode = (unsigned int)state.GetValue ( 1, 0 );

	if(mode > s_videoModes.size())
		return 0;

	lua_pushnumber ( L, s_videoModes[mode].width );
	lua_pushnumber ( L, s_videoModes[mode].height );
	lua_pushnumber ( L, s_videoModes[mode].rate );

	return 3;
}

//================================================================//
// MOAIVideoModesMgr
//================================================================//

//----------------------------------------------------------------//
MOAIVideoModesMgr::MOAIVideoModesMgr () {

	// register all classes MOAIVideoModesMgr derives from
	// we need this for custom RTTI implementation
	RTTI_BEGIN
		RTTI_EXTEND ( MOAILuaObject )
		
		// and any other objects from multiple inheritance...
		// RTTI_EXTEND ( MOAIFooMgrBase )
	RTTI_END
}

//----------------------------------------------------------------//
MOAIVideoModesMgr::~MOAIVideoModesMgr () {
}

//----------------------------------------------------------------//
void MOAIVideoModesMgr::RegisterLuaClass ( MOAILuaState& state ) {

	// call any initializers for base classes here:
	// MOAIFooBase::RegisterLuaClass ( state );

	// also register constants:
	// state.SetField ( -1, "FOO_CONST", ( u32 )FOO_CONST );

	// here are the class methods:
	luaL_Reg regTable [] = {
		{ "GetNumDisplayModes", _GetNumDisplayModes },
		{ "GetDisplayMode", _GetDisplayMode },
		{ NULL, NULL }
	};

	luaL_register ( state, 0, regTable );
}

void MOAIVideoModesMgr::ClearDisplayModes()
{
	s_videoModes.clear();
}

void MOAIVideoModesMgr::AddDisplayMode(int width, int height, int rate)
{
	Mode m = {width, height, rate};
	s_videoModes.push_back(m);
}
