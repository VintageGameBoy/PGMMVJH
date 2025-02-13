


#include "jsapi.h"
#include "JavascriptManager.h"
#include "steam/steam_api.h"


#pragma region 方法定义

static bool register_agtk_steam(JSContext* cx, JS::HandleObject object);
// 检查是否steam客户端启动(可选)
static bool js_steam_SteamAPI_RestartAppIfNecessary(JSContext* cx, unsigned argc, JS::Value* vp);
// 初始化
static bool js_steam_SteamAPI_Init(JSContext* cx, unsigned argc, JS::Value* vp);
// 关闭游戏之前调用
static bool js_steam_SteamAPI_Shutdown(JSContext* cx, unsigned argc, JS::Value* vp);
// 保存状态 解锁或者重置成就之后调用
static bool js_steam_SteamUserStats_StoreStats(JSContext* cx, unsigned argc, JS::Value* vp);
// 获取用户名
static bool js_steam_SteamFriends_GetSteamUserName(JSContext* cx, unsigned argc, JS::Value* vp);
// 解锁一个成就
static bool js_steam_SteamUserStats_UnlockedAchievement(JSContext* cx, unsigned argc, JS::Value* vp);
// 重置成就
static bool js_steam_SteamUserStats_ResetAchievement(JSContext* cx, unsigned argc, JS::Value* vp);
// 获取所有成就信息
static bool js_steam_SteamUserStats_GetAchievements(JSContext* cx, unsigned argc, JS::Value* vp);
// 重置所有统计信息 (false 仅重置所有统计) (true 重置所有统计和成就)
static bool js_steam_SteamUserStats_ResetAllStats(JSContext* cx, unsigned argc, JS::Value* vp);
// 获取一个成就的状态
static bool js_steam_SteamUserStats_GetAchievementState(JSContext* cx, unsigned argc, JS::Value* vp);

#pragma endregion


#pragma region 初始化

void jsb_register_agtk_steam(JSContext* cx, JS::HandleObject object)
{
	auto ret = register_agtk_steam(cx, object);
	CCASSERT(ret, "Failed to jsb_register_agtk_steam");
}

