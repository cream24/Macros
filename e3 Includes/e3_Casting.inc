|----------------------------------------------------------------------------|
| Returns these values: ${Cast.Result} and ${castReturn}
|- The vast majority of checks (distance/mana/hp/etc should be done by the funcion
|- that calls e3_casting (i.e e3_heals, e3_assist, e3_buffcheck)
|----------------------+----------------------------------------------------------------------|
| CAST_CANCELLED       | Spell was cancelled by ducking (either manually or because mob died) |
| CAST_CANNOTSEE       | You can't see your target                                            |
| CAST_IMMUNE          | Target is immune to this spell                                       |
| CAST_INTERRUPTED     | Casting was interrupted and exceeded the given time limit            |
| CAST_INVIS           | You were invis, and noInvis is set to true                           |
| CAST_NOTARGET        | You don't have a target selected for this spell                      |
| CAST_NOTMEMMED       | Spell is not memmed and you gem to mem was not specified             |
| CAST_NOTREADY        | AA ability or spell is not ready yet                                 |
| CAST_OUTOFMANA       | You don't have enough mana for this spell!                           |
| CAST_OUTOFRANGE      | Target is out of range                                               |
| CAST_RESIST          | Your spell was resisted!                                             |
| CAST_SUCCESS         | Your spell was cast successfully! (yay)                              |
| CAST_UNKNOWN         | Spell/Item/Ability was not found                                     |
| CAST_COLLAPSE        | Gate Collapsed                                                       |
| CAST_TAKEHOLD        | Spell not hold                                                       |
| CAST_FIZZLE          | Spell Fizzle                                                         |
| CAST_INVISIBLE       | NOT Casting Invis                                                    |
| CAST_RECOVER	       | Spell not Recovered yet!                                             |
| CAST_STUNNED	       | Stunned                                                              |
| CAST_STANDIG	       | Not Standing                                                         |
| CAST_DISTRACTED      | To Distracted ( spell book open )                                    |
| CAST_COMPONENTS| Missing Component													                                |
|---------------------------------------------------------------------------------------------|
Sub e3_Cast(int targetID, ArrayName, int ArrayIndex)
/if (${Debug} || ${Debug_Casting}) /echo |- e3_Cast ==>
	/varset pendingCast		${${ArrayName}[${ArrayIndex},${iCastName}]}
	/varset pendingCastID	${${ArrayName}[${ArrayIndex},${iCastID}]}
	/declare castTargetName string local ${Spawn[id ${targetID}].CleanName}
	/varset castEndTime		0
  /varset castReturn		0
	/varset interruptFlag FALSE
	/declare pendingType	string local
  /declare oldItem			string local
	/declare i						int local
  |/varset Debug_Casting TRUE
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
| Checks to abort casting due to previous failures
  |if im invis and iCastInvis=0 (default except for heals)
  |if im a rog, not in combat, done no action for 10+ seconds, and near 2+ other players then drop invis to buff (allows autohide)
  /if (${Me.Invis}) {
    /if (${Me.Class.ShortName.Equal[Rog]}) {
      /if (!${Me.Combat} && ${SpawnCount[pc radius 40]} > 2 && !${SpawnCount[npc radius 100 zradius 100]} && ${idleTimer}<${Math.Calc[${idleInterval}*600 - 100]}) /makemevisible
    } else /if (!${${ArrayName}[${ArrayIndex},${iCastInvis}]}) {
      /varset castReturn CAST_INVISIBLE
			/echo SkipCast-Invis ${pendingCast} ${Spawn[id ${targetID}].Name} ${targetID}
			/varset ActionTaken TRUE
      /goto :skipCast
    }
  }
  |- Check if spell has a NOHOLD timer
  /if (${Defined[nht${targetID}-${pendingCastID}]}) {
    /if (${Bool[${nht${targetID}-${pendingCastID}}]}) {
      /varset castReturn CAST_TAKEHOLD
      /if (${Debug} || ${Debug_Casting}) /echo SkipCast-NoHold ${pendingCast} on ${Spawn[id ${targetID}].Name} Waiting [${nht${targetID}-${pendingCastID}}]
      /goto :skipCast
    } else {
      /echo Deleting nht${targetID}-${pendingCastID} ${nht${targetID}-${pendingCastID}}
      /deletevar nht${targetID}-${pendingCastID}
    }
  }
	|- Check the Immune List
	/if (${Select[${pendingCastID}_${castTargetName},${immuneList}]}) {
		/varset castReturn CAST_IMMUNE
    /if (${Debug} || ${Debug_Casting}) /echo SkipCast-Immune ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
		/goto :skipCast 		
	}
	|- Check for spells listed in the failureList: reset on zone
	/if (${Select[${${ArrayName}[${ArrayIndex},${iCastName}]},${failureList}]}) {
		/varset castReturn FAILURELIST
    /if (${Debug} || ${Debug_Casting}) /echo SkipCast-Failure ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
		/goto :skipCast 
	}
	|- Reagent Check *|
	/if (${Bool[${${ArrayName}[${ArrayIndex},${iReagent}]}]}) {
	  /declare itemCount int local ${FindItemCount[=${${ArrayName}[${ArrayIndex},${iReagent}]}]}
	  /if (!${itemCount}) {
      /bc Cannot cast [${pendingCast}], I do not have any [${${ArrayName}[${ArrayIndex},${iReagent}]}], removing this spell from array. Restock and Reload Macro
      /call RemoveSpellArrayElement "${ArrayName}" "${ArrayIndex}"
      /varset castReturn REAGENT
      /echo SkipCast-NoReagent ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
      /beep
      /goto :skipCast
    } else /if (${itemCount} <= ${reagentWarningCount}) {
      /bc I only have ${itemCount} ${${ArrayName}[${ArrayIndex},${iReagent}]} left. Restock me!
    }
	}
  |- Dont cast while waiting for char to load post-zoning
  /if (${currentZone} != ${Zone.ID}) {
    /delay 1s
    /varset castReturn CAST_ZONING
    /goto :skipCast
  }

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~		
|  Basic Checks
:wait_for_misc
  /if (${${ArrayName}[${ArrayIndex},${iTargetType}].Find[pet]}) {
    /if (!${Bool[${Me.Pet.ID}]} || ${Me.Pet.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]}) {
      /call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldPetDelay}"
      /echo SkipCast-PetRange ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
      /goto :skipCast
    }
  }
	/if (${Me.Feigning}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i am feigned and noFeigning=${noFeigning}
		/goto :skipCast
	}
	/if (${Window[SpellBookWnd].Open}) {
    /if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , my spellbook is open
    /goto :skipCast
	}
	/if (${Corpse.Open}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i have a corpse open and closeCorpse=${closeCorpse}
		/goto :skipCast
	}

  /if (!${${ArrayName}[${ArrayIndex},${iSpellType}].Find[Beneficial]}) {
    /if (!${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Self]}) {
      /if (!${Spawn[id ${targetID}].LineOfSight}) {
        /if (${Debug} || ${Debug_Casting}) /echo I cannot see [${Spawn[id ${targetID}].CleanName}]
        /varset castReturn CAST_CANNOTSEE
        /echo SkipCast-LoS ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
        /goto :skipCast
      }
    }
  }

	/if (${Twist.Twisting}) /call pauseTwist
	/delay 3s !${Me.Casting.ID}

	/declare startingLoc ${Me.Loc.Replace[ ,]}
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|  Casting
:cast_spell
  |/if (${Me.Name.Equal[Lube]}) /gu Casting ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
	/echo Casting ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name} ${targetID}
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|Use Disc
	/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
		/if (${Me.ActiveDisc.ID} && ${${ArrayName}[${ArrayIndex},${iDuration}]}) {
			/if (${Debug} || ${Debug_Casting}) /echo Skipping ${ArrayName}[${ArrayIndex},${iCastName}, Waiting on disc [${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}]
			/varset castReturn ACTIVEDISC
			/goto :skipCast
		} else {
			| Only change targets if needed
			/if (${targetID} != ${Me.ID} && ${Target.ID} != ${targetID}) {
				/squelch /target id ${targetID}
				/delay 5 ${Target.ID} == ${targetID}
			}
      /varset ActionTaken TRUE
			/disc ${pendingCast}
		}
	|-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|- Abilities should never go to e3_cast, just a safeguard
	} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]}) {
		/if (${Me.AbilityReady[${abilityName}]}) {
      /varset ActionTaken TRUE
		  /doability ${pendingCast}
    }
	|-~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|- Spells, AA, Items
	} else {
			|- Stop following for spell/item/aa with a cast time > 0 MyCastTime, unless im a bard
		/if (${${ArrayName}[${ArrayIndex},${iMyCastTime}]} >0 && ${Me.Class.ShortName.NotEqual[BRD]}) {
			/if (${Stick.Status.Equal[on]}) /squelch /stick pause
			/if (${NetAdvPath.Following} && ${Following}) /squelch /netfollow off
			/if (${MoveTo.Moving}) /moveto off
			/delay 3 !${Me.Moving}
		}
		|- Equip an item that must be equipped to cast and is not currently equipped.  must use base inventory slots 24 through 31
		/if (${Bool[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}]}) {
		  /if (${FindItem[=${pendingCast}].ItemSlot} >=23) {
        /varset oldItem ${Me.Inventory[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}].Name}
        /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${oldItem}/${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}" 1
        /call SwapItem "${pendingCast}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
      }
		}
		|- Define item|alt|gem# parameter
		/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]}) {
			/varset pendingType alt
		} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]}) {
			/varset pendingType item
		} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]}) {
			|- if Gem specified, use it, else use default
			/if (${${ArrayName}[${ArrayIndex},${iSpellGem}]}) {
				/varset pendingType ${${ArrayName}[${ArrayIndex},${iSpellGem}]}
			} else {
				/varset pendingType ${DefaultGem}
			}
		}
    |/echo AT pendingCastID ${pendingCastID} pendingType ${pendingType}" "-targetid|${targetID}"
		/varset ActionTaken TRUE
    /if (${${ArrayName}[${ArrayIndex},${iTargetType}].Equal[Self]}) {
		  /casting "${pendingCastID}|${pendingType}"
    } else {
      /casting "${pendingCastID}|${pendingType}" "-targetid|${targetID}"
    }
		|- Memorizing spell
		/if (${Cast.Status.Find[M]}) /delay 3s !${Cast.Status.Find[M]}
		|- Set expected cast end time in 1/10ths of a second
		/varcalc castEndTime ${Me.Casting.MyCastTime}/100
