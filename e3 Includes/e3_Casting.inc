|----------------------------------------------------------------------------|
|- e3_Macro_Casting.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ												-|
|- Credit to (mq2cast)spell_routines.inc by Rusty and A_Druid_00			-|
|- Contains casting functions for Spells, AAs, Items, and Discs.			-|
|----------------------------------------------------------------------------|
| Returns these values: ${Macro.Return} and ${Cast.Result} and ${castReturn}

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
| CAST_COMPONENTS| Missing Component													  |
|---------------------------------------------------------------------------------------------|

|------------------------------------------------------------------------------------------------|
Sub e3_Cast(int targetID, ArrayName, int ArrayIndex)
/if (${Debug} || ${Debug_Casting}) /echo |- e3_Cast ==>
	/varset pendingCast		${${ArrayName}[${ArrayIndex},${iCastName}]}
	/varset pendingCastID	${${ArrayName}[${ArrayIndex},${iCastID}]}
	/declare castTargetName string local ${Spawn[id ${targetID}].CleanName}
	/varset castEndTime		0
	/varset interruptFlag FALSE
	/varset castReturn		0
	/declare pendingType	string local
  /declare oldItem			string local
	/declare i						int local

	/if (!${${ArrayName}[${ArrayIndex},${iCastInvis}]} && ${Me.Invis}) {
    /echo invisskip ${pendingCast} ${Spawn[id ${targetID}].Name}  ${targetID}
	} else {
    /echo cast ${pendingCast} ${pendingCastID} ${Spawn[id ${targetID}].Name}  ${targetID}
  }
	|/call explodeforcallstack
  /varset Debug_Casting FALSE
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
| Checks to abort casting due to previous failures

  |if im invis and iCastInvis=0 (default except for heals)
  |if im a rog, not in combat, done no action for 10+ seconds, and near 2+ other players then drop invis to buff (allows autohide)
  /if (${Me.Invis}) {
    /if (${Me.Class.ShortName.Equal[Rog]} && !${Me.Combat} && ${SpawnCount[pc radius 40]} > 2 && (${idleTimer}<${Math.Calc[${idleInterval}*600 - 100]} ) && !${SpawnCount[npc radius 100 zradius 100]}) {
      /makemevisible
    } else /if (!${${ArrayName}[${ArrayIndex},${iCastInvis}]}) {
      /varset castReturn CAST_INVISIBLE
      /goto :skipCast
    }
  }
  |- Check if spell has a NOHOLD timer
	/if (${Bool[${nht${targetID}-${pendingCastID}}]}) {
		/if (${Debug} || ${Debug_Casting}) /bc [${pendingCast}] Did not take hold last time I cast it on ${targetID}.  Waiting [${nht${targetID}-${pendingCastID}}] until I try again.
		/varset castReturn CAST_TAKEHOLD
		/goto :skipCast 
	}
	|- Check the Immune List
	/if (${Select[${pendingCastID}_${castTargetName},${immuneList}]}) {
		/if (${Debug} || ${Debug_Casting}) /bc ${Target.CleanName} is immune to ${pendingCastID}, skipping cast
		/varset castReturn CAST_IMMUNE
		/goto :skipCast 		
	}
	|- Check for spells listed in the failureList: reset on zone
	/if (${Select[${${ArrayName}[${ArrayIndex},${iCastName}]},${failureList}]}) {
		/if (${Debug} || ${Debug_Casting}) /echo [${pendingCast}] Is listed in the failureList in this zone.
		/varset castReturn FAILURELIST
		/goto :skipCast 
	}
	|- Reagent Check *|
	/if (${Bool[${${ArrayName}[${ArrayIndex},${iReagent}]}]} && !${Bool[${FindItemCount[=${${ArrayName}[${ArrayIndex},${iReagent}]}]}]}) {
		/bc Cannot cast [${pendingCast}], I do not have any [${${ArrayName}[${ArrayIndex},${iReagent}]}], removing this spell from array. Restock and Reload Macro
		/call RemoveSpellArrayElement "${ArrayName}" "${ArrayIndex}"
		/varset castReturn REAGENT
		/beep
		/goto :skipCast
	} else /if (${Bool[${${ArrayName}[${ArrayIndex},${iReagent}]}]} && ${FindItemCount[=${${ArrayName}[${ArrayIndex},${iReagent}]}]} <= ${reagentWarningCount}) {
		/bc I only have ${FindItemCount[=${${ArrayName}[${ArrayIndex},${iReagent}]}]} ${${ArrayName}[${ArrayIndex},${iReagent}]} left. Restock me jerk!
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
  /if (${${ArrayName}[${ArrayIndex},${iTargetType}].Find[pet]} && (!${Bool[${Me.Pet.ID}]} || ${Me.Pet.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]})) {
    /call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldPetDelay}"
    /goto :skipCast
  }
	/if (${Me.Feigning}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i am feigned and noFeigning=${noFeigning}
		/goto :skipCast
	}
	/if (${Window[SpellBookWnd].Open}) {
			/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , my spellbook is open and spellbook_Timer=${spellbook_Timer})
			/goto :skipCast
	}
	/if (${Corpse.Open}) {
		/if (${Debug} || ${Debug_Casting}) /bc skipping [${pendingCast}] , i have a corpse open and closeCorpse=${closeCorpse}
		/goto :skipCast
	}
  /if (!${${ArrayName}[${ArrayIndex},${iSpellType}].Find[Beneficial]} && !${Spawn[id ${targetID}].LineOfSight}) {
    /if (${Debug} || ${Debug_Casting}) /echo I cannot see [${Spawn[id ${targetID}].CleanName}]
    /varset castReturn CAST_CANNOTSEE
    /goto :skipCast
  }

	/if (${Twist.Twisting}) /call pauseTwist
	/delay 3s !${Me.Casting.ID}

	/declare cast_locX float local ${Me.X}
	/declare cast_locY float local ${Me.Y}	
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|  Casting
:cast_spell
	|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	|Use Disc
	/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]}) {
		/if (${Me.ActiveDisc.ID} && ${${ArrayName}[${ArrayIndex},${iDuration}]}) {
			/if (${Debug} || ${Debug_Casting}) /echo |- QuickBurn -| Waiting on disc [${Window[CombatAbilityWnd].Child[CAW_CombatEffectLabel].Text}]
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
		/if (${${ArrayName}[${ArrayIndex},${MyCastTime}]} >0 && ${Me.Class.ShortName.NotEqual[BRD]}) {
			/if (${Stick.Status.Equal[on]}) /squelch /stick pause
			/if (${NetAdvPath.Following} && !${NetAdvPath.Paused} && ${Following}) /squelch /netfollow pause	
			/delay 3 !${Me.Moving}
		}
		|- Equip an item that must be equipped to cast and is not currently equipped.  must use base inventory slots 24 through 31
		/if (${Bool[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}]} && ${FindItem[=${pendingCast}].ItemSlot} >=23 ) {
			/varset oldItem ${Me.Inventory[${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}].Name}
      /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${oldItem}/${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}" 1
			/call SwapItem "${pendingCast}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
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
    |/echo atcast "${pendingCastID}|${pendingType}" "-targetid|${targetID}"
		/varset ActionTaken TRUE
    /if (${${ArrayName}[${ArrayIndex},${iTargetType}].Equal[Self]}) {
		  /casting "${pendingCastID}|${pendingType}"
    } else {
      /casting "${pendingCastID}|${pendingType}" "-targetid|${targetID}"
    }
		|- Memorizing spell
		/if (${Cast.Status.Find[M]}) /delay 4s !${Cast.Status.Find[M]}
		|- Set expected cast end time in 1/10ths of a second
		/varcalc castEndTime ${Me.Casting.MyCastTime}/100
    |/echo incastchecks ${Time}
:cast_still_pending
|- check for reasons to cancel casting

		/if (${Cast.Status.Find[C]}) {
		  |dont allow item swap/must equip interrupts to avoid crashes
		  /if (${oldItem.Length}) {
		    /delay 2
		    /goto :cast_still_pending
		  }
			/if (!${interruptFlag}) {
			   |- Run SubToRun
			   |/varset Debug_Casting TRUE
				/if (${Bool[${${ArrayName}[${ArrayIndex},${iSubToRun}]}]}) {
					/call ${${ArrayName}[${ArrayIndex},${iSubToRun}]} ${ArrayName} ${ArrayIndex}
					/if (!${Macro.Return} && ${Me.Casting.ID}) {
           /if (${Debug} || ${Debug_Casting}) /echo SubToRun called interrupt ${${ArrayName}[${ArrayIndex},${iSubToRun}]}
					 /call interrupt
					 /goto :interrupted
					}
        |-no SubToRun
				} else {
				  /delay 1
				}
				|- Other reasons to cancel
        /if (!${Assisting} && ${SpawnCount[=${FollowTarget}]} && ${Spawn[=${FollowTarget}].Distance} < ${LeashLength} && (${Spawn[=${FollowTarget}].Distance} > 20 || ${NetBots[${FollowTarget}].Moving}) ) {
          /if (${Debug} || ${Debug_Casting}) /echo following interrupt
          |/bc following interrupt ${Time}  ${NetBots[${FollowTarget}].Moving}  ${Spawn[${FollowTarget}].Moving} ${Spawn[${FollowTarget}].Distance}
          /call interrupt
          /goto :interrupted
        }
        |- goose
				/if (${Me.State.Equal[DUCK]}) {
          /if (${Debug} || ${Debug_Casting}) /echo duck called interrupt
					/call interrupt
          /goto :interrupted
				}
				|-Dead target and I'm not using a detrimental AE or self buff
				|/echo ${${ArrayName}[${ArrayIndex},${iCastName}]} ${${ArrayName}[${ArrayIndex},${iTargetType}]}
				/if (!${Bool[${Target.ID}]} && !${Select[${${ArrayName}[${ArrayIndex},${iTargetType}]},PB AE,Targeted AE,Corpse,Self]}) {
          /if (${Debug} || ${Debug_Casting}) /echo target corpse called interrupt
					/call interrupt
          /goto :interrupted
				}
				|-Target is out of range
				/if (${targetID} != ${Me.ID} && ${Target.Distance} > ${${ArrayName}[${ArrayIndex},${iMyRange}]}) {
          /if (${Debug} || ${Debug_Casting}) /echo target range called interrupt
					/call interrupt
          /goto :interrupted
				}
				|-Moved since casting began
				/if ((${cast_locX}!=${Me.X} || ${cast_locY}!=${Me.Y})) {
					/call Triangulate_Distance ${cast_locY},${cast_locY} ${Me.Loc.Replace[ ,]}
					/if (${Macro.Return}>=10 && ${castEndTime}>5) {
            /if (${Debug} || ${Debug_Casting}) /echo moved 10+ units after casting: called interrupt
						/call interrupt
            /goto :interrupted
					}
				}
				|-Check LifeSupport if more than 1.5 sec left to cast end
				/if (${Bool[${lifeSupport2D.Size[1]}]} && ${Me.PctHPs}<70 && ${castEndTime}>15) /call check_lifeSupport
        |-check only select background events
				/doevents Follow
				/doevents Stop
				/doevents MoveHere
				/doevents BackOff
				/doevents clickit
				/doevents bark
				/doevents vetAA_Background_Events
				/call alerts_Background_Events
        |/varset Debug_Casting FALSE
			}
			/goto :cast_still_pending
		}
  :interrupted
    |/echo outcastchecks ${Time}
		/varset castReturn ${Cast.Result}
		|- Cast Result Processing
		/if (${Select[${castReturn},CAST_SUCCESS]}) {
			/varset lastSuccessfulCast ${pendingCast}
		} else /if (${interruptFlag}) {		
			/varset castReturn CAST_CANCELLED
		} else /if (${castReturn.Equal[CAST_RESIST]}) {
			/if (${Debug_Casting} && ${Target.ID}) /docommand ${ChatToggle} ** ${Target.Name} resisted ${pendingCast} **
		} else /if (${castReturn.Equal[CAST_IMMUNE]}) {
      /docommand ${ChatToggle} ${Target.Name} is immune to ${pendingCast} **
			/varset immuneList ${immuneList}${pendingCastID}_${castTargetName},
    |dont create a nohold timer for detrimental AE spells
		} else /if (${castReturn.Equal[CAST_TAKEHOLD]} && ${Select[${ArrayName},TargetAE_Spells2D,PBAE_Spells2D]}==0) {
      /if (${Target.ID}) {
				/docommand ${ChatToggle} ** ${pendingCast} did not take hold on ${castTargetName} **
				/if (${Defined[nht${targetID}-${pendingCastID}]}) {
					/varset nht${targetID}-${pendingCastID} ${noHoldDelay}
				} else {
					/call CreateTimer "nht${targetID}-${pendingCastID}" "${noHoldDelay}"
				}
			}
		} else /if (${Select[${castReturn},CAST_OUTDOORS,CAST_COMPONENTS,CAST_UNKNOWN]}) {
			/docommand ** ${ChatToggle} Adding to failureList due to Return Events CAST_OUTDOORS,CAST_COMPONENT,CAST_UNKNOWN
			/varset failureList ${failureList}${pendingCastID},		
		}
  }

:skipCast
	/if (${oldItem.Length}) /call SwapItem "${oldItem}" "${${ArrayName}[${ArrayIndex},${iItemMustEquip}]}"
	/if (${Following} && !${Assisting}) /call AquireFollow
  /if (${Me.Class.ShortName.Equal[BRD]}) /varset resumeTwistDelay 1s
  |/if (${returnTwist})	/call unpauseTwist
	
	/if (${Debug} || ${Debug_Casting})  /echo |- e3_Cast -| castReturn= ${Cast.Result} ${castReturn}
/varset Debug_Casting FALSE
/return

|------------------------------------------------|
|- Check if the Spell/Item/AA is ready			    -|
|------------------------------------------------|
Sub check_Ready(ArrayName, int ArrayIndex)
  /declare passCheck bool local FALSE
| Memorize the spell if it is not memorized
/if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]} && !${Bool[${Me.Gem[${${ArrayName}[${ArrayIndex},${iCastName}]}]}]}) /call memorize_spell "${${ArrayName}[${ArrayIndex},${iCastName}]}" ${${ArrayName}[${ArrayIndex},${iSpellGem}]}

  /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Spell]} && ${Me.SpellReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
    /varset passCheck TRUE
    |${Me.ItemReady[=worn totem]}
  |} else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]} && ${Bool[${FindItem[${${ArrayName}[${ArrayIndex},${iCastName}]}]}]} && ${FindItem[${${ArrayName}[${ArrayIndex},${iCastName}]}].TimerReady}==0) {
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Item]} && ${Me.ItemReady[=${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
    /varset passCheck TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[AA]} && ${Me.AltAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
    /varset passCheck TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Disc]} && ${Me.CombatAbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
    /varset passCheck TRUE
  } else /if (${${ArrayName}[${ArrayIndex},${iCastType}].Equal[Ability]} && ${Me.AbilityReady[${${ArrayName}[${ArrayIndex},${iCastName}]}]}) {
    /varset passCheck TRUE
  }
/RETURN ${passCheck}

Sub memorize_spell(spellName,gemNum)
  /declare secWaited int local 0
  /memorize "${spellName}" ${gemNum}
  /delay 5s ${Bool[${Me.Gem[${spellName}]}]}
  /if (${Me.CombatState.NotEqual[COMBAT]}) {
    :checkMem
      /delay 2s ${Me.SpellReady[${spellName}]}
      /varcalc secWaited ${secWaited}+1
      /call Background_Events
      /if (${secWaited} < 5) /goto :checkMem
  } else {
    /delay 2s ${Me.SpellReady[${spellName}]}
  }
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
	/declare passCheck bool local FALSE
	/if (${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]} && ${Spawn[id ${targetID}].Distance} <= ${myRange}) /varset passCheck TRUE