bool register_agtk_steam(JSContext* cx, JS::HandleObject object)
{
	JS::RootedObject robj(cx, object);

	static JSClass steam_class = {
		"steam",
		0,
		/* All of these can be replaced with the corresponding JS_*Stub
		function pointers. */
		JS_PropertyStub,
		JS_DeletePropertyStub,
		JS_PropertyStub,
		JS_StrictPropertyStub,
		JS_EnumerateStub,
		JS_ResolveStub,
		JS_ConvertStub,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	auto steam = JS_DefineObject(cx, robj, "steam", &steam_class, JS::NullPtr(), JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE);
	CCASSERT(steam, "steam should not be nullptr");
	JS::RootedObject rsteam(cx, steam);
	static JSFunctionSpec steam_methods[] = {
		{ "steamapi_restarappifnecessary", js_steam_SteamAPI_RestartAppIfNecessary, 0, 0, 0 },
		{ "steamapi_init", js_steam_SteamAPI_Init, 0, 0, 0 },
		{ "steamapi_shutdown", js_steam_SteamAPI_Shutdown, 0, 0, 0 },
		{ "steamuserstats_storestats", js_steam_SteamUserStats_StoreStats, 0, 0, 0 },
		{ "steamfriends_getsteamusername", js_steam_SteamFriends_GetSteamUserName, 0, 0, 0 },
		{ "steamuserstats_unlockedachievement", js_steam_SteamUserStats_UnlockedAchievement, 0, 0, 0 },
		{ "steamuserstats_resetachievement", js_steam_SteamUserStats_ResetAchievement, 0, 0, 0 },
		{ "steamuserstats_getachievements", js_steam_SteamUserStats_GetAchievements, 0, 0, 0 },
		{ "steamuserstats_resetallstats", js_steam_SteamUserStats_ResetAllStats, 0, 0, 0 },
		{ "steamuserstats_getachievementstate", js_steam_SteamUserStats_GetAchievementState, 0, 0, 0 },
		{ NULL },
	};
	JS_DefineFunctions(cx, rsteam, steam_methods);

	JS::RootedValue rval(cx);
	rval.setObject(*steam);
	JS_SetProperty(cx, robj, "steam", rval);
	return true;
}
#pragma endregion


#pragma region steam初始化方法

bool js_steam_SteamAPI_RestartAppIfNecessary(JSContext* cx, unsigned argc, JS::Value* vp) {
	//检查参数，然后调用steamapi做初始化
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	if (len < 1) {
		args.rval().set(JS::NullValue());
		return false;
	}
	// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	auto& v = args[0];
#else
#endif
	if (!v.isNumber()) { //检查steam_appId
		args.rval().set(JS::NullValue());
		return false;
	}
	auto appleId = JavascriptManager::getInt32(v);
	bool restartNeeded = SteamAPI_RestartAppIfNecessary(appleId);
	args.rval().set(JS::BooleanValue(restartNeeded));
	return true;
}


bool js_steam_SteamAPI_Init(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	bool restartNeeded = SteamAPI_Init();
	args.rval().set(JS::BooleanValue(restartNeeded));
	return true;
}

bool js_steam_SteamAPI_Shutdown(JSContext* cx, unsigned argc, JS::Value* vp) {
	//JS::CallArgs args = CallArgsFromVp(argc, vp);
	SteamAPI_Shutdown();
	return true;
}

bool js_steam_SteamUserStats_StoreStats(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	bool storeStatsed = SteamUserStats()->StoreStats();
	args.rval().set(JS::BooleanValue(storeStatsed));
	return true;
}

bool js_steam_SteamFriends_GetSteamUserName(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if (SteamFriends() != NULL) {
		const char* name = SteamFriends()->GetPersonaName();
		JS::Rooted<JSString*> jsName(cx, JS_NewStringCopyZ(cx, name));
		if (jsName)
			args.rval().set(JS::StringValue(jsName));
		else
			args.rval().set(JS::NullValue());
	}
	else
		args.rval().set(JS::NullValue());
	return true;
}

bool js_steam_SteamUserStats_UnlockedAchievement(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if (argc < 1 || !args[0].isString()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS::Rooted<JSString*> rootedStr(cx, args[0].toString());
	char* achievementName = JS_EncodeStringToUTF8(cx, rootedStr);
	if (achievementName == nullptr) {
		args.rval().set(JS::NullValue());
		return false;
	}
	bool result = SteamUserStats()->SetAchievement(achievementName);
	JS_free(cx, achievementName);
	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_steam_SteamUserStats_ResetAchievement(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if (argc < 1 || !args[0].isString()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS::Rooted<JSString*> rootedStr(cx, args[0].toString());
	char* achievementName = JS_EncodeStringToUTF8(cx, rootedStr);
	if (achievementName == nullptr) {
		args.rval().set(JS::NullValue());
		return false;
	}
	bool result = SteamUserStats()->ClearAchievement(achievementName);
	JS_free(cx, achievementName);
	args.rval().set(JS::BooleanValue(result));
	return true;
}

bool js_steam_SteamUserStats_GetAchievements(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	int numAchievements = SteamUserStats()->GetNumAchievements();
	JS::RootedObject achievementArray(cx, JS_NewArrayObject(cx, 0));
	if (!achievementArray) {
		args.rval().setNull();
		return false;
	}

	for (int i = 0; i < numAchievements; ++i) {
		const char* achievementName = SteamUserStats()->GetAchievementName(i);
		if (!achievementName) continue;

		bool isUnlocked = false;
		SteamUserStats()->GetAchievement(achievementName, &isUnlocked);

		JS::RootedObject achievementObject(cx, JS_NewObject(cx, nullptr,JS::NullPtr(), JS::NullPtr()));
		if (!achievementObject) {
			JS_ReportOutOfMemory(cx);
			return false;
		}

		JS::RootedValue nameVal(cx);
		JSString* str = JS_NewStringCopyZ(cx, achievementName);
		if (!str) return false;
		nameVal.setString(str);
		JS::RootedValue unlockedVal(cx, JS::BooleanValue(isUnlocked));
		if (!JS_SetProperty(cx, achievementObject, "name", nameVal) ||
			!JS_SetProperty(cx, achievementObject, "unlocked", unlockedVal)) {
			return false;
		}
		uint32_t index;
		if (!JS_GetArrayLength(cx, achievementArray, &index)) {
			return false;
		}
		JS::RootedValue elementValue(cx, JS::ObjectValue(*achievementObject));
		if (!JS_SetElement(cx, achievementArray, index, elementValue)) {
			return false;
		}
	}

	args.rval().setObject(*achievementArray);
	return true;
}

bool js_steam_SteamUserStats_ResetAllStats(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	auto len = args.length();
	// 检查参数数量
	if (len < 1|| !args[0].isBoolean()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	bool resetAchievements = args[0].toBoolean();
	bool result = SteamUserStats()->ResetAllStats(resetAchievements);
	args.rval().setBoolean(result);
	return true;
}

bool js_steam_SteamUserStats_GetAchievementState(JSContext* cx, unsigned argc, JS::Value* vp) {
	JS::CallArgs args = CallArgsFromVp(argc, vp);
	if (argc < 1 || !args[0].isString()) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS::Rooted<JSString*> rootedStr(cx, args[0].toString());
	char* achievementName = JS_EncodeStringToUTF8(cx, rootedStr);
	if (achievementName == nullptr) {
		args.rval().set(JS::NullValue());
		return false;
	}
	bool result = false;
	bool success = SteamUserStats()->GetAchievement(achievementName, &result);
	JS_free(cx, achievementName);
	if (!success) {
		args.rval().set(JS::NullValue());
		return false;
	}
	JS_free(cx, achievementName);
	args.rval().set(JS::BooleanValue(result));
	return true;
}


#pragma endregion