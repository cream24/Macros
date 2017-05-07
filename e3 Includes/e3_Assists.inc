|--------------------------------------------------------------------|
|- e3_Macro_Assists.inc	( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ										                    -|
|--------------------------------------------------------------------|
|- Contains assisting related functions for e3.mac.					        -|
|- check_IsLookingAt by genbot.mac                                  -|
|- Use, 'Assist on ${Target.ID}' in /tell, /group, or /bc.					-|
|--------------------------------------------------------------------|
#event Assist "<#1#> Assist on #2#"
SUB event_Assist(line, ChatSender, int MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_Assist ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		|    |--------Included Bot-----------|
		|/echo included bot ${Bool[${Macro.Return.Arg[3,_]}]}
		/if (${Bool[${Macro.Return.Arg[3,_]}]}) {
			|/echo chatsender is ${ChatSender}
			| Check that you're not assisting on a corpse, pet or pc
      /if (!${Bool[${Spawn[id ${MobID}].Type.NotEqual[Corpse]}]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
				/if (${Toggle-PvP}) /goto :assistAnyway
				/if (${Debug} || ${Debug_Assists}) /echo [${Spawn[id ${MobID}].CleanName}] is not an acceptable TargetType
			} else {
		
				:assistAnyway
        |/echo ea
				| Check target distance.
				/if (${Spawn[${MobID}].Distance} > ${MaxAssistDistance}) {
					/docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
				} else {
					| Wait to catch up to group, before engage.
					|/echo ${Following}   ${Spawn[${FollowTarget}].Distance}   ${LeashLength}   ${Me.Moving}
					/if (${Following} && ${Spawn[${FollowTarget}].Distance} > ${LeashLength} && ${Me.Moving}) {
					|/echo distloop
						/declare Assist_Moving_Timer timer local 100
						:movetoAssist_Loop
						/call Background_Events
						/if (${Spawn[=${FollowTarget}].Distance} > ${LeashLength} && ${Assist_Moving_Timer}) /goto :movetoAssist_Loop
					}
					|/echo os
					| Disengage follow.
					/if (${Stick.Active}) /squelch /stick off
					/if (${NetAdvPath.Status}==1) /squelch /netfollow off
					| Set Assist Variables.
					/varset Assisting TRUE
					/varset AssistTarget ${MobID}
					/varset AllowControl FALSE
          /varset isEnraged FALSE
					| If I am the assist caller, or I am feigning, don't allow macro control.
					/if (${ChatSender.NotEqual[${Me.CleanName}]} && !${Me.Feigning}) {
					|/echo inif
					  /varset AllowControl TRUE
						| Output to user
						/if (${Verbosity}) /docommand ${ChatToggle} Attacking - ${AssistTarget} - ${Spawn[${AssistTarget}].CleanName}
						|/call check_AssistStatus
						| Target AssistTarget.
						|&& (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]})
						/if (${Target.ID} != ${AssistTarget} && (!${Bool[${Me.Casting}]} || ${Me.Class.ShortName.Equal[BRD]})) /call TrueTarget ${AssistTarget}
						|/echo ${AssistTarget}
            /if (${Me.Pet.ID}) {
              /if (${Debug} || ${Debug_Assists}) /echo Pet Check: Sending pet in
              /pet attack ${AssistTarget}
            }
            |/echo ${AssistType}
            /if (${AssistType.Equal[melee]}) {
              /if (${Debug} || ${Debug_Assists}) /echo melee on
              /if (${Int[${AssistDistanceSetting}]}) {
                /varset AssistStickDistance ${AssistDistanceSetting}
              } else {
                /if (${AssistDistanceSetting.Equal[MaxMelee]} || ${AssistDistanceSetting.Equal[auto]}) /varset AssistStickDistance ${Math.Calc[${Spawn[${Target.ID}].MaxRangeTo}*.75].Int}
              }
              | Check MaxRangeTo, make sure it is not out of bounds.
              /if (${AssistStickDistance} > 25) /varset AssistStickDistance 25
              /call StickToAssistTarget
              /if (${Me.Class.ShortName.Equal[Rog]} && ${Defined[strikeDisc]} && ${Bool[${strikeDisc}]}) {
                /call check_rogStrike
              } else {
                /attack on
              }
            } else /if (${AssistType.Equal[ranged]}) {
              /if (${Debug} || ${Debug_Assists}) /echo autofire on
              /squelch /face fast id ${AssistTarget}
              /if (!${Me.AutoFire}) /autofire on
              /if (${Target.Distance} < 35 || ${Target.Distance} > ${RangedStickDistance}) /squelch /stick hold moveback ${RangedStickDistance}
            }
					/if (${Debug} || ${Debug_Assists}) /echo Assist=${Assisting} || AssistTarget=${AssistTarget} || AllowControl=${AllowControl}
				  }
        }
		  }
	  }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_Assist -|
/RETURN

|--------------------------------------------------------------------|
|- Backs off an AssistTarget, and labels them as a 'PauseTarget'.	-|
|- You must manually call engage to re-engage a PauseTarget.		-|
|--------------------------------------------------------------------|
|- Use, '/backoff', or, 'Back off' in /tell, /group, or /bc.		-|
|--------------------------------------------------------------------|
#event BackOff "<#1#> Back Off"
SUB Event_BackOff(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_BackOff ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/if (${Assisting} && ${AllowControl} && ${Verbosity}) /docommand ${ChatToggle} Backing off.
		/varset PauseTarget ${Target.ID}
		/call AssistOff
	}
/if (${Debug} || ${Debug_Assists}) /echo <== Event_BackOff -|
/RETURN

|----------------------------------------------------------------|
|- Turns off Assist_Functions, and resets assisting variables.	-|
|----------------------------------------------------------------|
SUB AssistOff
/if (${Debug} || ${Debug_Assists}) /echo |- AssistOff -| ==>
	| Interrupt spells, turn off attack.
	|/echo in assistoff
	|/if (${AllowControl}) {
		/if (${Me.Casting.ID} && !${Spell[${Me.Casting}].SpellType.Find[beneficial]} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
		/if (${Me.Combat})    /attack off
		/if (${Me.AutoFire})  /autofire off
    /delay 5 !${Me.Combat} && !${Me.AutoFire}
	|}
  /if (${Me.Pet.ID}) {
    /pet back off
    /pet ghold on
  }

	/varset Assisting FALSE
	/varset AssistTarget 0
	/varset AllowControl FALSE
  /varset isEnraged FALSE

  /if (${Stick.Status.Equal[ON]}) /squelch /stick off
  /if (${Debuff_Targets.Size})    /deletevar Debuff_Targets
  /if (${DoT_Targets.Size})       /deletevar DoT_Targets
	/if (${Defined[use_QuickBurns]}) /deletevar use_QuickBurns
  /if (${Defined[use_LongBurns]}) /deletevar use_LongBurns
	/if (${Defined[use_FullBurns]}) /deletevar use_FullBurns

  /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
  | no epic burns variable as its only a single item cast
	|/varset use_TargetAE FALSE
  |/varset use_PBAE FALSE
	| Check loot, and reacquire follow.
  /if (${Auto_Loot}) /doevents YourKill
	/if (${Following}) /call AquireFollow

/if (${Debug}) {
	/echo Assisting: [${Assisting}] || AssistTarget: [${AssistTarget}]
	/echo |- AssistOff -| <==
}
/RETURN

|----------------------------------------|
|- Engages /stick on an AssistTarget.	-|
|----------------------------------------|
SUB StickToAssistTarget
/if (${Debug} || ${Debug_Assists}) /echo |- StickToAssistTarget ==> assistPoint ${AssistStickPoint} dist ${AssistStickDistance}
		/if (${AssistStickPoint.Equal[behind]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behind [${AssistStickDistance}].
			/squelch /stick moveback snaproll 15 uw
			/delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[behindonce]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick behindonce [${AssistStickDistance}].
			/squelch /stick moveback snaproll 15 uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behindonce ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[pin]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick pin [${AssistStickDistance}].
			/squelch /stick moveback snaproll 15 uw
			/delay 20 ${Stick.Stopped}
			/squelch /stick hold moveback pin ${AssistStickDistance} uw
		} else /if (${AssistStickPoint.Equal[front]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick front [${AssistStickDistance}].
			/squelch /stick hold front ${AssistStickDistance} uw
      /delay 20 ${Stick.Stopped}
		} else /if (${AssistStickPoint.Equal[!front]}) {
			/if (${Debug} || ${Debug_Assists}) /echo Attempting to stick !front [${AssistStickDistance}].
			/squelch /stick moveback snaproll 15 uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch stick hold moveback !front ${AssistStickDistance} uw
		} else {
			/if (${Debug} || ${Debug_Assists}) /echo [${AssistStickPoint}] is not a valid stick point.  Defaulting to stick behind.
			/squelch /stick moveback snaproll 15 uw
      /delay 20 ${Stick.Behind} && ${Stick.Stopped}
			/squelch /stick hold moveback behind ${AssistStickDistance} uw
		}
/if (${Debug} || ${Debug_Assists}) /echo <== StickToAssistTarget -|	
/RETURN


|----------------------------------------------------|
|- Checks range and combat status while assisting.	-|
|----------------------------------------------------|
SUB check_AssistStatus
/if (${Debug} || ${Debug_Assists}) {
	/echo |- check_AssistStatus ==>
	/echo AssistTarget = [${AssistTarget}] || AllowControl = [${AllowControl}] || Distance check = [${Spawn[id ${AssistTarget}].Distance} < ${MaxAssistDistance}] || TargetType = [${Spawn[id ${AssistTarget}].Type}]
}
  /if (${Me.Feigning}) {
    |/if (${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) {
    |  /if (${Debug} || ${Debug_Assists}) /echo I'm feigned, skipping assist checks
    |  /return
    |} else
    /stand
  }
  |if target is dead or simply doesnt exist assistoff
  /if (!${Bool[${Spawn[id ${AssistTarget}].Type.NotEqual[Corpse]}]}) {
    /if (${Debug} || ${Debug_Assists}) /echo Target doesnt exist or is dead, AssistOff
    /call AssistOff
    /return
  }
| If the character is macro controlled
  /if (${AllowControl} && ((${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]}))) {
    | Check Target: this only forces retarget for melee/autofire bots and lets casters/priests do their thing
    /if (${Target.ID} != ${AssistTarget} && !${Bool[${Me.Casting}]} && (${AssistType.Equal[melee]} || ${AssistType.Equal[ranged]})) /call TrueTarget ${AssistTarget}
    |----- Melee functions-------------------------------------------------------------------------------------------------------------|
      /if (${AssistType.Equal[Melee]}) {
        |- mob is or was enraged
        /if (${isEnraged}) {
          /call check_IsLookingAt
          /if (${Macro.Return}) {
            /if (${Debug} || ${Debug_Assists}) /echo I am not behind the target. Stopping attack.
            /attack off
            /return
          }
        }
        | If I am target of target and not sticking to front(tank) attack/stick off
        | If I am < 50% life turn off attack (unless tank/stick front)
        | why does not moving matter.. && !${Bool[${Int[${Target.Speed}]}]}
        /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]} && ${AssistStickPoint.NotEqual[front]}) {
          /if (${Me.PctHPs} < 80 ) /attack off
          /stick off
        } else {
          | If the mob is not targeting me
          /if (!${Me.Combat}) /attack on
          /if (!${Stick.Active} || (${Target.Distance} > ${AssistStickDistance})) {
            /doevents getCloser
            /call StickToAssistTarget
          }
        }
    |----- Ranged functions-------------------------------------------------------------------------------------------------------------||
    } else /if (${AssistType.Equal[Ranged]}) {
      /squelch /face fast id ${AssistTarget}
      |dont play chase
      /if (${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
        /stick off
        /squelch /autofire off
      } else /if (${Target.Distance} < 35 || ${Target.Distance} > ${RangedStickDistance}) {
        /squelch /stick hold moveback ${RangedStickDistance}
        /if (!${Me.AutoFire}) /autofire on
      }
    }
  }
  /if (!${Me.Casting.ID} && ${Me.Combat} && ${Abilities2D.Size[1]}) /call CombatAbilities
/if (${Debug} || ${Debug_Assists}) /echo <== check_AssistStatus -|
/RETURN

|------------------------------------------------|
|- Uses combat abilities, AAs, and disciplines.	-|
|- e3_casting is not used, for performance, as these are short recast low impact only
|------------------------------------------------|
SUB CombatAbilities
|/varset Debug TRUE
  /if (${Debug} || ${Debug_Assists}) /echo |- CombatAbilities ==>
  /declare i int local
| Prereq/priorities
  /if (${doTaunt} && ${Me.AbilityReady[Taunt]} && ${Me.PctAggro}< 100) /doability Taunt
  /if (${smartTaunt} && ${Me.AbilityReady[Taunt]} && ${Bool[${Me.TargetOfTarget.Class.ShortName}]} && ${Select[${Me.TargetOfTarget.Class.ShortName},WAR,PAL,SHD]}==0) {
    /bc Taunting ${Target.CleanName} : ${Me.TargetOfTarget.Class.ShortName} ${Me.TargetOfTarget.Name} has aggro
    /doability Taunt
  }


  /if (${AutoEvade}) /call AutoEvade

  /for i 1 to ${Abilities2D.Size[1]}
    |/echo ${Abilities2D[${i},${iCastName}]}
    |skip the ability if current aggro is less than defined PctAggro or the taret is not a Named
    /if (${Abilities2D[${i},${iPctAggro}]} > 0 && (${Me.PctAggro} < ${Abilities2D[${i},${iPctAggro}]} || !${Target.Named})) {
      /if (${Debug} || ${Debug_Assists}) /echo skipping ${Abilities2D[${i},${iCastName}]}, current aggro ${Me.PctAggro} defined ${Abilities2D[${i},${iPctAggro}]} named ${Target.Named}
      /next i
    }
    /if (${Abilities2D[${i},${iCastType}].Equal[Ability]}) {
      /if (${Abilities2D[${i},${iCastName}].Equal[Bash]}) {
        /if (!${Select[${Me.Inventory[Offhand].Type},Shield]} && !${Me.AltAbility[2 Hand Bash]}) /next i
      }
      /if (${Me.AbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        /doability "${Abilities2D[${i},${iCastName}]}"
        /delay 2 !${Me.AbilityReady[${Abilities2D[${i},${iCastName}]}]}
      }
      |- AA, only those with 0 cast time can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[AA]}) {
      /if (${Me.AltAbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        /casting "${Abilities2D[${i},${iCastName}]}" alt
        /delay 3 !${Me.AltAbilityReady[${Abilities2D[${i},${iCastName}]}]}
        |/echo casting ${abilityName}
      }
      |- Discipline, no disc with a duration can go here
    } else /if (${Abilities2D[${i},${iCastType}].Equal[Disc]}) {
    |/echo car ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbility[${abilityName}]} ${abilityName} ${Me.CombatAbilityReady[${abilityName}]} ${Me.CombatAbilityReady[Backstab]}
      /if (${Me.CombatAbilityReady[${Abilities2D[${i},${iCastName}]}]}) {
        |/echo ${Abilities2D[${i},${iCastName}]} myend ${Me.PctEndurance} >= minend ${Abilities2D[${i},${iMinEnd}]} and endcost ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}
        /if (${Me.PctEndurance} >= ${Abilities2D[${i},${iMinEnd}]} && ${Spell[${Abilities2D[${i},${iCastName}]}].EnduranceCost} <= ${Me.Endurance}) {
          |/echo ready ${abilityName} ${Me.CombatAbilityReady[${abilityName}]}
          /disc ${Abilities2D[${i},${iCastName}]}
          /delay 3 !${Me.CombatAbilityReady[${Abilities2D[${i},${iCastName}]}]}
          |/echo post disc
        }
      }
    }
  /next i

  /if (${Debug} || ${Debug_Assists}) /echo <== CombatAbilities -|
|/varset Debug FALSE
/RETURN

|--------------------------------------------------------|
| Check if the target is facing me(primarily for enrage) |
|--------------------------------------------------------|
SUB check_IsLookingAt
/if (${Debug} || ${Debug_Assists}) /echo |- CheckIsLookingAt ==>
	/declare MobHeading int local 0 
	/declare HeadingToPC int local 
	/declare DeltaX local float 
	/declare DeltaY local float 
	/declare HeadingDelta local float
	/declare FacingMe int local 0
	/varcalc MobHeading ${Spawn[id ${AssistTarget}].Heading.Degrees}
	/varcalc DeltaX ${Spawn[id ${Me.ID}].X}-${Spawn[id ${AssistTarget}].X}
	/varcalc DeltaY ${Spawn[id ${Me.ID}].Y}-${Spawn[id ${AssistTarget}].Y}

	/if (${DeltaX}>0) {
		/varcalc HeadingToPC ${Math.Atan[${DeltaY}/${DeltaX}]}+270 
	} else /if (${DeltaX}<0) {
		/varcalc HeadingToPC ${Math.Atan[${DeltaY}/${DeltaX}]}+90 
	} else {
		/if (${DeltaY}>0) {
			/varcalc HeadingToPC 90 
		} else {
			/varcalc HeadingToPC 270 
		}
	}
	/varcalc HeadingDelta ${Math.Abs[${HeadingToPC}-${MobHeading}]}
	/if (${HeadingDelta}<4 || ${HeadingDelta}>356) {
		/if (${Debug} || ${Debug_Assists}) /echo |- CheckIsLookingAt -| [${Spawn[id ${AssistTarget}].CleanName}] is facing me.
		/varset FacingMe 1
	}
/if (${Debug} || ${Debug_Assists}) /echo <== CheckIsLookingAt -|
/RETURN ${FacingMe}

|------------------------------------------------------------|
|- Turns off attack, when a mob you're attacking enrages.	-|
|------------------------------------------------------------|
#event EnrageOn "#1# has become ENRAGED."
SUB Event_EnrageOn(line, RageOn)
  /if (${Debug} || ${Debug_Assists}) /echo |- Event_EnrageOn ==> Enrage ON, turning attack OFF
  /varset isEnraged bool outer TRUE
  /if (${Me.Combat} && ${Target.ID} == ${Spawn[${RageOn}].ID} && ${Target.PctHPs} < 20) {
    /call check_IsLookingAt
    /if (${AssistStickPoint.Equal[behind]} && (${Stick.Behind} || ${Stick.MoveBehind}) && !${Me.TargetOfTarget.Name.Equal[${Me.Name}]} && !${Macro.Return}) /return
    /attack off
    /if (${Me.Pet.ID}) {
      /pet ghold on
      /pet back off
    }
  }
/RETURN
|------------------------------------------------|
|- Turns attack back on, after enrage is over.	-|
|------------------------------------------------|
#event EnrageOff "#1# is no longer enraged."
SUB Event_EnrageOff(line, RageOn)
  /if (${Debug} || ${Debug_Assists}) /echo |- EnrageOff ==> Enrage OFF, turning attack ON
  /if (${isEnraged} && ${Target.ID} == ${Spawn[${RageOn}].ID}) {
    /attack on
    /if (${Me.Pet.ID}) /pet attack
    /varset isEnraged FALSE
  }
/RETURN

|------------------------------------------------|
|- Adjust Melee distance if too far away msg   	-|
|------------------------------------------------|
#EVENT getCloser "Your target is too far away, get closer!"
SUB EVENT_getCloser
/if (${Debug} || ${Debug_Assists}) /echo |- EVENT_getCloser ==>
	/if (${Assisting} && ${AllowControl} && !${getCloserTimer}) {
		| If the spawn has moved, stick back to the target
    /doevents flush EVENT_getCloser
		/if (${Spawn[id ${AssistTarget}].Distance} <= ${AssistStickDistanceSetting} && ${AssistStickDistanceSetting} > 8) /varcalc AssistStickDistance ${AssistStickDistance} - 3
	}
/if (${Debug} || ${Debug_Assists}) /echo <== EVENT_getCloser -| AssistStickDistance has been set to [${AssistStickDistance}]
/RETURN

|------------------------------------------------------------|
|- dont use this, its dumb and expensive
|------------------------------------------------------------|
SUB check_AutoAssist
/if (${Debug} || ${Debug_Assists}) /echo |- check_AutoAssist ==>
	/if (!${Assisting}) {
		|/echo ${auto_AssistOn} engagepct ${EngagePct}
		/if ((${Target.ID} && ${Me.Combat}) || (${Target.ID} && ${Spell[${Me.Casting.ID}].SpellType.Equal[Detrimental]})) {
			/if (${Target.PctHPs} < ${EngagePct} && ${Select[${Spawn[id ${Target.ID}].Type},${AcceptableTargetTypes}]}) {
			  /if ((!${Bool[${PauseTarget}]} || ${Target.ID} != ${PauseTarget}) && (!${Bool[${Me.Pet.ID}]} || ${Target.ID} != ${Me.Pet.ID})) {
          /if (${Debug} || ${Debug_Assists}) /echo I am attacking, engaging Assist functions
          /assistme
          |/call EVENT_Assist "<${Me.CleanName}> Assist on ${Target.ID}" ${Me.CleanName} ${Target.ID}
        }
			}
    }
    |why do you need to check other netbots when one of them will engage all via event_assist above??
		|** Check all other netbots
		} else {
			/declare i int local
			/for i 1 to ${NetBots.Counts}
				/if (${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID} && ${NetBots[${NetBots.Client.Arg[${i}, ]}].Attacking} || ${Spell[${NetBots[${NetBots.Client.Arg[${i}, ]}].Casting}].SpellType.Equal[Detrimental]}) {
					/if (!${Spawn[id ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}].LineOfSight}) {
						/if (${Debug} || ${Debug_Assists}) /echo I do not have line of sight on [${Spawn[id ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}].CleanName}].  Holding auto assist
					} else {
						/if (${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetHP} < ${EngagePct} && ${Select[${Spawn[id ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}].Type},${AcceptableTargetTypes}]} && ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID} != ${PauseTarget} && ${Spawn[id ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}].Distance} < ${MaxAssistDistance}) {
              /call TrueTarget ${msgTarget}
              |/echo ${NetBots[${NetBots.Client.Arg[${i}, ]}].Name} targ ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID} thp ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetHP} attack ${NetBots[${NetBots.Client.Arg[${i}, ]}].Attacking} cast ${Spell[${NetBots[${NetBots.Client.Arg[${i}, ]}].Casting}].SpellType.Equal[Detrimental]}
							/squelch /target id ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}
							/delay 3 ${Target.BuffsPopulated}
							/if (${Target.PctHPs} < ${EngagePct}) {
							  /if (${Debug} || ${Debug_Assists}) /echo ${NetBots[${NetBots.Client.Arg[${i}, ]}]} is attacking, assisting...
							  /call EVENT_Assist PLACEHOLDER ${NetBots.Client.Arg[${i}, ]} ${NetBots[${NetBots.Client.Arg[${i}, ]}].TargetID}
              }
						}
					}
				}
			/if (!${Assisting}) /next i
		} **|
	}
