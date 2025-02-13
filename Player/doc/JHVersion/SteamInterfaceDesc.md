
# Steam API 集成调用文档

本文档介绍了如何在JavaScript中调用Steam API。所有的Steam API方法都通过`Agtk.steam`对象进行访问，具体调用实例和参数说明如下。
具体的内容需要去看 [Steamworks API 概览](https://partner.steamgames.com/doc/sdk/api?l=schinese#initialization_and_shutdown)

---

## 方法调用

### `Agtk.steam.steamapi_restarappifnecessary(steamAppId)`

此方法用于检查Steam应用是否需要重新启动。

**参数**:
- `steamAppId` (Number): Steam应用的ID。

**返回值**:
- `Boolean`: 如果需要重新启动，返回`true`，否则返回`false`。

**调用示例**:
```javascript
var steamAppId = 123456;  // 替换为实际的Steam App ID
var result = Agtk.steam.steamapi_restarappifnecessary(steamAppId);
Agtk.log(result);  // 输出: true 或 false
```

---

### `Agtk.steam.steamapi_init()`

此方法用于初始化Steam API。

**参数**:
- 无

**返回值**:
- `Boolean`: 返回`true`表示初始化成功，`false`表示初始化失败。

**调用示例**:
```javascript
var result = Agtk.steam.steamapi_init();
Agtk.log("[STEAM]steam初始化情况：" + result);
```

---

### `Agtk.steam.steamapi_shutdown()`

此方法用于关闭Steam API。

**参数**:
- 无

**返回值**:
- 无

**调用示例**:
```javascript
Agtk.steam.steamapi_shutdown();
Agtk.log("Steam API 已关闭");
```

---

### `Agtk.steam.steamuserstats_storestats()`

此方法用于保存Steam用户的统计信息。
每次调用完成就解锁或者重置成就后需要调用这个。

**参数**:
- 无

**返回值**:
- `Boolean`: 返回`true`表示成功保存统计信息，`false`表示保存失败。

**调用示例**:
```javascript
var result = Agtk.steam.steamuserstats_storestats();
Agtk.log(result);  // 输出: true 或 false
```

---

### `Agtk.steam.steamfriends_getsteamusername()`

此方法用于获取当前Steam用户的用户名。

**参数**:
- 无

**返回值**:
- `String`: 当前Steam用户名。

**调用示例**:
```javascript
var username = Agtk.steam.steamfriends_getsteamusername();
Agtk.log("[STEAM]当前steam用户名:" + username);  // 输出: 当前Steam用户名
```

---

### `Agtk.steam.steamuserstats_getachievementstate(achievementName)`

检查一个成就是否已经解锁。

**参数**:
- `achievementName` (String): 要检查的成就名称。

**返回值**:
- `Boolean`: 返回`true`表示已解锁成就，`false`表示未解锁。
- 失败时：返回 null（例如 Steam 未初始化或用户未登录）。

**调用示例**:
```javascript
Agtk.log("[STEAM]准备获取成就状态");
var achievementName = "NEW_ACHIEVEMENT_1_3";
var achievementstate = Agtk.steam.steamuserstats_getachievementstate(achievementName);
Agtk.log("[STEAM]成就:" + achievementName + " 状态:" + achievementstate);
```

---

### `Agtk.steam.steamuserstats_unlockedachievement(achievementName)`

此方法用于解锁指定的Steam成就。

**参数**:
- `achievementName` (String): 要解锁的成就名称。

**返回值**:
- `Boolean`: 返回`true`表示成功解锁成就，`false`表示解锁失败。

**调用示例**:
```javascript
Agtk.log("[STEAM]准备解锁成就");
var achievementName = "NEW_ACHIEVEMENT_1_3";
var unlockResult = Agtk.steam.steamuserstats_unlockedachievement(achievementName);
if(unlockResult && Agtk.steam.steamuserstats_storestats()){
  Agtk.log("[STEAM]解锁成就" + achievementName + "成功");
}else{
  Agtk.log("[STEAM]解锁成就" + achievementName + "失败");
}
```

---

### `Agtk.steam.steamuserstats_resetachievement(achievementName)`

此方法用于重置指定的Steam成就。

**参数**:
- `achievementName` (String): 要重置的成就名称。

**返回值**:
- `Boolean`: 返回`true`表示成功重置成就，`false`表示重置失败。

**调用示例**:
```javascript
Agtk.log("[STEAM]准备重置成就");
var achievementName = "NEW_ACHIEVEMENT_1_3";
var unlockResult = Agtk.steam.steamuserstats_resetachievement(achievementName);
if(unlockResult && Agtk.steam.steamuserstats_storestats()){
  Agtk.log("[STEAM]重置成就" + achievementName + "成功");
}else{
  Agtk.log("[STEAM]重置成就" + achievementName + "失败");
}
```

---

### `Agtk.steam.steamuserstats_getachievements()`

获取所有的成就数组。

**参数**:
- 无

**返回值**:
-成功时：返回一个成就数组，每个成就对象的结构如下：

```typescript
interface Achievement {
  name: string;      // 成就名称
  unlocked: boolean; // 成就是否已解锁
}
```
失败时：返回 null（例如 Steam 未初始化或用户未登录）。

**调用示例**:
```javascript
Agtk.log("[STEAM]尝试获取所有成就信息");
var steamAchievements = Agtk.steam.steamuserstats_getachievements();

if (steamAchievements) {
  Agtk.log("[STEAM]当前成就数量"+steamAchievements.length);
  for (i=0; i < steamAchievements.length; i++) {
    Agtk.log("[STEAM]Achievement: " + steamAchievements[i].name + "Status:" + steamAchievements[i].unlocked);
  }
}
```

---

### `Agtk.steam.steamuserstats_resetallstats()`

重置所有用户信息

**参数**:
- `resetAchievements` (Boolean) : 是否同时重置用户的成就。
- - 如果为 true，则重置所有统计数据 并 重置成就。
- - 如果为 false，则仅重置统计数据，成就保持不变。

**返回值**:
- `Boolean`: 返回`true`表示成功重置，`false`表示重置失败。

**调用示例**:
```javascript
Agtk.log("[STEAM]尝试获取所有成就信息");
var resetallstatsResult = Agtk.steam.steamuserstats_resetallstats(true);
Agtk.log("[STEAM]重置所有成就信息：" + resetallstatsResult);
```

---

## 总结

以上方法通过`Agtk.steam`对象进行调用，可以轻松地实现Steam API的各项功能。调用时，请确保传入正确的参数，并根据返回值判断操作是否成功。