:cast_still_pending
|- check for reasons to cancel casting
		/if (${Cast.Status.Find[C]}) {
		  |dont allow item swap/must equip interrupts to avoid crashes
		  /if (${oldItem.Length}) {
		    /delay 1
		    /goto :cast_still_pending
		  }
			/if (!${interruptFlag}) {
			   |- Run SubToRun
			   |/varset Debug_Casting TRUE
				/if (${Bool[${${ArrayName}[${ArrayIndex},${iSubToRun}]}]}) {
					/call ${${ArrayName}[${ArrayIndex},${iSubToRun}]} ${ArrayName} ${ArrayIndex}
					/if (!${c_SubToRun} && ${Me.Casting.ID}) {
           /if (${Debug} || ${Debug_Casting}) /echo SubToRun called interrupt ${${ArrayName}[${ArrayIndex},${iSubToRun}]}
					 /call interrupt
					 /goto :interrupted
					}
        |-no SubToRun
				} else {
				  /delay 1
				}
				|this will make tank and important heals ignore everything but subToRun
        /if (${Select[${ArrayName},tankHeals2D,importantHeals2D]}) /goto :cast_still_pending
				|- Other reasons to cancel
        /if (!${Assisting} && ${Following}) {
          /if (${SpawnCount[=${FollowTarget}]} && ${Spawn[=${FollowTarget}].Distance} < ${LeashLength}) {
            /if ((${Spawn[=${FollowTarget}].Distance} > 20 || ${NetBots[${FollowTarget}].Moving})) {
              /if (${Debug} || ${Debug_Casting}) /echo following interrupt
              |/bc following interrupt ${Time}  ${NetBots[${FollowTarget}].Moving}  ${Spawn[${FollowTarget}].Moving} ${Spawn[${FollowTarget}].Distance}
              /call interrupt
              /goto :interrupted
            }
          }
        }
        |- goose
				/if (${Me.State.Equal[DUCK]}) {
          /if (${Debug} || ${Debug_Casting}) /echo duck called interrupt
					/call interrupt
          /goto :interrupted
				}
				|-Dead target and I'm not using a detrimental AE or self buff
				/if (!${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Targeted AE,Corpse,Self]}) {
				  /if ((!${Bool[${Target.ID}]} || ${Spawn[${Target.ID}].Dead})) {
            /if (${Debug} || ${Debug_Casting}) /echo target corpse called interrupt
            /call interrupt
            /goto :interrupted
          }
				}
				|-Target is out of range
				/if (${targetID} != ${Me.ID} && !${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Self]}) {
				  /if (${Target.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]}) {
            /if (${Debug} || ${Debug_Casting}) /echo target range called interrupt
            /call interrupt
            /goto :interrupted
          }
				}
				|-Moved since casting began
				/if (${castEndTime}>5) {
					/if (${Math.Distance[${startingLoc}]}>=8) {
            /if (${Debug} || ${Debug_Casting}) /echo moved 8+ units after casting: called interrupt
						/call interrupt
            /goto :interrupted
					}
				}
				|-Check LifeSupport if more than 1.5 sec left to cast end
				/if (${Defined[lifeSupport2D]}) {
				  /if (${Me.PctHPs}<70 && ${castEndTime}>15) /call check_lifeSupport
        }
        |-check only select background events
				/doevents Follow
				/doevents Stop
				/doevents MoveHere
				/doevents BackOff
				/doevents clickit
				/doevents bark
				/call alerts_Background_Events
			}
			/goto :cast_still_pending
		}
  :interrupted
		/varset castReturn ${Cast.Result}
		|- Cast Result Processing
		/if (${castReturn.Equal[CAST_SUCCESS]}) {
			/varset lastSuccessfulCast ${pendingCast}
		} else /if (${castReturn.Equal[CAST_INTERRUPTED]} || ${interruptFlag}) {
			/varset castReturn CAST_INTERRUPTED
		} else /if (${castReturn.Equal[CAST_RESIST]}) {
			/if (${Debug_Casting} && ${Target.ID}) /docommand ${ChatToggle} ** ${Target.Name} resisted ${pendingCast} **
		} else /if (${castReturn.Equal[CAST_IMMUNE]}) {
      /docommand ${ChatToggle} ${Target.Name} is immune to ${pendingCast} **
			/varset immuneList ${immuneList}${pendingCastID}_${castTargetName},
    |dont create a nohold timer for detrimental AE spells
		} else /if (${castReturn.Equal[CAST_TAKEHOLD]} && ${Select[${ArrayName},TargetAE_Spells2D,PBAE_Spells2D]}==0) {
      /if (${Target.ID}==${targetID}) {
				/docommand ${ChatToggle} ** ${pendingCast} did not take hold on ${castTargetName} **
        /call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldDelay}"
			}
		} else /if (${Select[${castReturn},CAST_OUTDOORS,CAST_COMPONENTS,CAST_UNKNOWN]}) {
			/docommand ** ${ChatToggle} Adding to failureList due to Return Events CAST_OUTDOORS,CAST_COMPONENT,CAST_UNKNOWN
			/varset failureList ${failureList}${pendingCastID},		
		}
  }