/if (${Debug} || ${Debug_Assists}) /echo <== check_AutoAssist -|
/RETURN

|--------------------------------------------------------|
|- Casts direct damage spells on a specified target.	  -|
|--------------------------------------------------------|
SUB check_Nukes
|/varset Debug true
/if (${Debug} || ${Debug_Assists}) /echo |- check_Nukes ==>
	/if (${Nukes2D.Size[1]} && ${Assisting} && ${use_Nukes}) {
	  /declare castIndex int local
	  /declare s int local
    /declare m int local

   /for s 1 to ${Nukes2D.Size[1]}
    /varset castIndex ${s}
    /if (${Debug} || ${Debug_Assists}) /echo ${s} ${castIndex} ${Nukes2D[${castIndex},${iCastName}]}

    | skip casting if a delay is defined
    /if (${castDelay${Nukes2D[${castIndex},${iCastID}]}}) {
      /if (${Debug} || ${Debug_Assists}) /echo skipping nukes, /delay from previous cast
      /return
    }
    |if im burning and /noburn is set, skip the spell
    |if noburn and rotate is defined, then skip this spell and set lastSuccessfulCast to this spell, so the next spell can be used
    /if ((${use_QuickBurns} || ${use_LongBurns} || ${use_FullBurns}) && ${Nukes2D[${castIndex},${iNoBurn}]}) {
      /if (${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
        /if (${Debug} || ${Debug_Assists}) /echo setting lastSuccessfulCast to ${lastSuccessfulCast} because noburn and rotate defined
        /varset lastSuccessfulCast ${Nukes2D[${castIndex},${iCastName}]}
      } else {
        /if (${Debug} || ${Debug_Assists}) /echo \ay skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i am burning and /NoBurn is set
        /goto :skipCast
      }
    }
    | if /rotate is defined, and the current array index was the last successful cast, skip to next index
    /if (${Nukes2D[${castIndex},${iCastName}].Equal[${lastSuccessfulCast}]} && ${Nukes2D[${castIndex},${iRotate}]} && ${Nukes2D.Size[1]} > 1) {
      /if (${Debug} || ${Debug_Assists}) /echo skipping cast of ${Nukes2D[${castIndex},${iCastName}]} /rotate is defined
      /goto :skipCast
    }
    | if i have Gift Of Mana find a spell with /GoM
    /if (${Bool[${Me.Song[Gift of Mana].ID}]}) {
      /for m 1 to ${Nukes2D.Size[1]}
        /if (${Nukes2D[${m},${iGiftOfMana}]}) {
          /varset castIndex ${m}
          /varset lastSuccessfulCast 0
        }
      /next m
    }
    |if i have aggro and /noaggro is set, skip the spell
    /if (${Nukes2D[${castIndex},${iNoAggro}]} && ${Me.TargetOfTarget.CleanName.Equal[${Me.CleanName}]}) {
      /if (${Debug} || ${Debug_Assists}) /echo \ay skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have aggro and /noaggro is set
      /goto :skipCast
    }
    |only cast if my aggro is greater than PctAggro and the target is a Named
    /if (${Nukes2D[${castIndex},${iPctAggro}]} > 0 && (${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} || !${Target.Named})) {
      /if (${Debug} || ${Debug_Assists}) /echo \ay skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, my aggro ${Me.PctAggro} is < PctAggro ${Nukes2D[${castIndex},${iPctAggro}]}
      /if (${Debug} || ${Debug_Assists}) /echo ${Nukes2D[${castIndex},${iPctAggro}]} ${Me.PctAggro} < ${Nukes2D[${castIndex},${iPctAggro}]} !${Target.Named}
      /goto :skipCast
    }

    | if i have spell damage reduction recourse and the mob has more than 25% health or i have < 10% mana, skip cast.  cast anyway if i have GoM to avoid losing it
    /if ((${Me.Buff[Weave of Weakness].ID} || ${Me.Buff[Flames of Weakness].ID}) && (${Target.PctHPs} > 25 || ${Me.PctMana} < 10) && !${Bool[${Me.Song[Gift of Mana].ID}]} && !(${use_QuickBurns} || ${use_LongBurns} || ${use_FullBurns})) {
      /if (${Debug} || ${Debug_Assists}) /echo \ay skipping cast of ${Nukes2D[${castIndex},${iCastName}]}, i have spell damage reduction
      /goto :skipCast
    }

    /call check_Ready "Nukes2D" ${castIndex}
    /if (${Macro.Return}) {
      /call check_Mana "Nukes2D" ${castIndex}
      /if (${Macro.Return}) {
        /call check_Distance ${AssistTarget} ${Nukes2D[${s},${iMyRange}]}
        /if (${Macro.Return}) {
          |/squelch /face fast id ${AssistTarget}
          /call e3_Cast ${AssistTarget} "Nukes2D" "${castIndex}"
        }
      }
    }
    |if the nuke was succesful, see if there is a delay defined and create a timer which locks out all nukes for the duration
    |/echo delay ${Nukes2D[${castIndex},${iDelay}]}  castsuccess ${Select[${castReturn},CAST_SUCCESS]}
    /if (${Bool[${Nukes2D[${castIndex},${iDelay}]}]} && ${Select[${castReturn},CAST_SUCCESS]}) {
      /if (${Debug} || ${Debug_Assists}) /echo delaying for ${${ArrayName}[${castIndex},${iDelay}]}
      /if (${Defined[castDelay${Nukes2D[${castIndex},${iCastID}]}]}) {
        /varset castDelay${Nukes2D[${castIndex},${iCastID}]} ${Nukes2D[${castIndex},${iDelay}]}
        |/echo reset timer castDelay${Nukes2D[${castIndex},${iCastID}]}
      } else {
        /call CreateTimer "castDelay${Nukes2D[${castIndex},${iCastID}]}" "${Nukes2D[${castIndex},${iDelay}]}"
        |/echo created timer castDelay${Nukes2D[${castIndex},${iCastID}]}
      }
    }
	:skipCast
    /if (!${ActionTaken}) /next s
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_Nukes -|
|/varset Debug false
/RETURN

|----------------------------------------------------|
|- interrupts casting if target is invalid (dead)   -|
|----------------------------------------------------|
|Sub check_detrimental_target
|/if (${Debug} || ${Debug_Assists}) /echo |- check_detrimental_target ==>
|  /delay 1
|/if (${Debug} || ${Debug_Assists}) /echo <== check_detrimental_target -|
|/RETURN ${Bool[${Spawn[id ${Target.ID}].Type.NotEqual[Corpse]}]}

|----------------------------------------------------|
|- Enables Debuffing on a Target                    -|
|----------------------------------------------------|
SUB check_Debuffs
/if (${Debug} || ${Debug_Assists}) /echo |- check_Debuffs ==>
  |/echo Debuff_Targets ${Debuff_Targets.Size}
  /if (${Command_Debuffs2D.Size[1]} && ${Debuff_Targets.Size}) /call Debuffs_OnCommand
	/if (${Assisting} && ${Assist_Debuffs2D.Size[1]}) /call cast_longTermSpells "${AssistTarget}" "Assist_Debuffs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_Debuffs -|
/RETURN

|----------------------------------------------------|
|- Enables Dots on a Target                         -|
|----------------------------------------------------|
SUB check_DoTs
/if (${Debug} || ${Debug_Assists}) /echo |- check_DoTs ==>
  /if (${Command_DoTs2D.Size[1]} && ${DoT_Targets.Size}) /call Dots_OnCommand
	/if (${Assisting} && ${Assist_DoTs2D.Size}) /call cast_longTermSpells "${AssistTarget}" "Assist_DoTs2D"
/if (${Debug} || ${Debug_Assists}) /echo <== check_DoTs -|
/RETURN

|----------------------------------------------------|
|- Casts debuffs and dots on various assist targets -|
|----------------------------------------------------|
SUB cast_longTermSpells(int spellTarget, string ArrayName)
  /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] ==>
|/varset Debug_Assists TRUE
  /declare spellTimer string local
  /declare spellRecast int local 0
  /declare i int local

	|/echo ${ArrayName} ${${ArrayName}.Size[1]}
	/for i 1 to ${${ArrayName}.Size[1]}
		/varset spellTimer spellTimer_${spellTarget}_${ArrayName}-${i}
		/varset resistCount resist__${spellTarget}_${ArrayName}-${i}
    |/echo st ${spellTimer} ${${spellTimer}} ${Bool[${${spellTimer}}]}

		/if (${${spellTimer}}) {
        /if (${Debug} || ${Debug_Assists}) /echo |- cast_longTermSpells - [${ArrayName}] -|	Waiting to recast ${${ArrayName}[${i},${iCastName}]}.
    } else {

      |check if the target is already debuffed, and if the duration is longer than the recast time
      /if (${spellTarget} != ${Target.ID}) {
        /call TrueTarget ${spellTarget}
        /delay 5 ${Target.BuffsPopulated}
      }
      /if (${Debug} || ${Debug_Assists}) /echo ${Target.Name} ${Bool[${Select[${ArrayName},Command_Debuffs2D,Assist_Debuffs2D]}]} dura ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]}
      /if (!${Bool[${${spellTimer}}]} && ${Bool[${Select[${ArrayName},Command_Debuffs2D,Assist_Debuffs2D]}]} && ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]}/1000 > ${longTermRecastTime}) {
        /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]}/1000 - ${longTermRecastTime}
        /call createTimer "${spellTimer}" "${spellRecast}s"
        /echo \ag ${Target.CleanName} already debuffed ${${ArrayName}[${i},${iSpellName}]} ${spellRecast}
        /goto :skipCast
      }

      :attemptCast
      | cast the spell
      /call check_Ready "${ArrayName}" ${i}
      /if (${Macro.Return}) {
        /call check_Mana "${ArrayName}" ${i}
        /if (${Macro.Return}) {
          /call check_Distance ${spellTarget} ${${ArrayName}[${i},${iMyRange}]}
          /if (${Macro.Return}) {
            /call e3_Cast ${spellTarget} "${ArrayName}" "${i}"
          } else /goto :skipCast
        } else /goto :skipCast
      } else /goto :skipCast

      /if (${Debug} || ${Debug_Assists}) /echo castretun ${castReturn} res ${Cast.Result}
      /if (${Debug} || ${Debug_Assists}) /echo maxtries ${${ArrayName}[${i},${iMaxTries}]} rescount ${resistCount} ${${resistCount}}
      /if (${Debug} || ${Debug_Assists}) /echo castname ${${ArrayName}[${i},${iCastName}]} stored ${Cast.Stored.Name} return  ${castReturn}
      | If the spell was resisted.
      /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_RESIST]}) {
        /if (!${Defined[${resistCount}]}) {
          /declare ${resistCount} int outer 1
        } else {
          /varcalc ${resistCount} ${${resistCount}}+1
        }

        /if (${${resistCount}} < ${${ArrayName}[${i},${iMaxTries}]}) {
          /goto :attemptCast
        } else {
          /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
          /call createTimer "${spellTimer}" "1m"
        }
      | If the spell did not take hold.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_TAKEHOLD]}) {
        /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
        /call createTimer "${spellTimer}" "3m"
        | If the target was immune.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_IMMUNE]}) {
        /call createTimer "${spellTimer}" "30m"
      | If the cast was successful.
      } else /if (${Cast.Stored.Name.Equal[${${ArrayName}[${i},${iSpellName}]}]} && ${castReturn.Equal[CAST_SUCCESS]}) {
        /if (${Defined[${resistCount}]}) /deletevar ${resistCount}
        |if i dont have spell awarenes use default buff duration
        /varcalc spellRecast ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]}/1000
        /if (${spellRecast}==0) /varcalc spellRecast ${${ArrayName}[${i},${iDuration}]}*6
        |/echo ${Target.Name}  ${Target.BuffDuration[${${ArrayName}[${i},${iSpellName}]}]} ${spellRecast}
        |/echo ******** spellRecast ${spellRecast} ***********
        |duration over 90 seconds is considered long term
        /if (${spellRecast}>90) {
          /varcalc spellRecast ${spellRecast}-${longTermRecastTime}
        } else {
          /varcalc spellRecast ${spellRecast}-${shortTermRecastTime}
        }
        /if (${Debug} || ${Debug_Assists}) /echo |- longTermSpells - [${ArrayName}] -| Creating timer [${spellTimer}] -- [${spellRecast}]
        |/echo Creating timer [${spellTimer}] -- [${spellRecast}]
        /call createTimer "${spellTimer}" "${spellRecast}s"
      }
    }
  /if (!${ActionTaken}) /next i
 :skipCast
