|----------------------------------------------------------------------------|
| Paladin Functions
|----------------------------------------------------------------------------|
Sub PAL_Setup
/if (${Debug}) /echo |- PAL_Setup ==>
	/call iniToVarV "${Character_Ini},Paladin,Auto-Yaulp (On/Off)" AutoYaulp bool outer
	/call iniToVarV "${Character_Ini},Paladin,Yaulp Spell" yaulpSpell string outer
/if (${Debug}) /echo <== PAL_Setup -|
/return
|----------------------------------------------------------------------------|
SUB PAL_MacroSettings
/if (${Debug}) /echo |- PAL_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#1" "check_Tanking" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#2" "check_Yaulp" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#3" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#4" "check_Heals" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#5" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#6" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},PAL Functions,PAL Function#7" "check_Nukes" 1

/if (${Debug}) /echo <== PAL_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB PAL_CharacterSettings
/if (${Debug}) /echo |- PAL_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},Paladin,Auto-Yaulp (On/Off)" Off
	/call WriteToIni "${Character_Ini},Paladin,Yaulp Spell"

/if (${Debug}) /echo <== PAL_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
Sub PAL_Aliases
/return
|----------------------------------------------------------------------------|
Sub PAL_Background_Events
/return