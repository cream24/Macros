|----------------------------------------------------------------------------|
|- e3_Classes_Druid.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
Sub DRU_Setup
	/call IniToVar "${Character_Ini},Druid,Evac Spell" Evac_Spell string outer
/return

SUB DRU_MacroSettings
/if (${Debug}) /echo |- DRU_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#1" "check_healChain" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#2" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#3" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#4" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#5" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#6" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#7" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#8" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function#9" "check_Nukes" 1
/RETURN

SUB DRU_CharacterSettings
	/call WriteToIni "${Character_Ini},Druid,Evac Spell"
/RETURN

Sub DRU_Aliases
/return

#event AE_SOW "#*# tells you, 'AESOW'"
sub Event_AE_SOW(string line)
 /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Spirit of the Wood]}) {
		/bc MGB Spirit of the Wood inc...
		/rs MGB Spirit of the Wood inc...
		/casting "Mass Group Buff|alt" "-maxtries|3"
		/casting "Spirit of the Wood|alt" "-maxtries|3"
 } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
		/bc Mass Group Buff is not available...
 } else /bc Spirit of the Wood is not available...
/return

Sub DRU_Background_Events
	/doevents AE_SOW
/return