|/varset Debug_Assists FALSE
/if (${Debug} || ${Debug_Assists}) /echo <== cast_longTermSpells - [${arrayName}] -|
/RETURN

|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/Debuff' or 'Debuffs on ${Target.ID}' in /tell, /group, or /bc.					 -|
|---------------------------------------------------------------------------------------------|
#EVENT DebuffsOn "<#1#> Debuffs on #2#"
#EVENT DebuffsOn "#1# tells you#*#Debuffs on #2#'"
 SUB Event_DebuffsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOn ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		| Check if this character has debuffs to cast.
		/if (${Command_Debuffs2D.Size[1]}) {
			| Check if the target is a corpse
			/if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
				/echo Unacceptable Target ${Spawn[id ${MobID}].Type}
			} else {
				| Check range to the debuff target.
				/if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
					/docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
				} else {
					| Check if the Debuff_Targets array is already defined
					/if (${Debuff_Targets.Size}) {
						/declare i int local
						/for i 1 to ${Debuff_Targets.Size}
							/if (${Debuff_Targets[${i}].Arg[1,_]} == ${MobID}) {
								/docommand ${ChatToggle} I am already debuffing ${Spawn[id ${MobID}].CleanName}.
								/goto :Skip
							}
						/next i
					}
					| If the target was not found on the DebuffTargets array, add it.
					/call BuildArray Debuff_Targets ${MobID}_${Zone.ID}
          /docommand ${ChatToggle} -- Debuffing ${Spawn[${MobID}].CleanName} --
					|/echo Debuff_Targets ${Debuff_Targets.Size}
				}
			}
		}
	}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOn -|
