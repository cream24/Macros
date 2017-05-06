|----------------------------------------------------------------------------|
|- e3_Classes_Rogue.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
|--------------------------------------------------------------------------------|
|- Attempts to arm 'Assassin's Strike' type disciplines before using backstab.	-|
|--------------------------------------------------------------------------------|
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

|------------------------------------------------------------|
|- AutoHide													-|
|------------------------------------------------------------|
|- Automatically engages sneak and hide when not in combat.	-|
|------------------------------------------------------------|
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

|------------------------------------------------|
|- AutoEvade									-|
|------------------------------------------------|
|- Automatically engages hide while in combat.	-|
|------------------------------------------------|
sub AutoEvade
/if (${Debug}) /echo |- AutoEvade ==>
|only use evade over 80% aggro, and only if im using a disc (burning)
	/if (${Me.PctAggro} > 60 && ${Me.AbilityReady[Hide]} && !${Bool[${Me.ActiveDisc.ID}]}) {
		/attack off
		/delay 1s !${Me.Combat}	
		/doability Hide
		/delay 5 ${Me.Invis}
		/attack on
	}
/if (${Debug}) /echo <== AutoEvade -|
/return


Sub ROG_Setup
/if (${Debug}) /echo |- ROG_Setup ==>
  /declare strikeTimer timer outer
	/call IniToVar "${Character_Ini},Rogue,Auto-Hide (On/Off)" AutoHide bool outer
	/call IniToVar "${Character_Ini},Rogue,Auto-Evade (On/Off)" AutoEvade bool outer
	| Assassin Strikes
	/call IniToVar "${Character_Ini},Rogue,Sneak Attack Discipline" strikeDiscStr string outer
	/declare strikeDisc 		string outer ${strikeDiscStr.Arg[1,/]}
	/declare strikeDiscEndCost  string outer ${Spell[${strikeDiscStr}].EnduranceCost}	
	/declare strikeDiscMinEnd	int outer 0

  /if (${Bool[${strikeDisc}]} && !${AutoHide}) /docommand ${ChatToggle} I cannot use ${strikeDisc} without AutoHide On

  /if (${strikeDiscStr.Find[/MinEnd]}) {
    /call argueString MinEnd "${strikeDiscStr}"
    /varset strikeDiscMinEnd ${Macro.Return}
	}
/if (${Debug}) /echo <== ROG_Setup -|
/return

SUB ROG_MacroSettings
/if (${Debug}) /echo |- ROG_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function" "check_Buffs" 1
/if (${Debug}) /echo <== ROG_MacroSettings -|
/RETURN

SUB ROG_CharacterSettings
/if (${Debug}) /echo |- ROG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Hide (On/Off)" Off
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Evade (On/Off)" On
	/call WriteToIni "${Character_Ini},${Me.Class},Evade PctAggro" 75
	/call WriteToIni "${Character_Ini},${Me.Class},Sneak Attack Discipline"
/if (${Debug}) /echo <== ROG_CharacterSettings -|
/RETURN

Sub ROG_Aliases
/return

Sub ROG_Background_Events
	/if (${AutoHide} && !${Me.Invis} && !${Me.Moving} && ${Me.CombatState.NotEqual[COMBAT]}) /call AutoHide
/return