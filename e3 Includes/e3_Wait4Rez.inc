|----------------------------------------------------------------------------|
|- e3_Macro_Wait4Rez.inc v1.4 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
|- Based off Wait4Rez.inc by A_Druid_00 (originally written by fantum409)	-|
|----------------------------------------------------------------------------|
|- Accepts rezzes and loots corpses after being killed.						-|
|----------------------------------------------------------------------------|
|- Use,	[Wait4Rez] in /t or /echo to manually call Wait4Rez.				-|
|- Use, [Loot Now] in /t or /echo to manually skip rez acceptance,			-|
|- and begin looting.														-|
|----------------------------------------------------------------------------|

|------------------------------------------------|
|- Automatically engages Wait4Rez when you die.	-|
|------------------------------------------------|
#EVENT dead	"You have been slain by #*#"
#EVENT dead	"You died."
SUB EVENT_dead
/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Event_Dead ==>
	/if (!${Defined[use_Wait4Rez]}) /call Wait4Rez_Setup
	/if (${use_Wait4Rez}) {
		| Alert master that a bot has died.
		/if (${Wait4RezAlert}) {
			/declare a int local
			/for a 1 to ${Wait4RezAlert}
				/beep
				/delay 1
			/next a
		}
		| Wait for a Rez.
		/docommand ${ChatToggle} I'm dead. Waiting for rez.
		/if (!${Defined[Waiting4Rez]}) /declare Waiting4Rez bool outer TRUE
		/call Wait4Rez
	}
	/call AssistOff
/if (${Debug} || ${Debug_Wait4Rez}) /echo <== Event_Dead -|
/RETURN

|--------------------------------------------|
|- Manually engages Wait4Rez on command.	-|
|--------------------------------------------|
#EVENT waitNow "#1# tells you, 'Wait4Rez'"
#EVENT waitNow "<#1#> Wait4Rez"
SUB EVENT_waitNow(line, ChatSender)
/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Event_WaitNow ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/if (!${Defined[use_Wait4Rez]}) /call Wait4Rez_Setup
		/if (${use_Wait4Rez}) {
      /consent ${ChatSender}
			|If I'm not already waiting, wait for a rez.
			/if (!${Waiting4Rez}) {
				/if (!${Defined[Waiting4Rez]}) /declare Waiting4Rez bool outer TRUE
				/call Wait4Rez
			}
		}
	}
/if (${Debug} || ${Debug_Wait4Rez}) /echo <== Event_WaitNow -|
/RETURN

|--------------------------------------------------------------------------------------------------------|
|- Manual bypasses for rez acceptance and zoning features, to loot corpses that are already close by.	  -|
|--------------------------------------------------------------------------------------------------------|
#EVENT lootNow "[#1#] Loot Now"
#EVENT lootNow "<#1#> Loot Now#*#"
#EVENT lootNow "#1# tells you, 'Loot Now#*#"
SUB EVENT_lootNow(line, ChatSender)
/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Event_lootNow ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/if (!${Defined[use_Wait4Rez]}) /call Wait4Rez_Setup
		/if (${use_Wait4Rez}) {
			/echo Looting....
			/call Wait4Rez LOOT_NOW
		}
	}
/if (${Debug} || ${Debug_Wait4Rez}) /echo <== Event_lootNow -|	
/RETURN