/RETURN

|------------------------------------------------|
|- Cancels debuffs on command on all targets. 	-|
|------------------------------------------------|
#EVENT DebuffsOff "<#1#> End debuffs"
#EVENT DebuffsOff "<#1#> End debuffs on #2#"
| add LoS check to assist for detrimental spells
|${Spell[${Me.Casting.ID}].SpellType.Equal[Detrimental]} && !${Spawn[id ${Target.ID}].LineOfSight}
SUB event_DebuffsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DebuffsOff ==>
	| If I'm casting debuffs
	/if (${Debuff_Targets.Size[1]}) {
		| -Verify the event
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|
		/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		
			/if (${Defined[${MobID}]}) {
				/docommand ${ChatToggle} Ending Debuffs on ${Spawn[id ${MobID}].CleanName}.
				/call RemoveArrayElement Debuff_Targets ${MobID}_${Zone.ID}
			} else {
				/docommand ${ChatToggle} Ending Debuffs on all targets.
				/deletevar Debuff_Targets
			}
		}
	}
/if (${Debug} || ${Debug_Assists}) /echo <== event_DebuffsOff -|
/RETURN



|--------------------------------------------------------------------|
|- Casts Command_Debuffs on targets listed in DebuffTargets array.	-|
|--------------------------------------------------------------------|
SUB Debuffs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- Debuffs_OnCommand ==>
	/declare targetID int local
	/declare targetZone int local
	/declare i int local
	/for i 1 to ${Debuff_Targets.Size}
		/varset targetID ${Debuff_Targets[${i}].Arg[1,_]}
		/varset targetZone ${Debuff_Targets[${i}].Arg[2,_]}
		| Check Mob Zone
		/if (${Zone.ID} != ${targetZone}) {
			/call RemoveArrayElement Debuff_Targets ${Debuff_Targets[${i}]}
		} else {
			| If the Debuff_Target has died, remove them from the array.
      /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
				/call RemoveArrayElement Debuff_Targets ${Debuff_Targets[${i}]}
			} else {
			  |/echo castingDebuffOnCommand
				/call cast_longTermSpells "${targetID}" "Command_Debuffs2D"
			}
		}
	/if (!${ActionTaken}) /next i
/if (${Debug} || ${Debug_Assists}) /echo <== Debuffs_OnCommand -|
/RETURN


|---------------------------------------------------------------------------------------------|
|- Validates targets and adds them to a list of Debuff_Targets. Used for Debuffs_OnCommand.	 -|
|---------------------------------------------------------------------------------------------|
|- Use, '/DoTs' or 'DoTs on ${Target.ID}' in /tell, /group, or /bc.							 -|
|---------------------------------------------------------------------------------------------|
#EVENT DoTsOn "<#1#> DoTs on #2#"
#EVENT DoTsOn "#1# tells you#*#DoTs on #2#'"
SUB Event_DoTsOn(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOn ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		| Check if this character debuffs to cast.
		/if (${Command_DoTs2D.Size}) {
			| Check if the target is a corpse
      /if (${Spawn[id ${MobID}].Type.Equal[Corpse]} || !${Select[${Spawn[id ${MobID}].Type},${AcceptableTargetTypes}]}) {
        /echo Unacceptable Target ${Spawn[id ${MobID}].Type}
			} else {
				| Check range to the DoTs target.
				/if (${Spawn[id ${MobID}].Distance} > ${MaxAssistDistance}) {
					/docommand ${ChatToggle} ${Spawn[${MobID}].CleanName} is too far away.
				} else {
					| Check if the DoT_Targets array is already defined
					/if (${DoT_Targets.Size}) {
						/declare i int local
						/for i 1 to ${DoT_Targets.Size}
							/if (${DoT_Targets[${i}].Arg[1,_]} == ${MobID}) {
								/docommand ${ChatToggle} I am already debuffing ${Spawn[id ${MobID}].CleanName}.
								/goto :Skip
							}
						/next i
					}
					| If the target was not found on the DoT Targets array, add it.
					/call BuildArray DoT_Targets ${MobID}_${Zone.ID}
				}
			}
		}
	}
	:Skip
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOn -|
/RETURN

