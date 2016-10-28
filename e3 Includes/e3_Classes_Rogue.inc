|----------------------------------------------------------------------------|
|- e3_Classes_Rogue.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
|--------------------------------------------------------------------------------|
|- Attempts to arm 'Assassin's Strike' type disciplines before using backstab.	-|
|--------------------------------------------------------------------------------|
Sub check_rogStrike
/if (${Debug}) /echo |- rogueStrike ==>	
	/if (${Assisting} && ${Me.Endurance} > ${strikeDiscEndCost} && ${Me.PctEndurance} >= ${strikeDiscMinEnd} && ${Me.CombatAbilityReady[${strikeDisc}]} && ${Me.AbilityReady[Backstab]} && ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[No Effect]}) {
		| Turn off combat.
		/if (${Me.Combat}) /attack off
		/if (${Stick.Active}) /squelch /stick pause			
		
		/if (!${Me.Sneaking}) {
			/if (${Me.AbilityReady[Sneak]}) {
				/doability Sneak
			} else {
				/goto :skipStrike
			}
		}
		/delay 5 ${Me.Sneaking}
		/if (!${Me.Invis}) {
			/if (${Me.AbilityReady[Hide]}) {
				/doability Hide
			} else {
				/goto :skipStrike
			}
		}	
		/delay 5 ${Me.Invis}
		| Use 'Assassin's Strike' type disc.
		:waitForDisc
		/delay 1
		/if (!${Me.Sneaking} || !${Me.Invis}) /goto :skipStrike
		/disc ${strikeDisc}
		/delay 5 ${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.NotEqual[No Effect]}
		/if (${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[No Effect]} && ${Me.CombatAbilityReady[${strikeDisc}]}) /goto :waitForDisc
		/delay 2
		/doability Backstab

		/if (!${Me.CombatAbilityReady[${strikeDisc}]}) /varset strikeTimer 300
		:skipStrike
		/attack on
		/squelch /stick unpause
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
		/delay 2s ${Me.Sneaking}
	}
	| Engage hide.
	/if (${Me.Sneaking} && ${Me.AbilityReady[Hide]}) {
		/doability Hide
		/delay 2s ${Me.Invis}
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
|only evade when strike is on cooldown with more than 8 sec left, otherwise it burns hide cd
  |/echo razorTimer ${razorTimer}
  |/echo h ${Me.AbilityReady[Hide]} d !${Bool[${Me.CombatAbilityReady[${strikeDisc}]}]}
	/if (${Me.AbilityReady[Hide]} && (!${Bool[${strikeDisc}]} || ${strikeTimer}>80 || ${Me.Endurance} < ${strikeDiscEndCost} || ${Me.PctEndurance} < ${strikeDiscMinEnd} || !${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text.Equal[No Effect]})) {
		| Turn off combat.
		/attack off
		/delay 1s !${Me.Combat}	
		/doability Hide
		/delay 1s ${Me.Invis}		
		| Turn combat back on.
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

	/if (${strikeDiscStr.Find[/MinEnd]}) {
		/call argueString MinEnd "${strikeDiscStr}"
		/varset strikeDiscMinEnd ${Macro.Return}
	}
/if (${Debug}) /echo <== ROG_Setup -|
/return

SUB ROG_MacroSettings
/if (${Debug}) /echo |- ROG_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},ROG Functions,ROG Function#2" "check_Buffs" 1
/if (${Debug}) /echo <== ROG_MacroSettings -|
/RETURN

SUB ROG_CharacterSettings
/if (${Debug}) /echo |- ROG_CharacterSettings ==>
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Hide (On/Off)" Off
	/call WriteToIni "${Character_Ini},${Me.Class},Auto-Evade (On/Off)" On
	/call WriteToIni "${Character_Ini},${Me.Class},Sneak Attack Discipline"
/if (${Debug}) /echo <== ROG_CharacterSettings -|
/RETURN

Sub ROG_Aliases
/return

Sub ROG_Background_Events
	/if (${AutoHide} && !${Me.Invis} && !${Me.Moving} && ${NetBots[${Me}].CombatState} > 0) /call AutoHide
/return