:skipCast
	/if (${oldItem.Length}) /call SwapItem "${oldItem}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
	/if (${Following} && !${Assisting}) /call AcquireFollow
  /if (${Me.Class.ShortName.Equal[BRD]}) /varset resumeTwistDelay 5
  |/if (${returnTwist})	/call unpauseTwist
	/if (${Debug} || ${Debug_Casting})  /echo |- e3_Cast -| castReturn= ${Cast.Result} ${castReturn}
|/varset Debug_Casting FALSE
/return

|------------------------------------------------|
|- Check if the Spell/Item/AA is ready			    -|
|------------------------------------------------|
Sub check_Ready(ArrayName, int ArrayIndex)
  /if (${Debug} || ${Debug_Casting}) /echo |- check_Ready =>
  /varset c_Ready FALSE
  | Memorize the spell if it is not memorized
  /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]}) {
    /if (!${Bool[${Me.Gem[${${ArrayName}[${ArrayIndex},${iCastName}]}]}]}) /call memorize_spell "${${ArrayName}[${ArrayIndex},${iCastName}]}" ${${ArrayName}[${ArrayIndex},${iSpellGem}]}
    /if (${Me.SpellReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) /varset c_Ready TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]}) {
    /if (${Me.ItemReady[=${${ArrayName}[${ArrayIndex},${iCastName}]}]}) /varset c_Ready TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]}) {
    /if (${Me.AltAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]})  /varset c_Ready TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
    /if (${Me.CombatAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) /varset c_Ready TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]}) {
    /if (${Me.AbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) /varset c_Ready TRUE
  }
  /if (${Debug} || ${Debug_Casting}) /echo |- check_Ready <= ${c_Ready}
/RETURN ${c_Ready}

Sub memorize_spell(spellName,gemNum)
  /if (${Debug} || ${Debug_Casting}) /echo |- memorize_spell =>
  /memorize "${spellName}" ${gemNum}
	/delay 3s ${Bool[${Me.Gem[${spellName}]}]}
	|only wait for beneficial,self spells
	/if (${Spell[${spellName}].Beneficial} || ${Spell[${spellName}].TargetType.Equal[Self]}) {
    /declare waitForReady timer local ${If[${Me.CombatState.Equal[COMBAT]},35,${Math.Calc[35+(${Spell[${spellName}].RecastTime}/100)]}]}
    |/echo wfr ${waitForReady} rc ${Spell[${spellName}].RecastTime} math ${Math.Calc[25+(${Spell[${spellName}].RecastTime}/100)]}
    :checkTimer
      |/echo chectimer ${waitForReady} ${Me.SpellReady[${spellName}]}
      /if (${waitForReady}) {
      /doevents Follow
      /doevents Stop
      /doevents MoveHere
      /doevents BackOff
      /call alerts_Background_Events
      /delay 1
      /if (!${Me.SpellReady[${spellName}]}) /goto :checkTimer
		}
	}
  /if (${Debug} || ${Debug_Casting}) /echo |- memorize_spell <=
/RETURN
|------------------------------------------------|
|- concept from KA                        	    -|
|- MQ needs a .recovered function               -|
|------------------------------------------------|
Sub check_GCD
  /declare passCheck bool local FALSE
  /if (${Me.SpellReady[${Me.Gem[1].Name}]} || ${Me.SpellReady[${Me.Gem[3].Name}]} || ${Me.SpellReady[${Me.Gem[5].Name}]} || ${Me.SpellReady[${Me.Gem[7].Name}]}) /varset passCheck TRUE
/RETURN ${passCheck}

|------------------------------------------------|
|- Check if the Target is in Zone and in Range  -|
|------------------------------------------------|
Sub check_Distance(int targetID, int myRange)
	/declare c_Dist bool local FALSE
	/if (${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
	  /if (${myRange}==0 || ${Spawn[id ${targetID}].Distance} <= ${myRange}) /varset c_Dist TRUE
  }
/RETURN ${c_Dist}

|------------------------------------------------|
|- Check various Mana-related metrics		        -|
|------------------------------------------------|
Sub check_Mana(ArrayName, int ArrayIndex)
	/declare hasMana bool local FALSE
	/if (${Me.CurrentMana} >= ${${ArrayName}[${ArrayIndex},${iMana}]}) {
		/if (${Me.PctMana} >= ${${ArrayName}[${ArrayIndex},${iMinMana}]}) {
			/if (${Me.PctMana} <= ${${ArrayName}[${ArrayIndex},${iMaxMana}]}) /varset hasMana TRUE
		}
	}
/RETURN ${hasMana}

|------------------------------------------------|
|- Interrupt a spell when appropriate		    -|
|------------------------------------------------|
Sub interrupt
	/varset interruptFlag TRUE
	/interrupt
/return

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#EVENT vetAA "[#1#] VetAA #2#"
#EVENT vetAA "<#1#> VetAA #2#"
#EVENT vetAA "#1# tells you, 'VetAA #2#'"
#EVENT vetAA "#1# tells the group, 'VetAA #2#'"
#EVENT vetAA "#1# tell your party, 'VetAA #2#'"
SUB EVENT_vetAA(line, ChatSender,vetAAtoUse)
	/varset line ${line.Left[${Math.Calc[${line.Find[,]}-1]}]} ${line.Right[-${line.Find[,]}]}
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${vetAAtoUse}]}) /return
  /varset vetAAtoUse ${c_eventArgData}

  /declare i int local
  /for i 1 to ${vetArray2D.Size[1]}
  /if (${vetArray2D[${i},${iCastName}].Equal[${vetAAtoUse}]}) {
    /call check_Ready "vetArray2D" ${i}
    /if (${c_Ready}) {
      /if (${i}!=${iStaunch} || (${Me.PctMana}>30 && ${Me.PctEndurance}>30)) {
        /call e3_Cast ${Me.ID} "vetArray2D" ${i}
      }
    } else /echo ${vetAAtoUse} is not ready..
  }
  /next i
