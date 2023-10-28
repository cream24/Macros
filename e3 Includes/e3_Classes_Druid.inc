|----------------------------------------------------------------------------|
| Druid Functions
|----------------------------------------------------------------------------|
Sub DRU_Setup
	/call iniToVarV "${Character_Ini},Druid,Evac Spell" Evac_Spell string
/return
|----------------------------------------------------------------------------|
SUB DRU_MacroSettings
/if (${Debug}) /echo |- DRU_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_healChain" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Debuffs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_DoTs" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_AE" 1
	/call WriteToIni "${advSettings_Ini},DRU Functions,DRU Function" "check_Nukes" 1
/RETURN
|----------------------------------------------------------------------------|
SUB DRU_CharacterSettings
	/call WriteToIni "${Character_Ini},Druid,Evac Spell"
/RETURN
|----------------------------------------------------------------------------|
Sub DRU_Aliases
/return
|----------------------------------------------------------------------------|
#event AE_SOW "#*# tells you, 'AESOW'"
sub Event_AE_SOW(string line)
	/if (${Me.Class.ShortName.Equal[DRU]}) {
	 /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Spirit of the Wood]}) {
		 /bc MGB Spirit of the Wood inc...
		 /casting "Mass Group Buff|alt" -maxtries|3
		 /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
		 /delay 5
		 /casting "Spirit of the Wood|alt" -maxtries|3
		 /delay 5 !${Me.AltAbilityReady[Spirit of the Wood]}
		 /casting "Spirit of the Wood|alt" -maxtries|3
		 /rs MGB Spirit of the Wood inc...
	 } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
			/bc Mass Group Buff is not available...
	 } else /bc Spirit of the Wood is not available...
	}
/return

#event AE_FOE "#*# tells you, 'AEFOE'"
sub Event_AE_FOE(string line)
	/if (${Me.Class.ShortName.Equal[DRU]}) {
	 /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Bool[${Me.Book[Flight of Eagles]}]}) {
		 /bc MGB Flight of Eagles inc...
		 /casting "Mass Group Buff|alt" -maxtries|3
		 /delay 5 !${Me.AltAbilityReady[Mass Group Buff]}
		 /delay 5
		 /casting "Flight of Eagles" -maxtries|3
		 /delay 5 !${Me.SpellReady[Flight of Eagles]}
		 /casting "Flight of Eagles" -maxtries|3
		 /rs MGB Flight of Eagles inc...
	 } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
			/bc Mass Group Buff is not available...
	 } else /bc Flight of Eagles is not available...
	}
/return
|----------------------------------------------------------------------------|
Sub DRU_Background_Events
	/doevents AE_SOW
	/doevents AE_FOE
/return