|----------------------------------------------------------------------------|
| Bard Functions
|----------------------------------------------------------------------------|
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
			/for i 1 to 8 + ${Me.SPA[326]}
				/if (${Ini[${Character_Ini},${melodyName} Melody,Song].Length}) /call WriteToIni "${Character_Ini},${melodyName} Melody,Song#${i}" *PLACEHOLDER 1
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
			/call WriteToIni "${Character_Ini},${melodyName} Melody,Song" "${Me.Gem[${songsToSave.Arg[${i}, ]}]}/gem|${songsToSave.Arg[${i}, ]}" ${If[${Ini[${Character_Ini},${melodyName} Melody,Song].Length},1,0]}
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
  /if (${Me.Class.ShortName.Equal[BRD]}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZR,${melodyName}]}) /return
    /declare memGem int local
    /varset melodyName ${c_eventArgData}
    | If I don't have the melodyName listed in my ini.
    /call IniToArrayV "${Character_Ini},${melodyName} Melody,Song#" melodyArray
    /if (!${Defined[melodyArray]}) {
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
      /for i 1 to ${melodyArray.Size}
      |/echo ${i} ${melodyArray[${i}]}
      /if (${melodyArray[${i}].Length} && ${melodyArray[${i}].NotEqual[PLACEHOLDER]}) {
        /varset spellName ${melodyArray[${i}].Arg[1,/]}
        | Check that the song is in my book
        /if (!${Me.Book[${spellName}]}) {
          /echo I don't have [${spellName}] in my spellbook, playing the melody without it.
        } else {
          | If the song is not memmed, use mq2Cast to mem
          /if (!${Me.Gem[${spellName}]}) {
            /call argueString gem| "${melodyArray[${i}]}"
            /if (${Bool[${c_argueString}]}) {
              /varset memGem ${c_argueString}
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
        /docommand ${ChatToggle} Playing [${melodyName}].
        /twist ${songSet}
      }
    }
  }
/if (${Debug}) /echo <== EVENT_startMelody -|
/return
|--------------------------------------------------------------|
SUB pauseTwist
/if (${Debug} || ${Debug_Casting}) /echo |- pauseTwist ==>
	/varset	returnTwist TRUE
	/squelch /twist stop
	/delay 20 !${Me.Casting.ID}
/if (${Debug} || ${Debug_Casting}) /echo <== pauseTwist -|
/return
|--------------------------------------------------------------|
SUB unpauseTwist
	/delay 2 !${Me.Casting.ID}
	/varset 	returnTwist FALSE
	/squelch 	/twist ${songSet}
	/delay 2 ${Me.Casting.ID}
/return
|--------------------------------------------------------------|
Sub check_bard_charm
/if (${Debug} || ${Debug_Ench}) /echo |- check_Charm ==>
/if (${Defined[charmArray2D]}) {
  /declare s int local
  /declare charmDuration int local
  /if (${doCharm} && ${Bool[${charmTarget}]}) {
    /if (${SpawnCount[npc id ${charmTarget}]}==0 && ${charmTarget}!=${Me.Pet.ID}) {
      /docommand ${ChatToggle} Disabling Charm: ${charmTarget} is no longer a valid npc
      /varset doCharm FALSE
      /varset charmTarget 0
      /return
    }
    /if (${Me.Pet.Race.Name.Equal[Invisible Man]}) {
      /pet get lost
      /delay 1s !${Me.Pet.ID}
    }
    /if (!${Bool[${Me.Pet.ID}]}) {
      /bc [+r+] << ${Spawn[id ${charmTarget}].CleanName} is not Charmed! >> [+r+]
      /for s 1 to ${charmArray2D.Size[1]}
      /call check_Ready "charmArray2D" ${s}
      /if (${c_Ready}) {
        /if (${check_Mana["charmArray2D",${s}]}) {
          /if (${check_Distance[${charmTarget} ${charmArray2D[${s},${iMyRange}]}]}) {
            /docommand ${ChatToggle} << Charming ${charmTarget} ${Spawn[id ${charmTarget}].CleanName} with ${charmArray2D[${s},${iCastName}]} >>
            /call e3_Cast ${charmTarget} "charmArray2D" "${s}"
            /if (${Bool[${Me.Pet.ID}]}) {
              /bc [+g+] >> Charmed ${Spawn[id ${charmTarget}].CleanName} <<
              /timed 5 /pet back off
              /timed 10 /pet follow
            } else {
              /if (${castReturn.Equal[CAST_IMMUNE]}) {
                /varset doCharm FALSE
                /varset charmTarget 0
                /docommand ${ChatToggle} << ${Spawn[id ${charmTarget}].CleanName} is immune, turning off charm >>
              }
            }
          }
        }
      }
      /if (!${Bool[${Me.Pet.ID}]}) /next s
    } else {
      |accounce time left on charm
      /for s 1 to ${charmArray2D.Size[1]}
      /if (${Bool[${Me.PetBuff[${charmArray2D[${s},${iSpellName}]}]}]}) {
        /varset charmDuration ${Pet.BuffDuration[${charmArray2D[${s},${iSpellName}]}].TotalSeconds}
        |/echo charmduration ${charmDuration} timer ${charmBuffTimer}
        /if (${charmDuration} > 60) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+g+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 18s
          }
        } else /if (${charmDuration} <= 60 && ${charmDuration} > 9) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+y+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 6s
          }
        } else /if (${charmDuration} <= 9) {
          /if (!${Bool[${charmBuffTimer}]}) {
            /bc [+r+] My charm fades in ${charmDuration} sec
            /varset charmBuffTimer 1s
          }
        }
      }
      /next s
    }
  }
}
/if (${Debug} || ${Debug_Ench}) /echo <== check_Charm -|
/return
|--------------------------------------------------------------|
| Mez mobs in ae radius that are not assist target
sub check_bard_mez
  /if (${Debug} || ${Debug_Ench}) /echo |- check_Mez ==>
  /if (${autoMez}) {
    /declare i int local
    /declare s int local
    /declare mezTarg int local
    /if (!${SpawnCount[playerstate 4 npc targetable radius ${AE_Radius}]}) /return
    /for i 1 to ${SpawnCount[playerstate 4 npc targetable radius ${AE_Radius}]}
      /varset mezTarg ${NearestSpawn[${i},playerstate 4 npc targetable radius ${AE_Radius}].ID}
      /if (${Spawn[id ${mezTarget}].CleanName.Find[' pet]}) /next i
      /call TrueTarget ${mezTarg}
      /delay 1s ${Target.BuffsPopulated}
      /if (${Select[${AssistTarget},${mezTarg}]}==0 && (!${Bool[${Target.Mezzed}]} || ${Target.Mezzed.Duration.TotalSeconds} < 5)) {
        /for s 1 to ${mezArray2D.Size[1]}
          /if (${check_Distance[${mezTarg},${mezArray2D[${s},${iMyRange}]}]}) {
            /if (${mezArray2D[${s},${iCastType}].Equal[Spell]} && !${Bool[${Me.Gem[${mezArray2D[${s},${iCastName}]}]}]}) /call memorize_spell "${mezArray2D[${s},${iCastName}]}" ${mezArray2D[${s},${iSpellGem}]}
              /docommand ${ChatToggle} << Mezzing ${mezTarg} with ${mezArray2D[${s},${iCastName}]} >>
              /if (${Twist.Twisting}) /call pauseTwist
              |:retryMez
              /twist once ${mezArray2D[${s},${iSpellGem}]}
              /delay 3s !${Me.Casting.ID}
              /delay 1s ${Target.BuffsPopulated}

              /if (${returnTwist}) /call unpauseTwist
            }
        /if (!${ActionTaken}) /next s
      }
    /if (!${ActionTaken}) /next i
  }
  /if (${Debug}) /echo <== check_Mez -|
/return
|--------------------------------------------------------------|
Sub BRD_Setup
/if (${Debug}) /echo |- BRD_Setup ==>
	/call iniToVarV "${Character_Ini},Bard,Auto-Melody" autoMelody string outer
  /call iniToVarV "${Character_Ini},Bard,AutoMez (On/Off)" autoMez bool outer
  /call IniToArrayV "${Character_Ini},Bard,Mez#"	  mezArray
  /call IniToArrayV "${Character_Ini},Bard,Charm#"	charmArray
  /declare returnTwist		bool outer FALSE
  /declare playingMelody	bool outer TRUE
  /declare songSet string outer
  /declare resumeTwistDelay timer outer

  /if (${Defined[mezArray]}) {
    /call BuildSpellArray "mezArray"	"mezArray2D"
  } else {
    /varset autoMez FALSE
  }

  /declare charmTarget int outer
  /if (${Defined[charmArray]}) {
    /call BuildSpellArray "charmArray"	"charmArray2D"
    /declare doCharm bool outer TRUE
    /declare charmBuffTimer timer outer
  } else {
    /declare doCharm bool outer FALSE
  }
/if (${Debug}) /echo <== BRD_Setup -|
/return
|--------------------------------------------------------------|
Sub BRD_Background_Events
  /if (${returnTwist} && !${Bool[${Me.Casting}]} && !${Twist.Twisting} && !${resumeTwistDelay}) /call unpauseTwist
	/doevents saveMelody
	/doevents startMelody
  /doevents charmTarget
  /doevents toggleMez
  /doevents charmOn
  /doevents charmOff
/return
|--------------------------------------------------------------|
SUB BRD_MacroSettings
	| Adv Settings
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BRD Functions,BRD Function#2" "check_Buffs" 1
/RETURN
|--------------------------------------------------------------|
SUB BRD_CharacterSettings
	/call WriteToIni "${Character_Ini},Bard,Auto-Melody"
  /call WriteToIni "${Character_Ini},Bard,AutoMez (On/Off)" Off
  /call WriteToIni "${Character_Ini},Bard,Mez"
  /call WriteToIni "${Character_Ini},Bard,Charm"
/RETURN
|--------------------------------------------------------------|
Sub BRD_Aliases
	/squelch /alias /SaveMelody /echo Save Melody
	/squelch /alias /PlayMelody /bc Melody
/return