|------------------------------------------------------------------------------------|
|- Consents NetBots, loads spells, accepts rez, and loots your body after zoning.	-|
|------------------------------------------------------------------------------------|
SUB Wait4Rez(functionCall)
/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Wait4Rez${If[${Defined[functionCall]}, ${functionCall},]} ==>
	/declare i int local 0
	/if (${functionCall.Equal[LOOT_NOW]}) /goto :Looting
	/echo Awaiting rez: auto-accepting, and looting my corpse...
	|Consent NetBots.
	/for i 1 to ${NetBots.Counts}
	  /delay 2
		/consent ${NetBots.Client[${i}]}
	/next i
	|Load Spells.
	/if (${Me.Class.CanCast}) /call check_Gems
  |Wait for Rez------------------------------------------------------|
	:rezmeffs
	/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Wait4Rez -| Waiting to accept rez.
	/call Background_Events
	/delay 10 ${Window[ConfirmationDialogBox].Open}
	/if (!${Window[ConfirmationDialogBox].Open}) /goto :rezmeffs
	|Click yes to rez box.
	/nomodkey /notify ConfirmationDialogBox Yes_Button leftmouseup
	|Wait for zone-----------------------------------------------------|
	:zoneffs
	/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Wait4Rez -| Waiting to zone.
	/delay 300 ${Spawn[${Me}'s].ID}
	/if (!${Spawn[${Me}'s].ID}) /goto :zoneffs
	/delay 30
  |Loot corpse-------------------------------------------------------|
	/docommand ${ChatToggle} Looting...
	:Looting
	/if (${Debug} || ${Debug_Wait4Rez}) /echo |- Wait4Rez -| Attempting to loot my corpse.
	
	/if (!${Defined[lootTotal]}) /declare lootTotal int local 0
	/call TrueTarget ${Spawn[${Me}'s].ID}
	/if (${Target.Distance}<100 && ${Target.CleanName.Equal[${Me}'s corpse]}) {
	|Pull the corpse.
		/if (${Target.Distance}>15) /corpse
		/delay 20 ${Target.Distance}<15
		/loot
		/delay 20 ${Me.State.Equal[BIND]}
		/if (${Me.State.NotEqual[BIND]}) /goto :Looting
	|Wait for all the items on your corpse to appear in your loot window.
		/varset lootTotal -1
		/varset i 0
		:autoxplootlag
		/varcalc i ${i}+1
		/if (${lootTotal}!=${Corpse.Items}) {
			/varset lootTotal ${Corpse.Items}
			/delay 5
			/if (${i}<50) /goto :autoxplootlag
		}
	|Loot All Items.
		/notify LootWnd LootAllButton leftmouseup
		/delay 30s !${Window[LootWnd].Open}
	|Check you don't still have a corpse.
		/if (!${Me.State.Equal[BIND]}) {
			/delay 2s !${Spawn[${Me}'s].ID}
			/if (${Spawn[${Me}'s].ID}) /goto :Looting
		}
		/docommand ${ChatToggle} Ready to die again!
	}
	/doevents flush
/RETURN

|----------------------------------------------------------------|
|- Declares variables and settings used in Wait4Rez functions.	-|
|----------------------------------------------------------------|
SUB Wait4Rez_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Wait4Rez (On/Off)" Debug_Wait4Rez bool outer
	/call INItoVar "${genSettings_Ini},Wait4Rez,Wait4Rez (On/Off)" use_Wait4Rez bool outer
	/call INItoVar "${genSettings_Ini},Wait4Rez,Wait4Rez Alert (Beeps)" Wait4RezAlert int outer
	|/call INItoVar "${genSettings_Ini},Wait4Rez,Loot Bag Delay" BagDelay int outer
	|/call INItoVar "${genSettings_Ini},Wait4Rez,Loot Item Delay" ItemDelay int outer
/RETURN

SUB Wait4Rez_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug Wait4Rez (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Wait4Rez,Wait4Rez (On/Off)" On
	/call WriteToIni "${genSettings_Ini},Wait4Rez,Wait4Rez Alert (Beeps)" 1
/RETURN

SUB Wait4Rez_CharacterSettings
/RETURN

Sub Wait4Rez_Aliases
	/squelch /alias /wait4rez /bc Wait4Rez
	/squelch /alias /waitforrez /bc Wait4Rez
	/squelch /alias /lootCorpses /bc Loot Now
	/squelch /alias /dead /bc Wait4Rez
/return

 Sub Wait4Rez_Background_Events
   /doevents lootNow
   /doevents waitNow
   /doevents dead
 /return