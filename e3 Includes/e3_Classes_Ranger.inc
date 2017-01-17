|----------------------------------------------------------------------------|
|- e3_Classes_Ranger.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
Sub RNG_Setup
  /call IniToVar "${Character_Ini},${Me.Class},TargetAE Ranged (On/Off)" swapTgtAeRanged bool outer
  /if (${swapTgtAeRanged} && !${Ini[${Character_Ini},Swap Items,Ranged].Length}) {
    /bc TargetAE Ranged (On/Off) = On,  but I have no "Ranged=<item>" in [Swap Item]
    /beep
  }
/return

SUB RNG_MacroSettings
/if (${Debug}) /echo |- RNG_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_Cures" 1
  /call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},RNG Functions,RNG Function" "check_Buffs" 1
/if (${Debug}) /echo <== RNG_MacroSettings -|
/RETURN

SUB RNG_CharacterSettings
  /call WriteToIni "${Character_Ini},${Me.Class},TargetAE Ranged (On/Off)" Off
/RETURN

Sub RNG_Aliases
/return

#event AE_AOH "#*# tells you, 'AEAOH'"
sub Event_AE_AOH(string line)
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Auspice of the Hunter]}) {
      /bc MGB Auspice of the Hunter inc...
      /rs MGB Auspice of the Hunter inc...
      /casting "Mass Group Buff|alt" "-maxtries|3"
      /delay 5
      /casting "Auspice of the Hunter|alt" "-maxtries|3"
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Auspice of the Hunter is not available...
/return

Sub RNG_Background_Events
  /doevents AE_AOH
/return