|------------------------------------------------|
|- Cancels debuffs on command on all targets. 	-|
|------------------------------------------------|
#EVENT DoTsOff "<#1#> End DoTs"
#EVENT DoTsOff "<#1#> End DoTs on #2#"
SUB event_DoTsOff(line, ChatSender, MobID)
/if (${Debug} || ${Debug_Assists}) /echo |- event_DoTsOff ==>
	| If I'm casting debuffs
	/if (${DoT_Targets.Size}) {
		| -Verify the event
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|
		/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
			/if (${Defined[${MobID}]}) {
				/docommand ${ChatToggle} Ending DoTs on ${Spawn[id ${MobID}].CleanName}.
				/call RemoveArrayElement DoT_Targets ${MobID}_${Zone.ID}
			} else {
				/docommand ${ChatToggle} Ending DoTs on all targets.
				/deletevar DoT_Targets
			}
		}
	}
/if (${Debug} || ${Debug_Assists}) /echo <== event_DoTsOff -|
/RETURN

|----------------------------------------------------------------|
|- Casts Command_DoTs on targets listed in DoT_Targets array.	-|
|----------------------------------------------------------------|
SUB DoTs_OnCommand
/if (${Debug} || ${Debug_Assists}) /echo |- DoTs_OnCommand ==>
	/declare targetID int local
	/declare targetZone int local
	/declare i int local
	/for i 1 to ${DoT_Targets.Size}
		/varset targetID ${DoT_Targets[${i}].Arg[1,_]}
		/varset targetZone ${DoT_Targets[${i}].Arg[2,_]}
		| Check Mob Zone
		/if (${Zone.ID} != ${targetZone}) {
			/call RemoveArrayElement DoT_Targets ${DoT_Targets[${i}]}
		} else {	
			| If the DoT_Target has died, remove them from the array.
      /if (!${Bool[${Spawn[id ${targetID}].Type.NotEqual[Corpse]}]}) {
				/call RemoveArrayElement DoT_Targets ${DoT_Targets[${i}]}
			} else {
				/call cast_longTermSpells "${targetID}" "Command_DoTs2D"
			}
		}
	/if (!${ActionTaken}) /next i
/if (${Debug} || ${Debug_Assists}) /echo <== DoT_OnCommand -|
/RETURN

|----------------------------|
|- Turns AE functions on.	  -|
|----------------------------|
#event AEOn "<#1#> AllAE on#*#"
#event AEOn "#1# tells you#*#AllAE on'"
SUB Event_AEOn(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_AllAEOn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
  |/echo ${ChatSender} ret ${Macro.Return} ln ${line}
|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${PBAE_Spells2D.Size[1]} || ${TargetAE_Spells2D.Size[1]}) {
      /varset aemsg TRUE
      /docommand ${ChatToggle} Casting AE spells...
      /call Event_PBAEOn "${line}" "${ChatSender}"
      /call Event_TargetAEOn "${line}" "${ChatSender}"
      /varset aemsg FALSE
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Event_AllAEOn -|
/RETURN

|----------------------------|
|- Turns PBAE functions on.	-|
|----------------------------|
#event PBAEOn "<#1#> PBAE on#*#"
#event PBAEOn "#1# tells you#*#PBAE on'"
SUB Event_PBAEOn(line, ChatSender)
  /if (${Debug} || ${Debug_Assists}) /echo |- Event_PBAEOn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${PBAE_Spells2D.Size[1]}) {
      /varset use_PBAE TRUE
      /call TrueTarget ${NetBots[${Macro.Return.Arg[1,_]}].ID}
      /call MoveTo -Loc|${Target.Y},${Target.X}
      /if (!${aemsg}) /docommand ${ChatToggle} Casting PBAE spells...
    }
  }
  /if (${Debug} || ${Debug_Assists}) /echo <== Event_PBAEOn -|
/RETURN

|----------------------------------|
|- Turns Target AE functions on.	-|
|----------------------------------|
#event TargetAEOn "<#1#> TargetAE on#*#"
#event TargetAEOn "#1# tells you#*#TargetAE on'"
SUB Event_TargetAEOn(line, ChatSender)
  /if (${Debug} || ${Debug_Assists}) /echo |- Event_TargetAEOn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
  /varset aeTarget ${Spawn[=${ChatSender}].ID}
|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${TargetAE_Spells2D.Size[1]}) /varset use_TargetAE TRUE
    /if (!${aemsg}) /docommand ${ChatToggle} Casting Target AE spells...
  }
  | if ranger swap to ranged + bow
  /if (${Me.Class.ShortName.Equal[RNG]} && ${swapTgtAeRanged}) /call swapToRangedSet TRUE

  /if (${Debug} || ${Debug_Assists}) /echo <== Event_TargetAEOn -|
/RETURN

|----------------------------------|
|- Swap to Ranged for Target AE	  -|
|----------------------------------|
SUB swapToRangedSet(bool toggleRanged)
/echo ${toggleRanged} toggleRanged
  /if (${toggleRanged} && ${AssistType.Equal[melee]}) {
    /varset AssistType Ranged
    /varset revertRngToMelee TRUE
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${Me.Inventory[Ranged].Name}/Ranged" 1
    /delay 2
    /swap ranged
    /delay 2
  } else {
    /varset AssistType Melee
    /varset revertRngToMelee FALSE
    /swap melee
    /delay 2
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "NONE"
    /delay 2
  }
/RETURN

