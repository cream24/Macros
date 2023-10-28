|----------------------------------------------------------------------------|
| Rogue Functions
|----------------------------------------------------------------------------|
Sub check_rogStrike
/if (${Debug}) /echo |- rogueStrike ==>	
	/if (${Me.Endurance} > ${strikeDiscEndCost} && ${Me.PctEndurance} >= ${strikeDiscMinEnd} && ${Me.CombatAbilityReady[${strikeDisc}]} && ${Me.AbilityReady[Backstab]} && !${Me.ActiveDisc.ID} && ${Me.Invis} && ${Me.Sneaking}) {
		| Use 'Assassin's Strike' type disc.
		/delay 1
		/disc ${strikeDisc}
		/delay 5 ${Bool[${Me.ActiveDisc.ID}]}
		/delay 3
		/doability Backstab
		/delay 1
		/attack on
	}
/if (${Debug}) /echo <== rogueStrike -|
/return
|----------------------------------------------------------------------------|
sub AutoHide
/if (${Debug}) /echo |- AutoHide ==>
	| Engage sneak.
	/if (!${Me.Sneaking} && ${Me.AbilityReady[Sneak]}) {
		/doability Sneak
		/delay 1s ${Me.Sneaking}
	}
	| Engage hide.
	/if (${Me.Sneaking} && ${Me.AbilityReady[Hide]}) {
		/doability Hide
		/delay 1s ${Me.Invis}
	}
/if (${Debug}) /echo <== AutoHide -|
/return
|----------------------------------------------------------------------------|
sub AutoEvade
/if (${Debug}) /echo |- AutoEvade ==>
  |only use evade over evadePctAggro aggro, but not when using a disc
	/if (${Me.PctAggro} > ${evadePctAggro} && ${Me.AbilityReady[Hide]} && !${Bool[${Me.ActiveDisc.ID}]}) {
		/attack off
		/delay 1s !${Me.Combat}
		/delay 5
		/doability Hide
		/delay 5 ${Me.Invis}
		/attack on
	}
/if (${Debug}) /echo <== AutoEvade -|
/return
|----------------------------------------------------------------------------|
Sub ROG_Setup
/if (${Debug}) /echo |- ROG_Setup ==>
	/call iniToVarV "${Character_Ini},Rogue,Auto-Hide (On/Off)" AutoHide bool
	/call iniToVarV "${Character_Ini},Rogue,Auto-Evade (On/Off)" AutoEvade bool
  /call iniToVarV "${Character_Ini},Rogue,Evade PctAggro" evadePctAggro int
  /call iniToVarV "${Character_Ini},Rogue,PoisonPR" poisonPR string
  /call iniToVarV "${Character_Ini},Rogue,PoisonFR" poisonFR string
  /call iniToVarV "${Character_Ini},Rogue,PoisonCR" poisonCR string
	| Assassin Strikes
	/call iniToVarV "${Character_Ini},Rogue,Sneak Attack Discipline" strikeDiscStr string
	/if (${Defined[strikeDiscStr]}) {
		/declare strikeDisc string outer ${strikeDiscStr.Arg[1,/]}
		/declare strikeDiscEndCost  string outer ${Spell[${strikeDiscStr}].EnduranceCost}
		/declare strikeDiscMinEnd	int outer 0
		/if (${strikeDiscStr.Find[/MinEnd]}) {
			/call argueString MinEnd "${strikeDiscStr}"
			/varset strikeDiscMinEnd ${c_argueString}
		}
		/if (${Defined[strikeDisc]} && !${AutoHide}) {
			/docommand ${ChatToggle} I cannot use ${strikeDisc} without AutoHide On
		}
	}
/if (${Debug}) /echo <== ROG_Setup -|
/return
|----------------------------------------------------------------------------|
SUB ROG_MacroSettings
	/if (${Debug}) /echo |- ROG_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function" "check_Buffs" 1
	/if (${Debug}) /echo <== ROG_MacroSettings -|
/RETURN
|----------------------------------------------------------------------------|
SUB ROG_CharacterSettings
	/if (${Debug}) /echo |- ROG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Hide (On/Off)" Off
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Evade (On/Off)" On
	/call WriteToIni "${Character_Ini},${Me.Class},Evade PctAggro" 75
	/call WriteToIni "${Character_Ini},${Me.Class},Sneak Attack Discipline"
	/call WriteToIni "${Character_Ini},${Me.Class},PoisonPR" "Bite of the Shissar XII"
	/call WriteToIni "${Character_Ini},${Me.Class},PoisonFR" "Solusek's Burn XII"
	/call WriteToIni "${Character_Ini},${Me.Class},PoisonCR" "E`ci's Lament XII"
	/if (${Debug}) /echo <== ROG_CharacterSettings -|
/RETURN
|----------------------------------------------------------------------------|
Sub ROG_Aliases
/return
|----------------------------------------------------------------------------|
Sub ROG_Background_Events
	/if (${AutoHide} && !${Me.Invis} && !${Me.Moving} && ${Me.CombatState.NotEqual[COMBAT]}) /call AutoHide
/return