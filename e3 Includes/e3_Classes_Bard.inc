|------------------------------------------------------------------------|
|- e3_Classes_Bard.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											                      -|
|------------------------------------------------------------------------|
#event saveMelody "<#1#> Save Melody #2# #3#"
#event saveMelody "[MQ2] Save Melody #2# #3#"
SUB EVENT_saveMelody(line, ChatSender, melodyName, songsToSave)
/if (${Debug}) /echo |- EVENT_saveMelody ==>
	| If no ChatSender is defined, make one.
	/if (!${Defined[ChatSender]}) /declare ChatSender string local ${Me.CleanName}
	| If I'm the ChatSender.
	/if (${ChatSender.Equal[${Me.CleanName}]}) {
		/declare i int local
		| Check if the melody already exists, and should not be overwritten.
		/if (${Ini[${Character_Ini},${melodyName} Melody].Length}) {	
			/if (${Debug}) /echo |- EVENT_saveMelody -| Overwriting [${melodyName}] melody...
			/for i 1 to 7
				/if (${Ini[${Character_Ini},${melodyName} Melody,Song#${i}].Length}) /call WriteToIni "${Character_Ini},${melodyName} Melody,Song#${i}" *PLACEHOLDER 1
			/next i
		}
		/echo Saving [${melodyName}] melody...
		/varset i 1
		:check_nextSong

		| If the gem number is not recognized
		/if (!${Int[${songsToSave.Arg[${i}, ]}]} || ${Int[${songsToSave.Arg[${i}, ]}]} < 1 || ${Int[${songsToSave.Arg[${i}, ]}]} > 9) {
			/if (${songsToSave.Arg[${i}, ].NotEqual[-Overwrite]}) {
				/echo [${songsToSave.Arg[${i}, ]}] is not a valid gem number.  Please use the following syntax when saving melodies, "Save Melody [NAME] [# # # # #] (-Overwrite)".  Ex.  /echo Save Melody Raid 1 2 3 4 5 6 7 -Overwrite
			}
		} else {
			/echo Song#${i}=${Me.Gem[${songsToSave.Arg[${i}, ]}]}/gem|${songsToSave.Arg[${i}, ]}
			/call WriteToIni "${Character_Ini},${melodyName} Melody,Song#${i}" "${Me.Gem[${songsToSave.Arg[${i}, ]}]}/gem|${songsToSave.Arg[${i}, ]}" ${If[${Ini[${Character_Ini},${melodyName} Melody,Song#${i}].Length},1,0]}
		}
		/varcalc i ${i} + 1
		/if (${songsToSave.Arg[${i}, ].Length}) /goto :check_nextSong
	}
/if (${Debug}) /echo <== EVENT_saveMelody -|
/return

|------------------------------------------------------------------------|
#EVENT startMelody "<#1#> Melody #2#"
#EVENT startMelody "[#1#] Melody #2#"
#EVENT startMelody "#1# tells you, 'Melody #2#'"
SUB EVENT_startMelody(line, ChatSender, melodyName)
/if (${Debug}) /echo |- EVENT_startMelody ==>
	| -Verify the event
	/declare memGem int local
	/call verifyEvent "${ChatSender}" "${line}"
	/varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		/if (${melodyName.Find[ /]}) /varset melodyName ${melodyName.Arg[1, /]}
		/if (${Me.Class.ShortName.Equal[BRD]}) {			
			| If I don't have the melodyName listed in my ini.
			/if (!${Ini[${Character_Ini},${melodyName} Melody,Song#1].Length}) {				
				| If Stop, end, or pause were used, stop the melody.
				/if (${Select[${melodyName},Stop,End,Pause]}) {
					/varset playingMelody FALSE
					/varset songSet
					/docommand ${ChatToggle} Ending melody.
					/twist end
				} else {
					/docommand ${ChatToggle} [${melodyName}] is not a known melody.
				}
			} else {
				/declare songList string local
				/declare spellName string local
				/declare i int local
				| For 1 to 7 (maximum melody size to include selos, 6 songs played actively with some droppage)
				/for i 1 to 7
				/if (${Ini[${Character_Ini},${melodyName} Melody,Song#${i}].Length} && ${Ini[${Character_Ini},${melodyName} Melody,Song#${i}].NotEqual[PLACEHOLDER]}) {
					/varset spellName ${Ini[${Character_Ini},${melodyName} Melody,Song#${i}].Arg[1,/]}
					| Check that the song is in my book
					/if (!${Me.Book[${spellName}]}) {	
						/echo I don't have [${spellName}] in my spellbook, playing the melody without it.
					} else {
						| If the song is not memmed, use mq2Cast to mem
						/if (!${Me.Gem[${spellName}]}) {
							/call argueString gem| "${Ini[${Character_Ini},${melodyName} Melody,Song#${i}]}"
              /if (${Bool[${Macro.Return}]}) {
                /varset memGem ${Macro.Return}
              } else {
                /varset memGem ${DefaultGem}
              }
							/memorize "${spellName}" "${memGem}"
              /if (${Cast.Status.Find[M]}) /delay 3s !${Cast.Status.Find[M]}
						}
						| Update songList with the corresponding gem#
						/varset songList ${songList} ${Me.Gem[${spellName}]}
					}
					/next i
				}
				
				| If I have a songList, start singing the melody
				/if (${Bool[${songList}]}) {
					| Twist the newly built songList.
					/varset playingMelody TRUE
					/varset returnTwist FALSE
					/varset songSet ${songList}
					/docommand ${ChatToggle} Playing [${melodyName}] melody...
					/twist ${songSet}
				}
			}
		}
	}
/if (${Debug}) /echo <== EVENT_startMelody -|
/return

|------------------------------------|
|- Pauses MQ2Twist while casting.	-|
|------------------------------------|
SUB pauseTwist
/if (${Debug} || ${Debug_Casting}) /echo |- pauseTwist ==>
	/varset	returnTwist TRUE
	/squelch /twist stop
	/delay 20 !${Me.Casting.ID}
/if (${Debug} || ${Debug_Casting}) /echo <== pauseTwist -|
/return

|------------------------------------|
|- Unpauses MQ2Twist after casting.	-|
|------------------------------------|
SUB unpauseTwist
	/delay 20 !${Me.Casting.ID}
	/varset 	returnTwist FALSE
	/squelch 	/twist ${songSet}
	/delay 20 ${Me.Casting.ID}
/return

|------------------------------------------------|
|- Loads melody from ini and configures for use	-|
|------------------------------------------------|
|-- to do, load from ini once + reload command
|-- automem songs in slots not used by other songs in melody

Sub BRD_Setup
/if (${Debug}) /echo |- BRD_Setup ==>
	/call InitoVar "${Character_Ini},Bard,Auto-Melody" autoMelody string outer
  /declare returnTwist		bool outer FALSE
  /declare playingMelody	bool outer FALSE
  /declare songSet string outer
  /declare resumeTwistDelay timer outer
/if (${Debug}) /echo <== BRD_Setup -|
/return

|------------------------------------|
Sub BRD_Background_Events	
	/doevents saveMelody
	/doevents startMelody
	/if (${returnTwist} && !${Bool[${Me.Casting}]} && !${Twist.Twisting} && !${resumeTwistDelay}) /call unpauseTwist
	| Check melody
	|/if (!${ActionTaken} && ${playingMelody} && !${Twist.Twisting} && !${Bool[${Me.Casting}]} && ${Bool[${songSet}]}) /doevents startMelody
/return

SUB BRD_MacroSettings
	| Adv Settings
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#2" "check_Buffs" 1
/RETURN

SUB BRD_CharacterSettings
	/call WriteToIni "${Character_Ini},Bard,Auto-Melody"
/RETURN

Sub BRD_Aliases
	/squelch /alias /SaveMelody /echo Save Melody
	/squelch /alias /PlayMelody /bc Melody
/return