|--------------------------------|
|- Turns AE functions off.	    -|
|--------------------------------|
#event AEOff "<#1#> AllAE off"
#event AEOff "#1# tells you#*#AllAE off'"
SUB Event_AEOff(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_AEOff ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${PBAE_Spells2D.Size[1]} || ${TargetAE_Spells2D.Size[1]}) {
      /varset aemsg TRUE
      /docommand ${ChatToggle} Ending AE spells...
      /call Event_PBAEOff "${line}" "${ChatSender}"
      /call Event_TargetAEOff "${line}" "${ChatSender}"
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Event_AEOff -|
/RETURN

|--------------------------------|
|- Turns PBAE functions off.	  -|
|--------------------------------|
#event PBAEOff "<#1#> PBAE off"
#event PBAEOff "#1# tells you#*#PBAE off'"
SUB Event_PBAEOff(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- Event_PBAEOff ==>
  /if (${use_PBAE}) {
    /call verifyEvent "${ChatSender}" "${line}"
    /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
  |    |-----------Valid User----------|    |---------Included Bot----------|
    /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
      /if (${Me.Casting.ID}) /call interrupt
      /varset use_PBAE FALSE
      /if (!${aemsg}) /docommand ${ChatToggle} Ending PBAE.
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== Event_PBAEOff -|
/RETURN

|--------------------------------------|
|- Turns TargetAE functions off.	    -|
|--------------------------------------|
#event TargetAEOff "<#1#> TargetAE off"
#event TargetAEOff "#1# tells you#*#TargetAE off'"
SUB Event_TargetAEOff(line, ChatSender)
  /if (${Debug} || ${Debug_Assists}) /echo |- Event_TargetAEOff ==>
  /if (${use_TargetAE}) {
    /call verifyEvent "${ChatSender}" "${line}"
  |/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
  |    |-----------Valid User----------|    |---------Included Bot----------|
    /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
      /if (${Me.Casting.ID}) /call interrupt
      /varset use_TargetAE FALSE
      /varset aeTarget 0
      |/if (${Defined[aeTarget]}) /deletevar aeTarget
      /if (!${aemsg}) /docommand ${ChatToggle} Ending Target AE.
      /if (${revertRngToMelee}) /call swapToRangedSet FALSE
    }
  }
  /if (${Debug} || ${Debug_Assists}) /echo <== Event_TargetAEOff -|
/RETURN

|------------------------------------------------------------------------------------------------------------|
|- Casts spells listed in the [AE] section of the Character_Ini on mobs which wander into your AE_Radius.	-|
|------------------------------------------------------------------------------------------------------------|
SUB check_AE
/if (${Debug} || ${Debug_Assists}) /echo |- check_AE ==>
  /if ((${use_PBAE} || ${use_TargetAE}) && !${Bool[${SpawnCount[npc radius ${AE_Radius}]}]} && ${Me.CombatState.NotEqual[COMBAT]}) {
    /varset use_TargetAE FALSE
    /varset use_PBAE FALSE
    /varset aeTarget 0
    |/if (${Defined[aeTarget]}) /deletevar aeTarget
    /docommand ${ChatToggle} Ending AE: No mobs in radius ${AE_Radius}
    /if (${revertRngToMelee}) /call swapToRangedSet FALSE
  }
  /declare i int local
	/if (${use_PBAE}) {
    /for i 1 to ${PBAE_Spells2D.Size[1]}
      /call check_Ready "PBAE_Spells2D" ${i}
      /if (${Macro.Return}) {
        /call check_Mana "PBAE_Spells2D" ${i}
        /if (${Macro.Return}) {
          /squelch /face fast id ${AssistTarget}
          /call e3_Cast ${Me.ID} "PBAE_Spells2D" "${i}"
        }
      }
    /if (!${ActionTaken}) /next i
	}

  /if (${use_TargetAE}) {
    /declare useBestTarget int local
    /if (${Bool[${AssistTarget}]}) {
      |if i have an assist target, use that as ae target
      /varset useBestTarget ${AssistTarget}
    } else {
      |otherwise target the chatsender for ae
      /varset useBestTarget ${aeTarget}
    }
    /for i 1 to ${TargetAE_Spells2D.Size[1]}
      /call check_Ready "TargetAE_Spells2D" ${i}
      /if (${Macro.Return}) {
        /call check_Mana "TargetAE_Spells2D" ${i}
        /if (${Macro.Return}) {
          /call check_Distance ${useBestTarget} ${TargetAE_Spells2D[${i},${iMyRange}]}
          /echo tgt ${useBestTarget} ${TargetAE_Spells2D[${i},${iMyRange}]} ${TargetAE_Spells2D[${i},${iCastName}]}
          /if (${Macro.Return}) {
            /squelch /face fast id ${useBestTarget}
            /call e3_Cast ${useBestTarget} "TargetAE_Spells2D" "${i}"
          }
        }
      }
    /if (!${ActionTaken}) /next i
  }
/if (${Debug} || ${Debug_Assists}) /echo <== check_AE -|
/RETURN



|----------------------------------------------------------------|
|- Casts swarm pets from Mages, Necros, Shaman, and Wizards.	-|
|----------------------------------------------------------------|
#event SwarmPets "<#1#> Swarm Pets on #2#"
SUB event_SwarmPets(line, ChatSender, int swarmTarget)
/if (${Debug} || ${Debug_Assists}) /echo |- event_SwarmPets ==>
	| -Verify the event
/if (${swarmPets2D.Size[1]} > 0) {
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		| Validate target.
		/if (${SpawnCount[id ${swarmTarget}]} && ${Spawn[${swarmTarget}].Type.NotEqual[Corpse]} && ${Select[${Spawn[${swarmTarget}].Type},${AcceptableTargetTypes}]} && ${SpawnCount[pc ${ChatSender}]}) {
			| Cast swarm pets
			/declare i int local
			/for i 1 to ${swarmPets2D.Size[1]}
				/call e3_Cast ${swarmTarget} "swarmPets2D" "${i}"
				/delay 5s !${Me.Casting.ID}
			/next i
		}
	}
}
/if (${Debug} || ${Debug_Assists}) /echo <== event_SwarmPets -|
/RETURN


|----------------------------------------------------|
SUB check_Burns
|/if (${Debug} || ${Debug_Assists}) /echo |- check_Burns ==>
	/if (${use_FullBurns})  /call useBurns "FullBurns2D"
  /if (${use_LongBurns})  /call useBurns "LongBurns2D"
	/if (${use_QuickBurns}) /call useBurns "QuickBurns2D"
|/if (${Debug} || ${Debug_Assists}) /echo <== check_Burns -|
/RETURN

|----------------------------------------------------|
|- Engages EpicBurns 								                -|
|----------------------------------------------------|
#EVENT EpicBurn "<#1#> Epic Burn#2#"
#EVENT EpicBurn "#1# tells you, 'Epic Burn'"
#EVENT EpicBurn "#1# tells the group, 'Epic Burn'"
#EVENT EpicBurn "#1# tell your party, 'Epic Burn'"
SUB event_EpicBurn(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- event_EpicBurn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
      |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		/if (${Defined[EpicName]}) {
			| -Verify the event
			/call verifyEvent "${ChatSender}" "${line}"
			/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
			|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
			/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
				/docommand ${ChatToggle} Using Epic Burns...
        /casting ${EpicName} item -maxtries|3
        /delay 10 !${Me.Casting.ID}
			}
		}
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_EpicBurn -|
/RETURN

|----------------------------------------------------|
|- Engages QuickBurns listed in the Character_Ini.	-|
|----------------------------------------------------|
#EVENT QuickBurn "<#1#> Quick Burn#2#"
#EVENT QuickBurn "#1# tells you, 'Quick Burn'"
#EVENT QuickBurn "#1# tells the group, 'Quick Burn'"
#EVENT QuickBurn "#1# tell your party, 'Quick Burn'"
SUB event_QuickBurn(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- event_QuickBurn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
      |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    | Check Assisting
    /if (!${Assisting}) {
      /echo You must be attacking something to burn.
    } else {
      | Check for quickburns to cast
      /if (${Defined[QuickBurns2D]}) {
        | -Verify the event
        /call verifyEvent "${ChatSender}" "${line}"
        /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
        |    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
        /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
          /docommand ${ChatToggle} Using Quick Burns...
          /if (!${Defined[use_QuickBurns]}) /declare use_QuickBurns bool outer TRUE
        }
      }
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_QuickBurn -|
/RETURN

|----------------------------------------------------|
|- Engages LongBurns listed in the Character_Ini.	-|
|----------------------------------------------------|
#EVENT LongBurn "<#1#> Long Burn#2#"
#EVENT LongBurn "#1# tells you, 'Long Burn'"
#EVENT LongBurn "#1# tells the group, 'Long Burn'"
#EVENT LongBurn "#1# tell your party, 'Long Burn'"
SUB event_LongBurn(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- event_LongBurn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
        |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
  | Check Assisting
    /if (!${Assisting}) {
      /echo You must be attacking something to burn.
    } else {
      | Check for longburns to cast
      /if (${Defined[LongBurns2D]}) {
        | -Verify the event
        /call verifyEvent "${ChatSender}" "${line}"
        /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
        |    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
        /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
          /docommand ${ChatToggle} Using Long Burns...
          /if (!${Defined[use_LongBurns]}) /declare use_LongBurns bool outer TRUE
        }
      }
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_LongBurn -|
/RETURN

|----------------------------------------------------|
|- Engages FullBurns listed in the Character_Ini.	-|
|----------------------------------------------------|
#EVENT FullBurn "<#1#> Full Burn#2#"
#EVENT FullBurn "#1# tells you, 'Full Burn'"
#EVENT FullBurn "#1# tells the group, 'Full Burn'"
#EVENT FullBurn "#1# tell your party, 'Full Burn'"
SUB event_FullBurn(line, ChatSender)
/if (${Debug} || ${Debug_Assists}) /echo |- event_FullBurn ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
       |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
	| Check Assisting
    /if (!${Assisting}) {
      /echo You must be attacking something to burn.
    } else {
      | Check for FullBurns to cast
      /if (${Defined[FullBurns2D]}) {
        | -Verify the event
        /call verifyEvent "${ChatSender}" "${line}"
        /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
        |    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
        /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
          /docommand ${ChatToggle} Using Full Burns...
          /if (!${Defined[use_FullBurns]}) /declare use_FullBurns bool outer TRUE
        }
      }
    }
  }
/if (${Debug} || ${Debug_Assists}) /echo <== event_FullBurn -|
/RETURN

|--------------------------------------------------------------------------------|
|- Casts spells, abilities, and burns through disciplines on a given targetID.	-|
|--------------------------------------------------------------------------------|
SUB useBurns(ArrayName)
/if (${Debug} || ${Debug_Assists}) /echo |- useBurns Target=${burnTarget}- Array=${ArrayName} ==>
  |/echo useburnstart ${Time}
	/declare i int local
	/for i 1 to ${${ArrayName}.Size[1]}
    /call check_Ready "${ArrayName}" ${i}
    /if (${Macro.Return}) {
      /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastName}].Equal[Savage Spirit]}) /goto :skipCurrent
      /if (${Me.ActiveDisc.ID} && ${${ArrayName}[${i},${iCastType}].Equal[Disc]} && ${Bool[${${ArrayName}[${i},${iDuration}]}]}) /goto :skipCurrent

      /g useBurns ${Time} : ${ArrayName}-${${ArrayName}[${i},${iCastName}]}
      /call e3_Cast ${AssistTarget} "${ArrayName}" "${i}"
    } else {
      /if (${Debug} || ${Debug_Assists}) /echo NotReady: ${i} ${${ArrayName}[${i},${iCastName}]}
    }
    /delay 1
  :skipCurrent
  /if (!${ActionTaken}) /next i
/if (${Debug} || ${Debug_Assists}) /echo <== useBurns -|
/RETURN

|--------------------------------------------------------------------------------|
SUB check_lifeTap
  /if (${Debug}) /echo |- check_LifeTap ==>
  /if (${Assisting}) {
    /declare s int local
    /for s 1 to ${lifeTaps2D.Size[1]}
    /call check_Ready "lifeTaps2D" ${s}
    /if (${Macro.Return}) {
      /call check_Mana "lifeTaps2D" ${s}
      /if (${Macro.Return}) {
        /if (${Me.PctHPs} <= ${lifeTaps2D[${s},${iHealPct}]}) {
          /call check_Distance ${AssistTarget} ${lifeTaps2D[${s},${iMyRange}]}
          /if (${Macro.Return}) {
            /call e3_Cast ${AssistTarget} "lifeTaps2D" ${s}
          }
        }
      }
    }
    /if (!${ActionTaken}) /next s
  }
  /if (${Debug}) /echo <== check_LifeTap -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT NukeSet "<#1#> NukeSet #2#"
#EVENT NukeSet "[#1#] NukeSet #2#"
#EVENT NukeSet "#1# tells you, 'NukeSet #2#'"
#EVENT NukeSet "#1# tells the group, 'NukeSet #2#'"
#EVENT NukeSet "#1# tell your party, 'NukeSet #2#'"
SUB event_NukeSet(line, ChatSender, NukeArray)
/if (${Debug} || ${Debug_Assists})  /echo |- event_NukeSet ==>
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
      |multiple default options
      /if (${Select[${NukeArray},Default,Main,Nukes,Nuke]} > 0) {
        /varset NukeArray Nuke
      } else /if (${Select[${NukeArray},OFF,FALSE]} > 0) {
        /varset use_Nukes FALSE
        /echo \ag Nuking Disabled
      }

      /if (!${Ini[${Character_Ini},Nukes,${NukeArray}].Length}) {
        /if (${Debug} || ${Debug_Assists}) /echo \ay [${NukeArray}] is not configured in your bot ini, keeping existing nuke set: [${nukesToUse}]
        /return
      } else {
        /call #INItoArray "${Character_Ini},Nukes,${NukeArray}#" MyNukes
        /if (${MyNukes.Size} > 0) /call BuildSpellArray "MyNukes" "Nukes2D"
        /varset nukesToUse ${NukeArray}
        /varset use_Nukes TRUE
        /echo \ag Switched to [${nukesToUse}] - ${Nukes2D.Size[1]} spells
      }
    }
  }
/if (${Debug} || ${Debug_Assists})  /echo <== event_NukeSet -|
/RETURN

#event CombatMode "<#1#> CombatMode #2#"
sub event_CombatMode(line, ChatSender, parms)
  /if (${Debug} || ${Debug_Assists})  /echo |-- Entering Combat Mode --|
  
  |-Verify the event
  /call #CreateVars string outer argPart InclusionPart
  /call #SplitInclusionArgs "${parms}" argPart InclusionPart
  /if (${Debug} || ${Debug_Assists}) /echo argPart: ${argPart} InclusionPart: ${InclusionPart}  

  /call #verifyEvent "${ChatSender}" "${line}"
  /if (!${Macro.Return}) /return
  
  /if (${argPart.Equal[melee]}) {
    /varset AssistType Melee
    /delay 2
    /swap melee
    /delay 2
  }
  /if ( ${argPart.Equal[ranged]}) {
    /varset AssistType Ranged
    /delay 2
    /swap ranged
    /delay 2
  }
  /if (${Debug} || ${Debug_Assists})  /echo |-- Leaving Combat Mode ---|
/return