/RETURN ${passCheck}

|------------------------------------------------|
|- Check various Mana-related metrics		    -|
|------------------------------------------------|
Sub check_Mana(ArrayName, int ArrayIndex)
	/declare passCheck bool local FALSE
	/if (${Me.CurrentMana} >= ${${ArrayName}[${ArrayIndex},${iMana}]}) {
		/if (${Me.PctMana} >= ${${ArrayName}[${ArrayIndex},${iMinMana}]}) {
			/if (${Me.PctMana} <= ${${ArrayName}[${ArrayIndex},${iMaxMana}]}) /varset passCheck TRUE
		}
	}
/RETURN ${passCheck}

|------------------------------------------------|
|- Interrupt a spell when appropriate		    -|
|------------------------------------------------|
Sub interrupt
	/varset interruptFlag TRUE
	/interrupt
/return

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SUB casting_MacroSettings
/if (${Debug} || ${Debug_Casting}) /echo |- casting_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Casting (On/Off)"  Off
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Set" Default
	/call WriteToIni "${genSettings_Ini},Casting,Default Spell Gem" ${If[${Me.AltAbility[Mnemonic Retention]},9,8]}
/if (${Debug} || ${Debug_Casting}) /echo <== casting_MacroSettings -|
/RETURN

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Casting (On/Off)" Debug_Casting bool outer
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
	/declare giveUpTimer				timer outer
	/declare noInvis						bool outer TRUE
	/declare noFeigning					bool outer TRUE
  /declare interruptFlag			bool outer FALSE
	
	/declare failureList			string outer
	/declare immuneList				string outer

| Import Casting Settings.	
	| Default Spellset
	/if (${Ini[${genSettings_Ini},Casting,Default Spell Set].Length}) /call IniToVar "${genSettings_Ini},Casting,Default Spell Set" Default_SpellSet string outer	
	| Default Gems
	/declare DefaultGem int outer 8
	/call IniToVar "${genSettings_Ini},Casting,Default Spell Gem" DefaultGem int outer

/if (${Debug} || ${Debug_Casting}) /echo <== casting_Setup -|
/return

|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sub casting_Background_Events
/return
SUB casting_CharacterSettings
/return
Sub casting_Aliases
/return