/RETURN

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SUB casting_MacroSettings
/if (${Debug}) /echo |- casting_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Casting (On/Off)"  Off
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Set" Default
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Gem" ${If[${Me.AltAbility[Mnemonic Retention]},9,8]}
/if (${Debug}) /echo <== casting_MacroSettings -|
/RETURN

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Casting (On/Off)" Debug_Casting bool outer
/if (${Debug} || ${Debug_Casting}) /echo |- casting_Setup ==>
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	| Configurable
	/declare reagentWarningCount	int outer 5
	/declare noHoldDelay			int outer 600
	/declare noHoldPetDelay   int outer 300
	/declare closeCorpse			string outer FALSE
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	
	/declare pendingCast				string outer
	/declare pendingCastID			int outer
	/declare lastSuccessfulCast	string outer
	/declare castReturn					string outer
	/declare castEndTime				timer outer
	/declare noInvis						bool outer TRUE
	/declare noFeigning					bool outer TRUE
  /declare interruptFlag			bool outer FALSE
	
	/declare failureList			string outer
	/declare immuneList				string outer

  /declare c_Ready bool outer FALSE
  /declare c_SubToRun bool outer FALSE

	/declare DefaultGem int outer 8
	/call iniToVarV "${genSettings_Ini},Casting,Default Spell Gem" DefaultGem int outer
  /call iniToVarV "${genSettings_Ini},Casting,Default Spell Set" Default_SpellSet string outer

  /declare vetArray[9] string outer
  /declare iLesson    int outer 1
  /declare iThrone    int outer 2
  /declare iArmor     int outer 3
  /declare iIntensity int outer 4
  /declare iImfusion  int outer 5
  /declare iSteadfast int outer 6
  /declare iStaunch   int outer 7
  /declare iExpedient int outer 8
  /declare iJester    int outer 9
  /varset vetArray[${iLesson}]    "Lesson of the Devoted"
  /varset vetArray[${iThrone}]    "Throne of Heroes"
  /varset vetArray[${iArmor}]     "Armor of Experience"
  /varset vetArray[${iIntensity}] "Intensity of the Resolute"
  /varset vetArray[${iImfusion}]  "Infusion of the Faithful"
  /varset vetArray[${iSteadfast}] "Steadfast Servant"
  /varset vetArray[${iStaunch}]   "Staunch Recovery"
  /varset vetArray[${iExpedient}] "Expedient Recovery"
  /varset vetArray[${iJester}]    "Chaotic Jester"
  /call BuildSpellArray "vetArray" "vetArray2D"

/if (${Debug} || ${Debug_Casting}) /echo <== casting_Setup -|
/return

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Background_Events
  /doevents vetAA
/return

SUB casting_CharacterSettings
/return

Sub casting_Aliases
/return