|--------------------------------------------------------------------------------|
Sub assist_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Assists (On/Off)" Debug_Assists bool outer
/if (${Debug} || ${Debug_Assists}) /echo |- assist_Setup ==>

	/declare Toggle-PvP bool outer FALSE
	/declare Assisting bool outer FALSE
	/declare AllowControl bool outer FALSE
  /declare AssistTarget int outer 0
	/declare AssistStickDistance int outer
	/declare PauseTarget string outer
|	/declare AutoAggro bool outer TRUE
	/declare use_Nukes bool outer TRUE
	/declare use_TargetAE bool outer FALSE
	/declare use_PBAE bool outer FALSE
  /declare resistCount string outer
	/declare isEnraged bool outer FALSE
	/declare revertRngToMelee bool outer FALSE
	/declare aemsg bool outer FALSE
  /declare aeTarget string outer
  /if (!${Defined[nukesToUse]})   /declare nukesToUse string global Nuke

| ------------------------------------- Import Macro_Ini Settings.
	/call IniToVar "${genSettings_Ini},Assists,Max Engage Distance"     MaxAssistDistance int outer
	/call IniToVar "${genSettings_Ini},Assists,AE Threat Range"         AE_Radius int outer
	/call INItoVar "${genSettings_Ini},Assists,Acceptable Target Types" AcceptableTargetTypes string outer
	/call INItoVar "${genSettings_Ini},Assists,Auto-Assist (On/Off)"    auto_AssistOn bool outer
  /call INItoVar "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" longTermRecastTime int outer
  /call INItoVar "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" shortTermRecastTime int outer

| ------------------------------------- Import Character_Ini Settings.
	/if (${Ini[${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)].Length})  /call IniToVar "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" AssistType string outer
  /if (${Ini[${Character_Ini},Assist Settings,Taunt(On/Off)].Length})                   /call IniToVar "${Character_Ini},Assist Settings,Taunt(On/Off)" doTaunt bool outer
  /if (${Ini[${Character_Ini},Assist Settings,SmartTaunt(On/Off)].Length})              /call IniToVar "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" smartTaunt bool outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Stick Point].Length})               /call IniToVar "${Character_Ini},Assist Settings,Melee Stick Point" AssistStickPoint string outer
	/if (${Ini[${Character_Ini},Assist Settings,Melee Distance].Length})                  /call IniToVar "${Character_Ini},Assist Settings,Melee Distance" AssistDistanceSetting string outer
	/if (${Ini[${Character_Ini},Assist Settings,Ranged Distance].Length})                 /call IniToVar "${Character_Ini},Assist Settings,Ranged Distance" RangedStickDistance string outer
	/if (${Ini[${Character_Ini},Assist Settings,Auto-Assist Engage Percent].Length})      /call IniToVar "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" EngagePct int outer
	/if (!${Defined[EngagePct]}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/declare EngagePct int outer 98
	} else /if (${Defined[EngagePct]} && !${EngagePct}) {
		/if (${Debug} || ${Debug_Assists}) /echo ERROR: @[EngagePct] - EngagePct is invalid, defaulting to 98%
		/varset EngagePct 98
	}


  | Epic Definitions for EpicBurn - warr, shd, pal,
  /if (${FindItemCount["Prismatic Dragon Blade"]})                /declare EpicName string outer "Prismatic Dragon Blade"
  /if (${FindItemCount["Blade of Vesagran"]})                     /declare EpicName string outer "Blade of Vesagran"
  |/if (${FindItemCount["Savage Lord's Totem"]})                   /declare EpicName string outer "Savage Lord's Totem"
  |/if (${FindItemCount["Spiritcaller Totem of the Feral"]})       /declare EpicName string outer "Spiritcaller Totem of the Feral"
  /if (${FindItemCount["Raging Taelosian Alloy Axe"]})            /declare EpicName string outer "Raging Taelosian Alloy Axe"
  /if (${FindItemCount["Vengeful Taelosian Blood Axe"]})          /declare EpicName string outer "Vengeful Taelosian Blood Axe"
  /if (${FindItemCount["Staff of Living Brambles"]})              /declare EpicName string outer "Staff of Living Brambles"
  /if (${FindItemCount["Staff of Everliving Brambles"]})          /declare EpicName string outer "Staff of Everliving Brambles"
  /if (${FindItemCount["Fistwraps of Celestial Discipline"]})     /declare EpicName string outer "Fistwraps of Celestial Discipline"
  /if (${FindItemCount["Transcended Fistwraps of Immortality"]})  /declare EpicName string outer "Transcended Fistwraps of Immortality"
  /if (${FindItemCount["Redemption"]})                            /declare EpicName string outer "Redemption"
  /if (${FindItemCount["Nightbane, Sword of the Valiant"]})       /declare EpicName string outer "Nightbane, Sword of the Valiant"
  /if (${FindItemCount["Heartwood Blade"]})                       /declare EpicName string outer "Heartwood Blade"
  /if (${FindItemCount["Aurora, the Heartwood Blade"]})           /declare EpicName string outer "Aurora, the Heartwood Blade"
  /if (${FindItemCount["Fatestealer"]})                           /declare EpicName string outer "Fatestealer"
  /if (${FindItemCount["Nightshade, Blade of Entropy"]})          /declare EpicName string outer "Nightshade, Blade of Entropy"
  /if (${FindItemCount["Innoruuk's Voice"]})                      /declare EpicName string outer "Innoruuk's Voice"
  /if (${FindItemCount["Innoruuk's Dark Blessing"]})              /declare EpicName string outer "Innoruuk's Dark Blessing"
  /if (${FindItemCount["Crafted Talisman of Fates"]})             /declare EpicName string outer "Crafted Talisman of Fates"
  /if (${FindItemCount["Blessed Spiritstaff of the Heyokah"]})    /declare EpicName string outer "Blessed Spiritstaff of the Heyokah"
  /if (${FindItemCount["Staff of Prismatic Power"]})              /declare EpicName string outer "Staff of Prismatic Power"
  /if (${FindItemCount["Staff of Phenomenal Power"]})             /declare EpicName string outer "Staff of Phenomenal Power"
  /if (${FindItemCount["Soulwhisper"]})                           /declare EpicName string outer "Soulwisper"
  /if (${FindItemCount["Deathwhisper"]})                          /declare EpicName string outer "Deathwisper"
  |/if (${FindItemCount["Champions Sword of Eternal Power"]}) /declare EpicName string outer "Champions Sword of Eternal Power"
  |/if (${FindItemCount["Kreljnok's Sword of Eternal Power"]}) /declare EpicName string outer "Kreljnok's Sword of Eternal Power"
  |/if (${FindItemCount["Staff of Elemental Essence"]}) /declare EpicName string outer "Staff of Elemental Essence"
  |/if (${FindItemCount["Focus of Primal Elements"]}) /declare EpicName string outer "Focus of Primal Elements"
  |/if (${FindItemCount["Oculus of Percussion"]}) /declare EpicName string outer "Oculus of Percussion"
  |/if (${FindItemCount["Staff of Eternal Eloquence"]}) /declare EpicName string outer "Staff of Eternal Eloquence"
  |/if (${FindItemCount["Harmony of the Soul"]}) /declare EpicName string outer "Harmony of the Soul"
  |/if (${FindItemCount["Aegis of Superior Divinity"]}) /declare EpicName string outer "Aegis of Superior Divinity"

  /call assist_SpellArrays

  /if (${Debug} || ${Debug_Assists}) /echo <== assist_Setup -|
/return

|------------------------------------------------------------------------------------------------
|------------------------------------------------------------------------------------------------
SUB assist_SpellArrays
| [Melee Abilities].
  /call #INItoArray "${Character_Ini},Melee Abilities,Ability#" MyAbilities

| [Nukes].
  /call #INItoArray "${Character_Ini},Nukes,${nukesToUse}#"       MyNukes
  /call #INItoArray "${Character_Ini},DoTs,DoT on Assist#"        Assist_DoTs
  /call #INItoArray "${Character_Ini},DoTs,DoT on Command#"       Command_DoTs
  /call #INItoArray "${Character_Ini},Debuffs,Debuff on Assist#"  Assist_Debuffs
  /call #INItoArray "${Character_Ini},Debuffs,Debuff on Command#" Command_Debuffs
  /call #INItoArray "${Character_Ini},TargetAE,TargetAE#"         TargetAE_Spells
  /call #INItoArray "${Character_Ini},PBAE,PBAE#"                 PBAE_Spells
  /call #IniToArray "${Character_Ini},Burn,Quick Burn#"           QuickBurns
  /call #IniToArray "${Character_Ini},Burn,Long Burn#"            LongBurns
  /call #IniToArray "${Character_Ini},Burn,Full Burn#"            FullBurns

  /if (${Me.Class.ShortName.Equal[MAG]}) {
    /if (${Bool[${Me.AltAbility[Servant of Ro]}]})        /call BuildArray "swarmPets" "Servant of Ro"
    /if (${Bool[${Me.AltAbility[Host of the Elements]}]}) /call BuildArray "swarmPets" "Host of the Elements"
  } else /if (${Me.Class.ShortName.Equal[NEC]}) {
    /if (${Bool[${Me.AltAbility[Swarm of Decay]}]}) /call BuildArray "swarmPets" "Swarm of Decay"
    /if (${Bool[${Me.AltAbility[Wake the Dead]}]})  /call BuildArray "swarmPets" "Wake the Dead"
  } else /if (${Me.Class.ShortName.Equal[SHM]}) {
    /if (${Bool[${Me.AltAbility[Spirit Call]}]})    /call BuildArray "swarmPets" "Spirit Call"
  } else /if (${Me.Class.ShortName.Equal[WIZ]}) {
    /if (${Bool[${Me.AltAbility[Call of Xuzl]}]})   /call BuildArray "swarmPets" "Call of Xuzl"
  } else /if (${Me.Class.ShortName.Equal[BRD]}) {
    /if (${Bool[${Me.AltAbility[Song of Stone]}]})   /call BuildArray "swarmPets" "Song of Stone"
  } else /if (${Me.Class.ShortName.Equal[CLR]}) {
    /if (${Bool[${Me.AltAbility[Celestial Hammer]}]})   /call BuildArray "swarmPets" "Celestial Hammer"
  }

  /if (${MyAbilities.Size} > 0)		    /call BuildSpellArray "MyAbilities"	      "Abilities2D"
  /if (${MyNukes.Size} > 0)		        /call BuildSpellArray "MyNukes"	          "Nukes2D"

  /if (${PBAE_Spells.Size} > 0)		    /call BuildSpellArray "PBAE_Spells"	      "PBAE_Spells2D"
  /if (${TargetAE_Spells.Size} > 0)		/call BuildSpellArray "TargetAE_Spells" 	"TargetAE_Spells2D"
  /if (${Assist_Debuffs.Size} > 0)		/call BuildSpellArray "Assist_Debuffs" 		"Assist_Debuffs2D"
  /if (${Command_Debuffs.Size} > 0)		/call BuildSpellArray "Command_Debuffs" 	"Command_Debuffs2D"
  /if (${Assist_DoTs.Size} > 0)	      /call BuildSpellArray "Assist_DoTs" 	    "Assist_DoTs2D"
  /if (${Command_DoTs.Size} > 0)	    /call BuildSpellArray "Command_DoTs" 	    "Command_DoTs2D"
  /if (${QuickBurns.Size} > 0)	      /call BuildSpellArray "QuickBurns" 	      "QuickBurns2D"
  /if (${LongBurns.Size} > 0)	        /call BuildSpellArray "LongBurns" 	      "LongBurns2D"
  /if (${FullBurns.Size} > 0)	        /call BuildSpellArray "FullBurns" 	      "FullBurns2D"
  /if (${swarmPets.Size} > 0)	        /call BuildSpellArray "swarmPets" 	      "swarmPets2D"

