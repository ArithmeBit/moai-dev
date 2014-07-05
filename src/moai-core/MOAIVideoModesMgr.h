// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#ifndef MOAIVIDEOMODESMGR_H
#define MOAIVIDEOMODESMGR_H

#include <moai-core/headers.h>
#include <vector>

//================================================================//
// MOAIVideoModesMgr
//================================================================//
/**	@name	MOAIVideoModesMgr
	@text	Example singleton for extending Moai using MOAILuaObject.
			Copy this object, rename it and add your own stuff.
			Just don't forget to register it with the runtime
			using the REGISTER_LUA_CLASS macro (see moaicore.cpp).
*/
class MOAIVideoModesMgr :
	public MOAIGlobalClass < MOAIVideoModesMgr, MOAILuaObject > {
private:

	struct Mode
	{
		int width;
		int height;
		int rate;
		// todo: add more
	};
	
	//----------------------------------------------------------------//
	static int		_GetNumDisplayModes		( lua_State* L );
	static int		_GetDisplayMode			( lua_State* L );
	static std::vector<Mode> s_videoModes;

public:
	
	DECL_LUA_SINGLETON ( MOAIVideoModesMgr )

	//----------------------------------------------------------------//
					MOAIVideoModesMgr			();
					~MOAIVideoModesMgr			();
	void			RegisterLuaClass	( MOAILuaState& state );
	void 			ClearDisplayModes();
	void 			AddDisplayMode(int width, int height, int rate);
};

#endif
