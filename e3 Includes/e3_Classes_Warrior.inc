|----------------------------------------------------------------------------|
| Warrior Functions
|----------------------------------------------------------------------------|
Sub WAR_Setup
 /if (${Me.Name.Equal[Lush]}) /log on
/return
|----------------------------------------------------------------------------|
SUB WAR_MacroSettings
/if (${Debug}) /echo |- WAR_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},WAR Functions,WAR Function" "check_Tanking" 1
	/call WriteToIni "${advSettings_Ini},WAR Functions,WAR Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},WAR Functions,WAR Function" "check_Buffs" 1
/if (${Debug}) /echo <== WAR_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB WAR_CharacterSettings
/RETURN
|----------------------------------------------------------------------------|
Sub WAR_Aliases
/return
|----------------------------------------------------------------------------|
Sub WAR_Background_Events
/return