|  /declare i int local
|  /if (${Nukes2D.Size} >0) {
|    /for i 1 to ${Nukes2D.Size[1]}
|    /varset Nukes2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
|  /if (${Assist_Debuffs2D.Size} >0) {
|    /for i 1 to ${Assist_Debuffs2D.Size[1]}
|    /varset Assist_Debuffs2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
|  /if (${Command_Debuffs2D.Size} >0) {
|    /for i 1 to ${Command_Debuffs2D.Size[1]}
|    /varset Command_Debuffs2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
|  /if (${Assist_DoTs2D.Size} >0) {
|    /for i 1 to ${Assist_DoTs2D.Size[1]}
|    /varset Assist_DoTs2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
|  /if (${Command_DoTs2D.Size} >0) {
|    /for i 1 to ${Command_DoTs2D.Size[1]}
|    /varset Command_DoTs2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
|  /if (${swarmPets2D.Size} >0) {
|    /for i 1 to ${swarmPets2D.Size[1]}
|    /varset swarmPets2D[${i},${iSubToRun}] check_detrimental_target
|    /next i
|  }
/RETURN

|--------------------------------------------------------------------------------|
SUB assist_MacroSettings
/if (${Debug} || ${Debug_Assists}) /echo |- assist_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Assists (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Auto-Assist (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Assists,Max Engage Distance" 250
	/call WriteToIni "${genSettings_Ini},Assists,AE Threat Range" 100
	/call WriteToIni "${genSettings_Ini},Assists,Acceptable Target Types" NPC,Pet
  /call WriteToIni "${genSettings_Ini},Assists,Long Term Debuff Recast(s)" 30
  /call WriteToIni "${genSettings_Ini},Assists,Short Term Debuff Recast(s)" 5
/if (${Debug} || ${Debug_Assists}) /echo <== assist_MacroSettings -|
/RETURN
|--------------------------------------------------------------------------------|
SUB assist_CharacterSettings
/if (${Debug} || ${Debug_Assists}) /echo |- assist_CharacterSettings ==>

	| Add Assist_Stick entries.
	/call WriteToIni "${Character_Ini},Assist Settings,Assist Type (Melee/Ranged/Off)" ${If[${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]},Melee,Off]}
  | Default Tanks to Front
  /if (${Select[${Me.Class.ShortName},PAL,SHD,WAR]}) {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Front
    /call WriteToIni "${Character_Ini},Assist Settings,Taunt(On/Off)" Off
    /call WriteToIni "${Character_Ini},Assist Settings,SmartTaunt(On/Off)" On
  } else {
    /call WriteToIni "${Character_Ini},Assist Settings,Melee Stick Point" Behind
  }
	/call WriteToIni "${Character_Ini},Assist Settings,Melee Distance" MaxMelee
	/call WriteToIni "${Character_Ini},Assist Settings,Ranged Distance" 100
  /call WriteToIni "${Character_Ini},Assist Settings,Auto-Assist Engage Percent" 98
	| If I'm a melee class, add Melee Abilities entries.
	/if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
		/if (!${Ini[${Character_Ini},Melee Abilities].Length}) /call WriteToIni "${Character_Ini},Melee Abilities,Ability#1"
	}
	| If I'm a casting class, add Nuke entries.
	/if (${Select[${Me.Class.ShortName},BST,CLR,DRU,ENC,MAG,PAL,NEC,RNG,SHD,SHM,WIZ]}) {
		/if (!${Ini[${Character_Ini},Nukes].Length}) /call WriteToIni "${Character_Ini},Nukes,Nuke#1"
	}
  /if (${Select[${Me.Class.ShortName},DRU,RNG,MAG,WIZ]}) {
    /if (!${Ini[${Character_Ini},Nukes].Length}) /call WriteToIni "${Character_Ini},Nukes,BigFire#1"
    /if (!${Ini[${Character_Ini},Nukes].Length}) /call WriteToIni "${Character_Ini},Nukes,FastFire#1"
    /if (!${Ini[${Character_Ini},Nukes].Length}) /call WriteToIni "${Character_Ini},Nukes,LureFire#1"
  }

	| If I'm a casting class, add DoT entries.
	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,NEC,RNG,SHD,SHM]}) {
		/if (!${Ini[${Character_Ini},DoTs].Length}) {
			/call WriteToIni "${Character_Ini},DoTs,DoT on Assist#1"
			/call WriteToIni "${Character_Ini},DoTs,DoT on Command#1"
		}
	}
	| If I'm a casting class, add Debuff entries.
	/if (${Select[${Me.Class.ShortName},BST,DRU,ENC,MAG,NEC,SHM,RNG]}) {
		/if (!${Ini[${Character_Ini},Debuffs].Length}) {
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Assist#1"
			/call WriteToIni "${Character_Ini},Debuffs,Debuff on Command#1"
		}
	}
	| If I'm a casting class, add AE entries.
	/if (${Select[${Me.Class.ShortName},CLR,ENC,DRU,MAG,SHM,WIZ,RNG]}) {
		/if (!${Ini[${Character_Ini},TargetAE].Length}) /call WriteToIni "${Character_Ini},TargetAE,TargetAE#1"
    /if (!${Ini[${Character_Ini},PBAE].Length}) /call WriteToIni "${Character_Ini},PBAE,PBAE#1"
	}
	| Burn entries.
	/if (!${Ini[${Character_Ini},Burn].Length}) {
		/call WriteToIni "${Character_Ini},Burn,Quick Burn#1"
    /call WriteToIni "${Character_Ini},Burn,Long Burn#1"
		/call WriteToIni "${Character_Ini},Burn,Full Burn#1"
	}
/if (${Debug} || ${Debug_Assists}) /echo <== assist_CharacterSettings -|
/RETURN

|--------------------------------------------------------------------------------|
Sub assist_Aliases
/if (${Debug} || ${Debug_Assists}) /echo |- assist_Aliases ==>
	/noparse /squelch /alias /AssistOn /bc Assist on ${Target.ID}
	/noparse /squelch /alias /AssistMe /bc Assist on ${Target.ID}
	/squelch /alias /BackOff /bc Back off
	/noparse /squelch /alias /debuff /bc Debuffs on ${Target.ID}
	/noparse /squelch /alias /debuffson /bc Debuffs on ${Target.ID}
	/squelch /alias /debuffsoff /bc End Debuffs
	/noparse /squelch /alias /dot /bc DoTs on ${Target.ID}
	/noparse /squelch /alias /dotson /bc DoTs on ${Target.ID}
	/squelch /alias /dotsoff /bc End DoTs
	/squelch /alias /targetaeon /bc targetae on
	/squelch /alias /targetaeoff /bc targetae off
	/squelch /alias /pbaeon /bc pbae on
  /squelch /alias /pbaeoff /bc pbae off
  /squelch /alias /aeon /bc allae on
  /squelch /alias /aeoff /bc allae off
  /noparse /squelch /alias /SwarmPets /bc Swarm pets on ${Target.ID}
	/squelch /alias /EpicBurns /bc Epic burn
	/squelch /alias /QuickBurns /bc Quick burn
  /squelch /alias /LongBurns /bc Long burn
	/squelch /alias /FullBurns /bc Full burn
	/squelch /alias /taunton /varset doTaunt 1
  /squelch /alias /tauntoff /varset doTaunt 0
|	/squelch /alias /pvpOn /bc Toggle PvP ON
|	/squelch /alias /pvpOff /bc Toggle PvP OFF
	/squelch /alias /AssistType /bc //varset Assist Type
	/squelch /alias /ns /bc NukeSet
	/squelch /alias /combatmode /bc combatmode
	/squelch /alias /cm /bc combatmode

/if (${Debug} || ${Debug_Assists}) /echo <== assist_Aliases -|
/return

|--------------------------------------------------------------------------------|
Sub assist_Background_Events
  |/echo assistbg ${Time}
  /if (${auto_AssistOn}) /call check_AutoAssist
  /if (${currentTank.Equal[${Me}]}) /call tankKill
  /doevents Assist
  /doevents BackOff
  /if (${Assisting}) {
    |AssistStatus checked by casters as well, turns off assist if corpse
    /call check_AssistStatus
    |CombatAbilities called from check_AssistStatus
    /varset combatTimer ${combatDelay}
  }
  /doevents QuickBurn
  /doevents LongBurn
  /doevents FullBurn
  /doevents EpicBurn
  /doevents EnrageOn
  /doevents EnrageOff
  /doevents DebuffsOn
  /doevents DebuffsOff
  /doevents DoTsOn
  /doevents DoTsOff
  /doevents AEOn
  /doevents AEOff
  /doevents PBAEOff
  /doevents PBAEOn
  /doevents TargetAEOff
  /doevents TargetAEOn
  /doevents SwarmPets
  /doevents NukeSet
  /doevents SetTank
  /doevents DisableTank
  /doevents ToggleOfftank
  /doevents CombatMode
  |/echo assistbg ${Time}
/return


