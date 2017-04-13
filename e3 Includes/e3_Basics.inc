|------------------------------------------------------------------------|
|- e3_Macro_Basics.inc v4.0	( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
|- Contains basic and miscellaneous EVENTs and functions of e3.mac.		-|
|------------------------------------------------------------------------|

|------------------------------
|- Engages, and sets variables related to Follow.
|------------------------------
|- Only bots in the same zone will engage follow.
|- Bots will wait to engage follow until, they can see the FollowTarget, and it is within the MaxResponseDist.
|- If bots are assisting, they will back off and follow until re-engage.
|------------------------------
|- @param line: The line that called the event
|- @param ChatSender: The PC who sent the command
|- @param eventParams: Who to follow and /only,/not specifications
|------------------------------
#EVENT Follow "[#1#] Follow"
#EVENT Follow "<#1#> Follow"
#EVENT Follow "#1# tells you, 'Follow'"
#EVENT Follow "#1# tells the group, 'Follow'"
#EVENT Follow "#1# tell your party, 'Follow'"
#EVENT Follow "[#1#] Follow #2#"
#EVENT Follow "<#1#> Follow #2#"
#EVENT Follow "#1# tells you, 'Follow #2#'"
#EVENT Follow "#1# tells the group, 'Follow #2#'"
#EVENT Follow "#1# tell your party, 'Follow #2#'"
SUB EVENT_Follow(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Follow ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
	
		/declare SpawnToFollow string local ${If[${eventParams.Arg[1, ].Find[/]} || ${eventParams.Arg[1, ].Equal[me]} || ${eventParams.Arg[1, ].Equal[on]},${ChatSender},${eventParams.Arg[1, ]}]}
		/if (!${Bool[${SpawnToFollow}]}) /varset SpawnToFollow ${ChatSender}
		| -Turn Following on and set the FollowTarget
		/varset Following TRUE
		/varset FollowTarget ${Spawn[pc ${SpawnToFollow}].CleanName}
		| -If I am the follow target
		/if (${FollowTarget.Equal[${Me}]}) {
			| -Turn off follow plugins.
			/if (${Stick.Active}) /squelch /stick off
			/if (${NetAdvPath.Status}==1) /squelch /netfollow off
			| -create a list of bots following you.
			/varset Followers 
			/declare i int local
			/for i 1 to ${NetBots.Counts}
				/if (${NetBots.Client.Arg[${i}, ].NotEqual[${Me.CleanName}]} && ${SpawnCount[pc =${NetBots.Client.Arg[${i}, ]}]} && ${Spawn[pc =${NetBots.Client.Arg[${i}, ]}].Distance}<${MaxResponseDist}) /varset Followers ${Followers}${NetBots.Client.Arg[${i}, ]},
			/next i
		| <=
		} else {
		| -If I am not the follow target
			| -Check Range
			/if (${Spawn[=${FollowTarget}].Distance} > ${MaxResponseDist}) {
				/docommand ${ChatToggle} [${FollowTarget}] is too far away.
			} else {
				| -Check LoS.
				/if (!${Spawn[=${FollowTarget}].LineOfSight}) {
					/docommand ${ChatToggle} I cannot see [${FollowTarget}].
				} else {
					| -Stop assisting so we don't try to stick to the AssistTarget.	
					/if (${Assisting} || ${Me.Combat}) /call AssistOff
					| -Start Following.
					/if (${Verbosity}) /docommand ${ChatToggle} Following ${FollowTarget}.
					/call AquireFollow
				}
			}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Follow -|
/RETURN



|----------------------------------------------------------------------------------------------------|
|- Engages follow plugins on specified follow targets.												-|
|----------------------------------------------------------------------------------------------------|
|- Uses NetAdvPath if the follow target is one of your bots, or MoveUtils, if its someone elses.	-|		
|----------------------------------------------------------------------------------------------------|
SUB AquireFollow
/if (${Debug} || ${Debug_Basics}) /echo |- AquireFollow ==>
	|------------------------------------------------------------------------------------Check for FollowTarget in zone.
	/if (${SpawnCount[=${FollowTarget}]}) {
		|--------------------------------------------------------------------------------Check range to the FollowTarget
		/if (${SpawnCount[=${FollowTarget} radius ${MaxResponseDist}]}) {
			|----------------------------------------------------------------------------Check that I'm not the follow target.
			/if (${FollowTarget.NotEqual[${Me.CleanName}]}) {
				|------------------------------------------------------------------------Turn off any previous follow plugins.
				/if (${Stick.Active}) /squelch /stick off
				/if (${NetAdvPath.Status}==1) /squelch /netfollow off

				/declare followTargetID int local
				|--------------------------------------------------------------------Check Target ID
				/if (${SpawnCount[pc =${FollowTarget}]}) {
					/varset followTargetID ${Spawn[pc =${FollowTarget}].ID}
				} else /if (${SpawnCount[npc ${FollowTarget}]}) {
					/varset followTargetID ${Spawn[npc =${FollowTarget}].ID}
				} else /if (${SpawnCount[pet ${FollowTarget}]}) {
					/varset followTargetID ${Spawn[pet =${FollowTarget}].ID}
				}
				/if (${Bool[${Me.Casting.ID}]} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
				| If the FollowTarget is a NetBot, and you're using NetAdvPath
				/if (${NetBots[${FollowTarget}].ID} && ${NetAdvPath}) {
					| If the FollowTarget is a NetBot, engage MQ2NetAdvPath.
					/if (${Spawn[id ${followTargetID}].LineOfSight}) {
						/call TrueTarget ${followTargetID}
						/squelch /netfollow on
					} else /if (${NetAdvPath.Status}==2) {
						/squelch /netfollow unpause
					}
				} else {
				| If MQ2NetAdvPath was not detected, or the FollowTarget was not a NetBot, engage MQ2MoveUtils.
					|---------------------------------------------------Check LoS
					/if (${Spawn[id ${followTargetID}].LineOfSight}) {
						/call TrueTarget ${followTargetID}
						/squelch /stick hold 20 uw
					}
				}
				|-------------------------------------------------------Clear my target
				/if (${Target.ID} == ${followTargetID}) /squelch /target clear
			}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== AquireFollow -|
/RETURN

|----------------------------------------------------------------------------|
|- Disengages follow on bots who are in the same zone as the ChatSender.	  -|
|- Use, /stop, in  /group, or /bc.							                              -|
|----------------------------------------------------------------------------|
#EVENT Stop	"<#1#> Stop"
#EVENT Stop "#1# tells the group, 'Stop'"
#EVENT Stop "#1# you tell your party, 'Stop'"
#EVENT Stop	"<#1#> Stop #2#"
#EVENT Stop "#1# tells the group, 'Stop #2#'"
#EVENT Stop "#1# you tell your party, 'Stop #2#'"
SUB EVENT_Stop(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Stop ==>
	| -Check if I'm following.
	/if (${Following}) {
		| -Verify the event
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
		/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
			| -Wait to get close to the followTarget.
			/if (!${Spawn[${FollowTarget}].Moving} && ${Spawn[${FollowTarget}].Distance} > ${LeashLength}) /delay 100 ${Spawn[${FollowTarget}].Distance} < ${LeashLength} || ${Spawn[${FollowTarget}].Moving}
			| -Stop following
			/if (${Stick.Active}) /squelch /stick off
			/if (${NetAdvPath.Status}==1) /squelch /netfollow off
			/varset FollowTarget NULL
			/varset Following FALSE
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Stop -|
/RETURN STOP

#EVENT MoveHere "<#1#> Move to #2#"
#EVENT MoveHere "#1# tells you, 'Move to #2#'"
#EVENT MoveHere "#1# tells the group, 'Move to #2#'"
SUB EVENT_MoveHere(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_MoveHere ==>

	| -Verify the event
	/if (${Defined[ChatSender]}) /call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
	
		| -Check if the ChatSender is me.
		/if (${ChatSender.Equal[${Me}]}) {
			/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_MoveHere -| I am the ChatSender, skipping.
		} else {
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Status}==1) /squelch /netfollow off
      /if (${Assisting}  || ${Me.Combat}) /call AssistOff
			| -Set WhereTo.
			/declare moveTo_Name string local
			/declare WhereTo string local ${If[${eventParams.Arg[1, ].Find[/]} || ${eventParams.Arg[1, ].Equal[me]} || ${eventParams.Arg[1, ].Equal[here]},${ChatSender},${eventParams.Arg[1, ]}]}
			
			| -Change WhereTo to a spawnID
			/if (!${Int[${WhereTo}]}) {
				/if (${SpawnCount[pc =${WhereTo}]}) {
					/varset WhereTo ${Spawn[pc =${WhereTo}].ID}
				} else /if (${SpawnCount[pet =${WhereTo}]}) {
					/varset WhereTo ${Spawn[pet =${WhereTo}].ID}
				} else /if (${SpawnCount[npc =${WhereTo}]}) {
					/varset WhereTo ${Spawn[npc =${WhereTo}].ID}
				}
			}
			
			| Convert SpawnID to Loc
			/varset moveTo_Name ${Spawn[id ${WhereTo}].CleanName}
			/varset WhereTo ${Spawn[id ${WhereTo}].Loc.Replace[ ,]}
			
			| -Stop assisting so I don't try to stick to the AssistTarget.	
			/if (${Assisting} || ${Me.Combat}) /call Event_BackOff
			
			/declare retryTimer timer local 150
			
			:retry_MoveTo
			/call MoveTo -Loc|${WhereTo}
			
			| -Check if I have moved to the Target.
			/call Triangulate_Distance ${WhereTo} ${Me.Loc.Replace[ ,]}
			/if (${Macro.Return} > 10) {
				/if  (${retryTimer}) {
					/goto :retry_MoveTo
				} else {
					/echo |- EVENT_MoveHere -| Failed to move to the specified target.
				}
			} else {
				/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_MoveHere -| I have arrived at the specified target.
			}
			
			| -If I'm following, reaquire follow.
			/if (${Following}) {
				/if (${moveTo_Name.Equal[${FollowTarget}]}) {
					/call AquireFollow
				} else {
					/call EVENT_Stop "<${Me.CleanName}> Stop" ${Me.CleanName}
				}
			}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_MoveHere -|
/RETURN


|--------------------------------------------------------------------------------------------------------|
|- Commands bots to bark a message at a target.															-|
|- Targets can be designated by name, or ID, using -ID| or -Name|
|--------------------------------------------------------------------------------------------------------|
|- @param line: '<Name> Bark at MobName BarkMsg'														-|
|- @param msgTarget: The NPC you wish to bark at.														-|
|- @param barkIt: What to bark at the NPC.																-|
|- @param ChatSender: The command sender that requested bark.											-|
|- @param selectedBots: Bots specified to bark at npc.													-|
|--------------------------------------------------------------------------------------------------------|
|- Targets must be within the MaxResponseDist (default 250units).										-|
|- Use, '/bark MESSAGE', or, the command 'Bark at NAME MESSAGE' can be given in /bc, /g, or /tell.		-|
|--------------------------------------------------------------------------------------------------------|
#EVENT bark "<#1#> Bark at #2# #3#"
#EVENT bark "#1# tells the group, 'Bark at #2# #3#'"
#EVENT bark "#1# tell your party, 'Bark at #2# #3#'"
SUB EVENT_bark(line, ChatSender, msgTarget, barkMsg)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Bark ==>
  |/echo received bark
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |---------Included Bot----------|    |-------------In Zone-----------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		| -If msgTarget is not an ID, find TargetID.
		/if (!${Int[${msgTarget}]}) {
			/if (${SpawnCount[npc =${msgTarget}]}) {
				/varset msgTarget ${Spawn[npc =${msgTarget}].ID}
			} else /if (${SpawnCount[pc =${msgTarget}]}) {
				/varset msgTarget ${Spawn[npc =${msgTarget}].ID}
			} else /if (${SpawnCount[pet =${msgTarget}]}) {
				/varset msgTarget ${Spawn[npc =${msgTarget}].ID}
			} else {
				/echo Could not find a SpawnID for [${msgTarget}].
				/varset msgTarget NULL
				/return
			}
		}
		/if (${ChatSender.Equal[${Me}]}) /delay 3s
		/declare barkTarget string outer ${Spawn[id ${msgTarget}].CleanName}
		/declare barkReturned bool outer FALSE
		/declare retryTimer timer local 35s
		/declare startingLoc string local
		
		/if (${Stick.Active}) /squelch /stick off
		/if (${NetAdvPath.Status}==1) /squelch /netfollow off
		/if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt

    /call TrueTarget ${msgTarget}
    /doevents flush barkReturn

    /if (${Me.Class.ShortName.Equal[ROG]}) {
      /target clear
      /delay 2
      /attack on
      /delay 1
      /attack off
      /delay 1
      /if (${Me.Invis}) /doability hide
			/if (${Me.Sneaking}) /doability sneak
    }
		:retry_Bark
		/call TrueTarget ${msgTarget}
		|/echo barking ${barkMsg} at ${barkTarget}
    /if (${Me.Invis}) /makemevisible

		/if (${Target.Distance} > 10) {
			/call MoveTo -Loc|${Target.Y},${Target.X}
			/delay 3
			/if (${retryTimer}) {
				/goto :retry_Bark
			} else {
				/echo |- EVENT_Bark -| Failed to move to the intended target.
			}
		} else {
			| -Wait a moment, then bark the message.
			/delay ${Math.Rand[10]}
			/varset startingLoc ${Me.Loc.Replace[ ,]}
			
			| -Reset barkMsg
			/if (${barkMsg.Find[-]} && ${barkMsg.Find[|]}) {
				/varset barkMsg ${barkMsg.Arg[1,-]}
				/varset barkMsg ${barkMsg.Left[${Math.Calc[${barkMsg.Length} - 1].Int}]}
			}
			
			/say ${barkMsg}
			
			| -Wait up to 2 seconds for something to happen.
			/delay 2s ${Zone.ID} != ${currentZone} || ${Me.Loc.Replace[ ,].NotEqual[${startingLoc}]} || ${Window[ConfirmationDialogBox].Open} || ${Window[LargeDialogWindow].Open}

			| -Click Yes to a confirmation box.
			/if (${Window[ConfirmationDialogBox].Open}) {
				/notify ConfirmationDialogBox Yes_Button leftmouseup
				/delay 10
			} else /if (${Window[LargeDialogWindow].Open}) {
				/notify LargeDialogWindow LDW_YesButton leftmouseup
				/delay 10
			}

      /doevents barkReturn
			| -Check to see if the bark was successful.
			/call Triangulate_Distance ${Me.Loc.Replace[ ,]} ${startingLoc}

			|/echo dist ${Macro.Return} barkret ${barkReturned} timer ${retryTimer}

			/if (${Zone.ID} == ${currentZone} && ${Macro.Return} < 50 && !${barkReturned}) {
				/if (${retryTimer}) {
					/goto :retry_Bark
				} else {
					/docommand ${ChatToggle} Nothing seems to have happened...
				}
			}
		}
		/deletevar barkTarget
		/deletevar barkReturned
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Bark -|
/RETURN



|------------------------------------------------------------|
|- Attempts to detected NPC responses to PC hails in /say.	-|
|- @param line: '|BarkTarget| says, 'Something''			      -|
|- @param returnBark: What the NPC replied.					        -|
|------------------------------------------------------------|
#EVENT barkReturn "${barkTarget} #1#"
SUB EVENT_barkReturn(line, returnBark)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_barkReturn ==>
	/varset barkReturned TRUE
	/echo returnBark ${returnBark}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_barkReturn -|
/RETURN

|----------------------------------------------------------------------------|
|- Moves to a given location, and clicks the nearest object.				        -|
|- @param ChatSender: The command sender.									                  -|
|----------------------------------------------------------------------------|
#EVENT clickIt "<#1#> Click It#*#"
#EVENT clickIt "#1# tells the group, 'Click It'"
#EVENT clickIt "#1# tell your party, 'Click It'"
SUB EVENT_clickIt(line, ChatSender)
	| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		/declare debug_clickit bool local FALSE
    |/echo received clickit ${ChatSender} ${ChatSender.Equal[${Me}]}
		/if (${debug_clickit}) /echo ${ChatSender} ${Zone.ShortName} ${Ini[e3 Macro Inis\doors.ini,${Zone.ShortName},${Zone.ShortName}#1].Length}
		/if (${ChatSender.Equal[${Me}]}) /delay 3s
    /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
		|**
		| Move to the person who calls ClickIT
		/if (${ChatSender.NotEqual[${Me.Name}]} && ${Spawn[pc =${ChatSender}].Distance3D} > 150) {
			/echo \ay Aborting ClickIt: ${ChatSender} is > 150 away
			/return
		}
		/if (${ChatSender.NotEqual[${Me.Name}]} && ${Spawn[pc =${ChatSender}].Distance3D} > 20) {
			/declare moveToChatSenderTimer timer local 5s
			/netfollow off
			/stick off
			/squelch /target id ${Spawn[pc =${ChatSender}].ID}
			/squelch /face fast
			/moveto id ${Spawn[pc =${ChatSender}].ID}
			:continueMoveToChatSender
			/delay 1
			/if (${Spawn[pc =${ChatSender}].ID} > 20 && ${moveToChatSenderTimer}) /goto :continueMoveToChatSender

			/if (${Spawn[pc =${ChatSender}].ID} > 20) {
				/echo \ay Aborting ClickIt: Could not move to < 20 of ${ChatSender}
				/moveto off
				/return
			}
		}
		**|
		|load array of doors from doors.ini:  doorid, x, y, z, name
		/call #InitoArray "e3 Macro Inis\doors.ini,${Zone.ShortName},${Zone.ShortName}#" doorList
		/if (${debug_clickit}) /echo Doors in zone ${doorList.Size}
		/declare i int local
		/declare closestDoorID int local 0
		/declare closestDoorDist float local 0
		|find the closest door to me
		/if (${doorList.Size} > 0) {
      /for i 1 to ${doorList.Size}
        /if (${debug_clickit}) /echo ${i} ${doorList[${i}]}
        /squelch /doortarget id ${doorList[${i}].Arg[1,,]}
        |/delay 1
        /if ((${closestDoorDist}==0 || ${DoorTarget.Distance} < ${closestDoorDist}) && ${DoorTarget.Distance3D} < 100) {
          /varset closestDoorDist ${DoorTarget.Distance}
          /varset closestDoorID ${doorList[${i}].Arg[1,,]}
        }
      /next i
    }
		| if i have a door within 100 distance then move to it and try to click
		/if (${debug_clickit}) /echo closest ${closestDoorID} ${closestDoorDist}
		/if (!${Bool[${closestDoorID}]}) {
		  /squelch /doortarget
		  /if (${DoorTarget.Distance} < 150) {
        /varset closestDoorID ${Switch.ID}
				/if (${debug_clickit}) /echo no door in file, picked closest door ${Switch.ID}
		  } else {
        /echo \ay Aborting ClickIt: I could not find an acceptable door < 150 3D units
        /return
		  }

		}
		/squelch /doortarget id ${closestDoorID}
		/if (${DoorTarget.Distance} > 20) {
			/declare moveTimer timer local 3s
			/netfollow off
			/stick off
			/squelch /face door fast
			/moveto loc ${DoorTarget.Y} ${DoorTarget.X}
			:continueMoveTo
			/delay 1
			/if (${DoorTarget.Distance} > 20 && ${Bool[${moveTimer}]}) /goto :continueMoveTo
		}
		/if (${DoorTarget.Distance} > 20) {
			/echo \ay Aborting ClickIt: I could not moveto < 20 units of ${DoorTarget.Name} within 3s
			/moveto off
			/return
		} else {
			/declare MyZone int local ${Zone.ID}
			/declare clickTimer timer local 5s
			|add starting loc vs final loc for intrazone click
			/declare clickDelay int local ${Math.Calc[${Math.Rand[${Me.ID}]}%${clickitRandomDelay}|1]}
			/declare doorStartingDist ${DoorTarget.Distance3D}
			:retryClickDoor
			/delay ${clickDelay}
      /squelch /doortarget id ${closestDoorID}
			/squelch /click left door
			/if (${clickTimer} && (${MyZone}==${Zone.ID} && ${Math.Abs[${doorStartingDist}-${DoorTarget.Distance3D}]} < 20)) /goto :retryClickDoor
			/if (${MyZone}==${Zone.ID} && ${Math.Abs[${doorStartingDist}-${DoorTarget.Distance3D}]} < 20) {
				/echo \ay I have failed to click Door ${Switch.ID} ${DoorTarget.Name}
				/moveto off
				/return
			}
		}
	}
/RETURN

|----------------------------------------------------------------------------|
|- Moves to a given location, and clicks the nearest object.				-|
|----------------------------------------------------------------------------|
|- @param ChatSender: The command sender.									-|
|- @param locToStart: Where to stand when attempting to click the object.	-|
|----------------------------------------------------------------------------|
#EVENT oldclickIt "<#1#> OldClickIt #2#"
#EVENT oldclickIt "#1# tells the group, 'OldClickIt #2#'"
#EVENT oldclickIt "#1# tell your party, 'OldClickIt #2#'"
SUB EVENT_oldclickIt(line, ChatSender, locToStart)
  /if (${Debug} || ${Debug_Basics}) /echo |- EVENT_clickIt ==>
| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    | -Check LoS.
    /echo in oldclickit
    /if (!${LineOfSight[${Me.Loc.Replace[ ,]}:${locToStart}]}) {
      /docommand ${ChatToggle} I cannot see the object to click.
    } else {
      /if (${locToStart.Find[ ]}) /varset locToStart ${locToStart.Arg[1, ]}
      | -Check range to object.
      /call Triangulate_Distance ${locToStart} ${Me.Loc.Replace[ ,]}
      /if (${Macro.Return} > ${Math.Calc[${MaxResponseDist}*3].Int}) {
        /docommand ${ChatToggle} That object is too far away to click.
      } else {
        /declare retryTimer timer local 300
        /declare miscTimer timer local
        /declare startZone int local ${Zone.ID}
        /if (${Stick.Active}) /squelch /stick off
        /if (${NetAdvPath.Status}==1) /squelch /netfollow off
        /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /delay 300 !${Me.Casting.ID}
        :retryClick
        | -Check if I need to move closer.
        /call Triangulate_Distance ${locToStart} ${Me.Loc.Replace[ ,]}
        /if (${Macro.Return} > 10) {
          /if (${retryTimer}) {
            /call MoveTo -Loc|${locToStart}
            /goto :retryClick
          } else {
            /docommand ${ChatToggle} I failed to move to the inteded object
          }
        } else {
          | -Switch to first person camera.
          /keypress First_Person_Camera
          | -Face object and attempt to click it for up to 5 seconds.
          /varset miscTimer 50
          /squelch /doortarget
          /squelch /face fast door
          /squelch /look -75
          :keypress
          | -Shifts look angle up and down while attempting to click.
          /look ${If[${Me.Look} < 120,${Math.Calc[${Me.Look} + 5]},-75]}
          /if (${Target.ID}) /squelch /target clear
          /keypress U
          /delay 2
          | -Check if I have zoned, or moved.
          /call Triangulate_Distance ${locToStart} ${Me.Loc.Replace[ ,]}
          /if (${Zone.ID} == ${currentZone} && ${Macro.Return} < 50) {
            | -Check keypressTimer.
            /if (${miscTimer}) {
              /goto :keypress
            } else {
              | -Check the retryTimer.
              /if (${retryTimer}) {
                /if (${Debug} || ${Debug_Basics}) /echo |- EVENT_clickIt -| Click attempt failed, retrying.
                /keypress back hold
                /delay 2
                /keypress forward
                /delay ${Math.Rand[30]}
                /if (${retryTimer}) /goto :retryClick
                | -If the retryTimer has expired, and I have not clicked the object.
              } else {
                /docommand ${ChatToggle} I have failed to click the object.
              }
            }
          }
        }
      }
    }
  }
  /if (${Debug} || ${Debug_Basics}) /echo <== EVENT_clickIt -|
/RETURN

|----------------------------------------------------------------------------|
|- Points faces bots to your current heading, and runs through a zone line.	-|
|----------------------------------------------------------------------------|
|- @param ChatSender: The command sender.									-|
|- @param locToStart: Where to stand when trying to click the door.			-|
|- @param snapHeading: The direction to face when trying to click the door.	-|
|----------------------------------------------------------------------------|
#EVENT runThruZone "<#1#> Run through zone at #2#"
#EVENT runThruZone "#1# tells the group, 'Run through zone at #2#'"
#EVENT runThruZone "#1# tell your party, 'Run through zone at #2#'"
SUB EVENT_runThruZone(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_runThruZone ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${ChatSender.Equal[${Me}]}) /delay 2s
		/declare locToStart string local ${eventParams.Arg[1, ].Arg[1,,]},${eventParams.Arg[1, ].Arg[2,,]}
		/declare startHeading float local ${eventParams.Arg[1, ].Arg[3,,]}
		/declare retryTimer timer local 15s
		/declare miscTimer timer local
		/declare startZone int local ${Zone.ID}
	
		| -Check that you can see the location to start from.
		/if (!${LineOfSight[${Me.Loc.Replace[ ,]}:${locToStart}]}) {
			/docommand ${ChatToggle} I cannot see the starting location.
		} else {
		
			| -Check range to object.
			/call Triangulate_Distance ${locToStart} ${Me.Loc.Replace[ ,]}
			/if (${Macro.Return} > ${Math.Calc[${MaxResponseDist}*3].Int}) {
				/docommand ${ChatToggle} I am to far away from that zone line.
			} else {
			
				/if (${Stick.Active}) /squelch /stick off
				/if (${NetAdvPath.Status}==1) /squelch /netfollow off
				/if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /delay 300 !${Me.Casting.ID}
			
				:retryRTZ
			
				| -Check if I need to move closer.
				/call Triangulate_Distance ${locToStart} ${Me.Loc.Replace[ ,]}
				/if (${Macro.Return} > 10) {
					/if (${retryTimer}) {
						/call MoveTo -Loc|${locToStart}
						/goto :retryRTZ
					} else {
						/docommand ${ChatToggle} I failed to move to the inteded zone line.
					}
				} else {
				
					| -Face the zone and run foward for 6 seconds.
					/face fast heading ${Math.Calc[${startHeading}*-1]}
					/delay 5
					/keypress forward hold
					
					| -Wait to zone.
					/delay 6s !${Zone.ID} || ${Zone.ID} != ${startZone}
					/if (!${Zone.ID}) /delay 600 ${Zone.ID}
	
					| -Check if I zoned.
					/if (${Zone.ID} == ${startZone}) {
						/if (${retryTimer}) {
							/goto :retryRTZ
						} else {
							/docommand ${ChatToggle} I failed to run through the zoneline in [${Zone}].
							/call MoveTo -Loc|${locToStart}
						}
					}
				}
			}
		}
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_runThruZone -|
/RETURN



|----------------------------------------|
|- Drops invis on connected NetBots.	-|
|----------------------------------------|
#EVENT dropInvis "<#1#> Drop Invis#*#"
SUB EVENT_dropInvis(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_dropInvis ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		/if (${Me.Invis}) /makemevisible
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_dropInvis -|
/RETURN


|----------------------------------------|
#EVENT dropBuff "<#1#> Drop Buff #2#"
SUB EVENT_dropBuff(line, ChatSender, buffName)
  /if (${Debug} || ${Debug_Basics}) /echo |- EVENT_dropInvis ==>
| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /echo ${buffName}
  /if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|    |------------In Range------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
    /if (${buffName.Equal[All]}) {
      /declare i int local 1
      /for i 1 to 22
      /if (${Me.Buff[${i}].ID}) {
        /squelch /removebuff ${Me.Buff[${i}].Name}
        /delay 1
      }
      /next i
    } else {
      /removebuff ${buffName}
    }
  }
  /if (${Debug} || ${Debug_Basics}) /echo <== EVENT_dropInvis -|
/RETURN

|----------------------------------------------------|
|- Uses /exit on all bots, except the ChatSender.	  -|
|- Use, /bc Exit		                                -|
|----------------------------------------------------|
#EVENT Exit "<#1#> Exit#*#"
SUB EVENT_Exit(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Exit ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/docommand ${ChatToggle} Bye!
		/if (${Spawn[${ChatSender}].ID}!=${Me.ID}) /exit
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Exit -|
/RETURN

|------------------------------------------------------------|
|- Attempts to load the spellset listed in e3_Settings.ini.	-|
|------------------------------------------------------------|
#EVENT LoadSpells "<#1#> Load Spells"
SUB EVENT_LoadSpells(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_LoadSpells ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		| -Check if I can cast, and have a spellset.	
		/if (${Me.Class.CanCast} && ${Defined[Default_SpellSet]}) {
	
			/declare spellsetMem_Timer timer local 300
			/docommand ${ChatToggle} Loading spells...
			/memspellset ${Default_SpellSet}
			:Memloop
			/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_LoadSpells -| :Memloop
			
			/call Background_Events
			/if (${Debug} || ${Debug_Basics}) /echo spellsetMem_Timer = [${spellsetMem_Timer}]
			/if (${Window[SpellBookWnd].Open} && ${spellsetMem_Timer}) /goto :Memloop
			/if (${Me.Sitting}) /stand	
		}
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_LoadSpells -|
/RETURN


|------------------------------------------------------------|
|- Checks for specified buffs and debuffs on NetBots.		-|
|------------------------------------------------------------|
|- Use, /bc Check Buff [Name], or /bc Check buff 'Name'.	-|
|------------------------------------------------------------|
#EVENT CheckBuffs "<#1#> Check buff #2#"
SUB EVENT_CheckBuffs(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_CheckBuffs ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/declare buffName string local ${eventParams.Arg[1,-]}
    /if (${Bool[${Me.Buff[${buffName}]}]}) {
				/docommand ${ChatToggle} I have [${Me.Buff[${buffName}].Duration.TotalSeconds} seconds] left on [${Me.Buff[${buffName}]}].
    }
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_CheckBuffs -|
/RETURN

|--------------------------------------------------------|
|- Searches inventory, and banks, for indicated items.	-|
|- Use, /bc Find 'Name', or, /echo Find 'Name'.			    -|
|--------------------------------------------------------|
#EVENT FindItems "<#1#> Find Item #2#"
SUB EVENT_FindItems(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_FindItems ==>
	| -Verify the event
  /call #CreateVars string outer TempPart InclusionPart
  /call #SplitInclusionArgs "${eventParams}" TempPart InclusionPart

  /call #verifyEvent "${ChatSender}" "${line}"
  /if (!${Macro.Return}) /return

	|/call verifyEvent "${ChatSender}" "${line}"
	|/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	|/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		| -Declare counting variables.
		/declare i int local
		/declare e int local
		/declare itemName string local
    /declare ItemToFind string local ${TempPart}
		|/declare ItemToFind string local ${eventParams.Arg[1,-]}
		|/declare ItemToFind string local ${itemName}
		/if (${FindItemCount[${ItemToFind}]} || ${FindItemBankCount[${ItemToFind}]} || ${Cursor.Name.Equal[${ItemToFind}]}) {
			/docommand ${ChatToggle} >> Found (${Math.Calc[${FindItemCount[${ItemToFind}]} + ${FindItemBankCount[${ItemToFind}]}].Int}) examples of [${ItemToFind}] ..  <<
			| wait for all chars to report item count before showing details
			/delay 1s
			| -Check cursor.
			/if (${Cursor.Name.Equal[${ItemToFind}]})  /docommand ${ChatToggle} >>> Found [${Cursor.Name}] -> [Cursor]. <<<
			| -Check Inventory.
			/if (${FindItemCount[${ItemToFind}]}) {
				| -Search Equippped Items.
				/for i 1 to 22
					/varset itemName ${InvSlot[${i}].Item}
					/if (${itemName.Find[${ItemToFind}]}) /docommand ${ChatToggle} [${InvSlot[${i}].Item}] -> (${InvSlot[${i}].Name})
				/next i
				| -Search packslots.
				/for i 1 to 8	
					/varset itemName ${Me.Inventory[pack${i}]}
					/if (${itemName.Find[${ItemToFind}]}) /docommand ${ChatToggle} [${Me.Inventory[pack${i}]}] -> (InvSlot${i})
					/if (${Me.Inventory[pack${i}].Container}) {
						/for e 1 to ${Me.Inventory[pack${i}].Container}
							/varset itemName ${Me.Inventory[pack${i}].Item[${e}]}
							/if (${itemName.Find[${ItemToFind}]}) /docommand ${ChatToggle} [${Me.Inventory[pack${i}].Item[${e}]}] -> (${Me.Inventory[pack${i}]} -> InvSlot${i} -> PackSlot${e})
						/next e
					}	
				/next i
			}
      | -Search bank packslots.
			/if (${FindItemBankCount[${ItemToFind}]}) {
				/for i 1 to 26	
					/varset itemName ${Me.Bank[${i}]}
					/if (${itemName.Find[${ItemToFind}]}) /docommand ${ChatToggle} [${Me.Bank[${i}]}] -> (BankSlot${i})
					/if (${Me.Bank[${i}].Container}) {
						/for e 1 to ${Me.Bank[${i}].Container}
							/varset itemName ${Me.Bank[${i}].Item[${e}]}
							/if (${itemName.Find[${ItemToFind}]}) /docommand ${ChatToggle} [${Me.Bank[${i}].Item[${e}]}] -> (${Me.Bank[${i}]}] -> BankSlot${i} -> PackSlot${e})
						/next e
					}
				/next i
			}
		}
	|}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_FindItems -|
/RETURN

|--------------------------------------------------------|
|- Searches inventory, and banks, for indicated items.	-|
| only characters missing the item will return msg
|--------------------------------------------------------|
|- Use, /bc Find 'Name', or, /echo Find 'Name'.			-|
|--------------------------------------------------------|
#EVENT FindMissingItem "<#1#> Find Missing Item #2#"
SUB EVENT_FindMissingItem(line, ChatSender, eventParams)
	/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_FindItems ==>
  | -Verify the event
  /call #CreateVars string outer TempPart InclusionPart
  /call #SplitInclusionArgs "${eventParams}" TempPart InclusionPart

  /call #verifyEvent "${ChatSender}" "${line}"
  /if (!${Macro.Return}) /return
  /declare ItemToFind string local ${TempPart}
		|/declare ItemToFind string local ${eventParams.Arg[1,-]}

		/if (${FindItemCount[=${ItemToFind}]} || ${FindItemBankCount[=${ItemToFind}]} || ${Cursor.Name.Equal[=${ItemToFind}]}) {
      /if (${Debug} || ${Debug_Basics}) /echo found ${ItemToFind}
      /return
    } else {
			/docommand ${ChatToggle} >> I did not find ${ItemToFind} ... <<
    }
	|}
	/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_FindItems -|
/RETURN

|------------------------------------------------------------|
|- Checks for bots who are out of range, or out of zone.	-|
|------------------------------------------------------------|
|- Use, /count, or, /bc Count bots							-|
|------------------------------------------------------------|
#EVENT CountBots "[#*#] Head Count"
SUB EVENT_CountBots
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_CountBots ==>

	/if (!${NetBots.Counts}) {
		/echo You must be connected to MQ2EQBC, and, MQ2NetBots, to use this function.
	} else {
		/declare i int local
		
		/declare InRangeCount int local
		
		/declare OutOfRange string local
		/declare OutOfRangeCount int local
		
		/declare OutOfZone string local
		/declare OutOfZoneCount int local
		
		/for i 1 to ${NetBots.Counts}
		
			/if (!${SpawnCount[pc ${NetBots.Client[${i}]}]}) {
				/varset OutOfZone ${OutOfZone}${NetBots.Client[${i}]},
				/varcalc OutOfZoneCount ${OutOfZoneCount} + 1
			} else /if (${SpawnCount[pc ${NetBots.Client[${i}]}]} && !${SpawnCount[pc ${NetBots.Client[${i}]} radius ${MaxResponseDist}]}) {
				/varset OutOfRange ${OutOfRange}${NetBots.Client[${i}]},
				/varcalc OutOfRangeCount ${OutOfRangeCount} + 1
			} else {
				/varcalc InRangeCount ${InRangeCount} + 1
			}
		
		/next i
	}
	
	/if (${NetBots.Counts} < 2) {
		/echo ------------------------------------------
		/echo No other NetBots are connected.
		/echo ------------------------------------------
	} else {
		/echo ------------------------------------------
		/echo [${NetBots.Counts}] NetBots connected.
		/echo ------------------------------------------
	}
	
	/if (${InRangeCount} > 1) {
		/echo [${InRangeCount}] NetBots are present.
		/echo ------------------------------------------
	}
	
	/if (${OutOfRange.Length}) {
		/varset i 1
		/echo [${OutOfRangeCount}] NetBots out of range:
		:next_OutOfRange
		/echo ----- ${OutOfRange.Arg[${i},,]}, ${Int[${Spawn[pc ${OutOfRange.Arg[${i},,]}].Distance}]} units away.
		/varcalc i ${i} + 1
		/if (${OutOfRange.Arg[${i},,].Length}) /goto :next_OutOfRange
		/echo ------------------------------------------
	}
	
	/if (${OutOfZone.Length}) {
		/varset i 1

		/echo [${OutOfZoneCount}] NetBots out of zone:
		:next_OutOfZone
		/echo ----- ${OutOfZone.Arg[${i},,]} is in, ${Zone[${NetBots[${OutOfZone.Arg[${i},,]}].Zone}]}.
		/varcalc i ${i} + 1
		/if (${OutOfZone.Arg[${i},,].Length}) /goto :next_OutOfZone
		/echo ------------------------------------------
	}
	
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_CountBots -|
/RETURN



|----------------------------------------------------|
|- Preforms item swaps, listed in Character_Inis.	-|
|----------------------------------------------------|
#EVENT Swap_Items "<#*#> Swap #1#"
#EVENT Swap_Items "[MQ2] Swap #1#"
SUB EVENT_Swap_Items(line, IniEntry)
/if (${Debug} || ${Debug_Basics}) /echo |- Event Swap_Items ==>
	| If I don't have a matching Ini entry.
	/if (!${Ini[${Character_Ini},Swap Items,${IniEntry}].Length}) {
		/echo \ay << Swap_Items: I have no listing for [${IniEntry}]. >>
	} else {
		/if (${Cursor.ID}) /call ClearCursor
		/if (${Bool[${Me.Casting}]} && ${Me.Class.ShortName.NotEqual[BRD]}) /delay 300 !${Bool[${Me.Casting}]}
	
		/declare i int local 1
		/declare itemToSwap string local
		/declare slotToSwap string local
		
		:SwapLoop
		/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Swap_Items -| Swaploop.
		
		/varset itemToSwap ${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Arg[1,|]}
		/varset slotToSwap ${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Arg[2,|]}
		
		| If I don't have the item in my inventory.
		/if (!${FindItemCount[${itemToSwap}]}) {
			/echo |- Event Swap_Items -| #*# Error #*# [${itemToSwap}] was not found.
			/varcalc i + 1
			/if (${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Length}) /goto :SwapLoop
		} else {
			| If I'm swapping in a 2 hander, while dual wielding.
			/if (${slotToSwap.Equal[Mainhand]} && ${FindItem[${itemToSwap}].Type.Left[2].Equal[2H]} && ${Bool[${InvSlot[Offhand].Item}]}) {
			  /unequip Offhand
			  /delay 2
			  /call PlaceInFirstFreeSlot
      }
      /if (${slotToSwap.Equal[Mainhand]} && ${FindItem[${itemToSwap}].ItemSlot}==14) {
        /unequip Offhand
        /delay 2
        /call PlaceInFirstFreeSlot
      }
      /if (${slotToSwap.Equal[Offhand]} && ${FindItem[${itemToSwap}].ItemSlot}==13) {
        /unequip Mainhand
        /delay 2
        /call PlaceInFirstFreeSlot
      }
			| Swap in item.
      /if (${Debug} || ${Debug_Basics}) /echo Equipping [${itemToSwap}] ${slotToSwap}
			/call SwapItem "${itemToSwap}" ${slotToSwap}
			/delay 2
			/varcalc i ${i} + 1
			/if (${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Length}) /goto :SwapLoop
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== Swap_Items -|
/RETURN


#EVENT castItem "[MQ2] Cast Item #1#"
SUB EVENT_castItem(line, itemToUse)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_useItem ==>
	/if (${Bool[${Cursor}]}) /call ClearCursor
  /echo ${itemToUse}
	:TryAgain
	| if I cant find the item
	/if (${FindItemCount[${itemToUse}]}==0) {
		|/echo ${FindItemCount[${itemToUse}]}
		| check ini for an alias
		|/echo ${Character_Ini} ${Ini[${Character_Ini},Use Items,${itemToUse}].Length}
		/if (${Ini[${Character_Ini},Use Items,${itemToUse}].Length}) {
			/varset itemToUse ${Ini[${Character_Ini},Use Items,${itemToUse}]}
			/goto :TryAgain
		} else {
			/echo Could not find [${itemToUse}].
		}
	} else {
		| check item timers
		/if (${FindItem[${itemToUse}].Timer} || ${itemTimer_${FindItem[${itemToUse}].ID}}) {
			/echo [${itemToUse}] is not ready! [${If[${FindItem[${itemToUse}].Timer},${FindItem[${itemToUse}].Timer} ticks,${Math.Calc[${itemTimer_${FindItem[${itemToUse}].ID}}/10].Int} sec]}]
		} else {
			| cast the item
			/if (${Me.Casting.ID}) /call Interrupt
			/if (${Cursor.ID}) /call clearCursor
			/if (${Target.ID}) /declare oldTarget int local ${Target.ID}
			|/echo calling cast
			/casting "${itemToUse}|item" "-targetid|${Me.ID}"
			/if (${oldTarget}) /call TrueTarget ${oldTarget}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_useItem -|
/RETURN

|----------------------------------------------------|
|- Ends the macro before a bot finishes camping.	  -|
|----------------------------------------------------|
#EVENT EndMacro "It will take about 20 more seconds to prepare your camp."
SUB EVENT_EndMacro
	/echo User is camping, ending macro.
	/endmacro
/RETURN


|------------------------------------------------|
|- Engages MedBreak mode, on bots who can cast.	-|
|------------------------------------------------|
#EVENT medOn "<#1#> MedBreak On#*#"
SUB EVENT_medOn(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_medOn ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|		|-----------In Range-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[5,_]}]}) {
		| Check that I'm a caster, and not a bard
|		/if (${Me.Class.CanCast} && ${Me.Class.ShortName.NotEqual[BRD]}) {
		|/if (${Me.Class.ShortName.NotEqual[BRD]} && ${Me.Class.ShortName.NotEqual[WAR]}) {
    /if (${Me.Class.ShortName.NotEqual[BRD]}) {
				| Check range
			/if (${Spawn[pc ${ChatSender}].Distance} < ${MaxResponseDist}) {
				/docommand ${ChatToggle} Meditating...
        | Set medbreak variables
        /varset medBreak TRUE
        /if (${line.Find[ Hold]}) {
          /varset medBreak_Hold TRUE
        } else {
          /varset medBreak_Hold FALSE
        }
			} else {
				/docommand ${ChatToggle} I am too far away from you to meditate
			}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_medOn -|
/RETURN

|----------------------------------------------------|
|- disengages MedBreak mode, on bots who can cast.	-|
|----------------------------------------------------|
#EVENT medOff "<#1#> MedBreak Off#*#"
SUB EVENT_medOff(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_medOff ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		| If medbreak is engaged
		/if (${medBreak}) {
			| Reset variables, and stand up
			/docommand ${ChatToggle} Ending Medbreak.
			/varset medBreak FALSE
			/varset medBreak_Hold FALSE
			/if (${Me.Sitting}) /stand
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_medOff -|
/RETURN



|--------------------------------------------------------------------------------|
|- MedBreak hook for Tertiary loop.												-|
|--------------------------------------------------------------------------------|
|- Automatically disengages and re-engages MedBreak when conditions are met.	-|
|- Ends MedBreak mode at 99% mana, unless, 'MedBreak hold command is given.		-|
|--------------------------------------------------------------------------------|
SUB check_MedBreak
/if (${Debug} || ${Debug_Basics}) /echo |- check_MedBreak ==>

	| If I'm following, check distance to follow target
	/if (${Following} && ${SpawnCount[pc =${FollowTarget}]} && ${Spawn[pc =${FollowTarget}].Distance} < ${LeashLength}  && !${Me.Moving}) {
		/call check_Follow
		/delay 10 ${Me.Moving}
	}
	| If I'm not moving disengage any active follow plugins
	/if (!${Me.Moving} && (${Stick.Active} || ${NetAdvPath.Status}==1)) {
		/if (${Stick.Active}) /squelch /stick off
		/if (${NetAdvPath.Status}==1) /squelch /netfollow off
	}
	| If End MedBreak in Combat (On/Off)=On, and a netbot is in combat, call medOff
	/if (${medOn_combatBreak} && ${Me.CombatState.Equal[COMBAT]}) {
    /docommand ${ChatToggle} Ending Medbreak.
    /varset medBreak FALSE
    /varset medBreak_Hold FALSE
    /if (${Me.Sitting}) /stand
	}

	/if (!${Me.Feigning}) {
		| do ***Meditating*** popup
		/if (!${medbreak_Popup_Timer}) {
			/popup ***Meditating***
			/varset medbreak_Popup_Timer 30
		}
		| sit down, if we are not sitting
		/if (!${Me.Sitting} && !${Me.Casting.ID} && !${Me.Moving}) /sit
		| end MedBreak when we are full mana, unless hold is engaged
		/if (${medBreak} && !${medBreak_Hold} && (${Me.PctMana} >= 99 || ${Me.PctMana}==0) && ${Me.PctEndurance}>=99) {
			/docommand ${ChatToggle} Full Mana/Endurance, ending MedBreak.
			/if (${Me.Sitting}) /stand
			/varset medBreak FALSE
			/varset medBreak_Hold FALSE				
		}	
	}
/if (${Debug} || ${Debug_Basics}) /echo <== check_MedBreak -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT set_AAXP "<#1#> Set AA XP #2#"
#EVENT set_AAXP "[#1#] Set AA XP #2#"
SUB EVENT_set_AAXP(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- set_AAXP ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {	

		| reset eventParams 
		/varset eventParams ${Int[${eventParams}]}
	
		/echo Setting AA XP [${eventParams}%].
		/alternateadv ON ${eventParams}
	}
	
/if (${Debug} || ${Debug_Basics}) /echo <== set_AAXP -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT fixCorpses "<#*#> Fix Corpses"
SUB EVENT_fixCorpses
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_fixCorpses ==>

	/echo Attempting to reset corpse positions...
	/hidecorpse all
	/delay 2
	/hidecorpse none
	/delay 2
	/hidecorpse looted

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_fixCorpses -|
/RETURN

|----------------------------------------|
|- Consents a PC to drag your corpse.	-|
|----------------------------------------|
#event Consent "#1# tells you, 'Consent #2#'"
#event Consent "<#1#> Consent #2#"
SUB Event_Consent(line, ChatSender, ConsentTarget)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Consent ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		
		/if (${ConsentTarget.NotEqual[${Me.CleanName}]}) {
			/if (${Verbosity}) /docommand ${ChatToggle} Consenting ${ConsentTarget}
			/consent ${ConsentTarget}
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Consent -|
/RETURN

|--------------------------------------------------------------------------------------------|
|- Adds a PC name to your list of consentTargets.  This is used is corpse drag functions.	-|
|--------------------------------------------------------------------------------------------|
#event addConsent "You have been given permission to drag #1#'s corpse in all zones."
SUB event_addConsent(line, consentTarget)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_addConsent ==>

	/if (!${Select[${consentTarget},${consentTargets}]}) {
		/varset consentTargets ${consentTargets},${consentTarget}
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_addConsent -|
/RETURN

|--------------------------------------------------------------------------------|
|- Clears your list of PCs who have consented to have you move their corpse.	-|
|--------------------------------------------------------------------------------|
#event flushConsent "<#*#> Flush consent"
#event flushConsent "[MQ2] Flush consent"
SUB event_flushConsent
	/echo Flushing my current consented targets.
	/if (${Defined[consentTargets]}) /varset consentTargets ${Me.CleanName}
/RETURN

|--------------------------------------------------------------------------------|
#event gatherCorpses "[MQ2] Gather corpses"
#event gatherCorpses "[MQ2] Gather corpse"
SUB event_gatherCorpses
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_gatherCorpses ==>
  /bc Consent ${Me.CleanName}
	/declare i int local
	/for i 1 to ${SpawnCount['s radius 100]}
		/if (${Select[${Spawn[${NearestSpawn[${i},pccorpse radius 100]}].CleanName.Arg[1,']},${consentTargets}]}) {
			/call TrueTarget ${Spawn[${NearestSpawn[${i},pccorpse radius 100]}].ID}
			/delay 2
			/corpse
			/delay 10 ${Target.Distance} < 20
		}
	/next i
	/squelch /target clear
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_gatherCorpses -|
/RETURN

|-------------------------------
|- All bots click yes or no to confirmation boxes.
|-------------------------------
#EVENT ClickYes	"<#1#> Click #2#"
SUB EVENT_ClickYes(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_ClickYes ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
	
		/declare buttonToClick string local ${If[${eventParams.Find[yes]},Yes,No]}
		/declare miscTimer timer local 100
		
		:clickButton
		/if (${Window[ConfirmationDialogBox].Open}) {
			/notify ConfirmationDialogBox ${buttonToClick}_Button leftmouseup
		} else /if (${Window[LargeDialogWindow].Open}) {
			/notify LargeDialogWindow LDW_${buttonToClick}Button leftmouseup
		}
		/if (${miscTimer}) {
			/if (${Window[ConfirmationDialogBox].Open} || ${Window[LargeDialogWindow].Open}) /goto :clickButton
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_ClickYes -|	
/RETURN



|--------------------------------------------------------|
|- Attempt to gate on all bots.							-|
|--------------------------------------------------------|
|- Use, /bc gate, /tell 'Name' [Gate], or, /g [Gate].	-|
|--------------------------------------------------------|
#EVENT gate	"<#1#> Gate"
#EVENT gate "#1# tells you, 'Gate'"
#EVENT gate "#1# tell your party, 'Gate'"
#EVENT gate "#1# tells the group, 'Gate'"
SUB EVENT_gate(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Gate ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {

		| -Attempt to gate.
		/if (!${Me.Book[gate]}) {
			/if (${FindItem[Philter of Major Translocation].ID}) {
				/docommand ${ChatToggle} Preparing to gate...
				/call e3_Cast ${Me.ID} "Philter of Major Translocation"
			} else /if (${FindItem[Vial of Swirling Smoke].ID}) {
				/docommand ${ChatToggle} Preparing to gate...
				/call e3_Cast ${Me.ID} "Vial of Swirling Smoke"
			} else {
				/docommand ${ChatToggle} I am not able to gate at this time.
			}
		} else {
			/docommand ${ChatToggle} Preparing to gate...
			/call e3_Cast ${Me.ID} "Gate/GiveUpTimer|30s"
		}
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Gate -|
/RETURN



|--------------------------------------------------------|
|- Request evac from bots set to Evac_OnCommand.		-|
|--------------------------------------------------------|
|- Use, /bc Evac, /tell 'Name' [Evac], or, /g [Evac].	-|
|--------------------------------------------------------|
#EVENT Evacuate "[#1#] Evac"
#EVENT Evacuate	"<#1#> Evac"
#EVENT Evacuate "#1# tells you, 'Evac'"
#EVENT Evacuate "#1# tell your party, 'Evac'"
#EVENT Evacuate "#1# tells the group, 'Evac'"
#EVENT Evacuate "[#1#] Evacuate"
#EVENT Evacuate	"<#1#> Evacuate"
#EVENT Evacuate "#1# tells you, 'Evacuate'"
#EVENT Evacuate "#1# tell your party, 'Evacuate'"
#EVENT Evacuate "#1# tells the group, 'Evacuate'"

#EVENT Evacuate	"<#1#> Evac |/|#*#"
#EVENT Evacuate "#1# tells you, 'Evac |/|#*#'"
#EVENT Evacuate "#1# tell your party, 'Evac |/|#*#'"
#EVENT Evacuate "#1# tells the group, 'Evac |/|#*#'"
#EVENT Evacuate	"<#1#> Evacuate |/|#*#"
#EVENT Evacuate "#1# tells you, 'Evacuate |/|#*#'"
#EVENT Evacuate "#1# tell your party, 'Evacuate |/|#*#'"
#EVENT Evacuate "#1# tells the group, 'Evacuate |/|#*#'"
SUB EVENT_Evacuate(line, ChatSender)
/if (${Select[${Me.Class.ShortName},DRU,WIZ]}) {
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Evacuate ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {

		| -Attempt to evac.
		/if (${Me.AltAbilityReady[Exodus]}) {
			/docommand ${ChatToggle} Evacuating!
			/casting "Exodus" alt
		} else /if (${Me.Book[${Evac_Spell.Arg[1,/]}]}) {
			/docommand ${ChatToggle} Evacuating!
			/casting "${Evac_Spell.Arg[1,/]}"
		} else {
			/docommand ${ChatToggle} I am not able to Evac at this time.
		}
	}

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Evacuate -|
}
/RETURN



|----------------------------------------|
|- Automatically accepts raid invites.	-|
|----------------------------------------|
#EVENT RaidAccept "#*#invites you to join a raid.#*#"
SUB EVENT_RaidAccept
  /delay 1s
	/raidaccept
/RETURN


|--------------------------------------------|
|- Forces a raid request in guild.			-|
|--------------------------------------------|
|- Use, /tell 'Name' Request raid invite	-|
|--------------------------------------------|
#EVENT Request_RaidInvite "#1# tells you, 'Request raid invite'"
SUB EVENT_Request_RaidInvite(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- Request_RaidInvite ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]}) {
		/gu Raid invite
	}

/if (${Debug} || ${Debug_Basics}) /echo <== Request_RaidInvite -|
/RETURN



|----------------------------------------------------------------------------------------|
|- Forces all bots to disband from their raid.											-|
|----------------------------------------------------------------------------------------|
|- Use, /bc RaidDisband, /bc disband from raid, or /tell 'Name' [Force raid disband].	-|
|----------------------------------------------------------------------------------------|
#EVENT Force_RaidDisband "<#*#> RaidDisband"
#EVENT Force_RaidDisband "<#*#> Disband from raid"
#EVENT Force_RaidDisband "#*# tells you, '[Force Raid Disband]'"
SUB EVENT_Force_RaidDisband
/if (${Debug} || ${Debug_Basics}) /echo |- Force_RaidDisband ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/bcaa //raiddisband
	}

/if (${Debug} || ${Debug_Basics}) /echo <== Force_RaidDisband -|
/RETURN

|----------------------------------------|
|- Automatically accepts group invites.	-|
|----------------------------------------|
#EVENT Invite "#1# invites you to join a group."
SUB EVENT_Invite(line, ChatSender)
/if (${Debug}) /echo |- EVENT_Invite ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]}) {
		| -Wait to finish casting
		/if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /stopcast
		/delay 10 !${Me.Casting.ID}
		/squelch /target clear

		/declare retryTimer timer local 5s
		
		:retry_Invite
|		/if (${Debug}) /echo |- EVENT_Invite -| retry_Invite loop.
		
		/delay 5
		/invite
		/delay 1s ${Group}
		
		/if (!${Group}) {
			/if (${retryTimer}) {
				/delay 1s
				/goto :retry_Invite
			} else {
				/echo Failed to join the group.
			}
		}
	}
/if (${Debug}) /echo <== EVENT_Invite -|
/RETURN



|----------------------------------------|
|- Disband bots from group and raid.	-|
|----------------------------------------|
|- Use, /bc disband						-|
|----------------------------------------|
#EVENT Disband "#1# tells you, 'Incoming group invite...'"
#EVENT Disband "<#1#> Disband all"
SUB EVENT_Disband(line, ChatSender)
/if (${Debug}) /echo |- EVENT_Disband ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]}) {
		| -Wait to stop casting.
    /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /stopcast
    /delay 2 !${Me.Casting.ID}
		| -Disband from a raid.
		/if (${Raid.Members}) {
			/call TrueTarget ${Me.ID}
			/delay 2
			/if (${Raid.Leader.Name.Equal[${Me}]}) /delay 3
			/raiddisband
			/delay 1s !${Raid.Members}
		}
		| -Disband from a group.
		/if (${Group} && !${Group.Leader.Name.Equal[${Me}]}) {
			/call TrueTarget ${Me.ID}
			/delay 2
			/disband
			/delay 1s !${Group}
		}
	}
/if (${Debug}) /echo <== EVENT_Disband -|
/RETURN

|--------------------------------------------------------------------------------------------|
|- Groupleaders save current group formations to the Groups.ini file, to be recalled later.	-|
|--------------------------------------------------------------------------------------------|
|- Use, /savegroup 'Name', /bc Save Group 'Name', or /bc SetGroup 'Name'					-|
|--------------------------------------------------------------------------------------------|
#EVENT saveGroup "<#1#> Save Group #2#"
SUB EVENT_saveGroup(line, ChatSender, eventParams)
/if (${Debug}) /echo |- EVENT_saveGroup ==>

	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		| -Check that I'm the group leader.
		/if (!${Select[${Group.Leader},${Me}]}) {
			/if (${Debug}) /echo I am not a group leader, skipping.
		} else {
			
			/echo Preparing to save...
			/declare groupName string local ${eventParams.Arg[1,/]}
			/if (${groupName.Right[1].Equal[ ]}) /varset groupName ${groupName.Left[${Math.Calc[${groupName.Length}-1]}]}
			/delay ${Math.Calc[${NetBots[${Me.CleanName}]} * 3].Int}
			/declare uniqueID int local
			/declare i int local
			/declare e int local
			
			| Set a random ID to associate with out group name.
			/for i 1 to ${NetBots.Counts}
				/if (${NetBots.Client[${i}].Equal[${Me.CleanName}]}) {
					/varset uniqueID ${i}
				}
			/next i
			
			| Check to see if existing entries need to be overwritten.
			/if (${Ini[${Group_Ini}].Find[${groupName}]}) {
				/echo Overwriting existing saved group(s)...
				/varset i 1

				:next_groupName
				/if (${Ini[${Group_Ini}].Arg[${i},|].Length}) {
					/if (${Ini[${Group_Ini}].Arg[${i},|].Arg[2,_].Equal[${groupName}]}) {
						/for e 0 to 5
							/call WriteToIni "${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]},GroupMember#${e}" "DEPRECATED" 1
						/next e
					}
					/varcalc i ${i}+1
					/goto :next_groupName
				}
			}
			
			| Delay for a moment to allow other bots to catch up, so they don't overwrite our soon to be saved group.
			/delay 20
			| Save the group configuration
			/echo Group configuration(s) saved. Use [/group ${groupName}] to recall this group setup.
			/for i 0 to 5
				| -Save group members 0-5.
				/if (${Bool[${Group.Member[${i}]}]}) {
					/call WriteToIni "${Group_Ini},${MacroQuest.Server}_${groupName}_${uniqueID},GroupMember#${i}" ${Group.Member[${i}]} 1
				| -If the group has less that 5 members, set placeholders
				} else {
					/call WriteToIni "${Group_Ini},${MacroQuest.Server}_${groupName}_${uniqueID},GroupMember#${i}" PLACEHOLDER 1
				}
			/next i
		}
	}

/if (${Debug}) /echo <== EVENT_saveGroup -|
/RETURN



|---------------------------------------------------------|
|- Lists all saved group setups, on your current server. -|
|---------------------------------------------------------|
#EVENT listGroups "<#1#> List Groups"
#EVENT listGroups "<#1#> ListGroups"
SUB EVENT_listGroups
/if (${Debug}) /echo |- listGroups ==>

	| -Check Group_Ini for groups on this server.
	/if (!${Ini[${Group_Ini}].Find[${MacroQuest.Server}]}) {
		/echo You don't have any group setups saved on, [${MacroQuest.Server}].
	} else {
	
		/declare i int local 1
		/declare e int local
		
		:next_groupListing

		/if (${Ini[${Group_Ini}].Arg[${i},|].Arg[1,_].Find[${MacroQuest.Server}]}) {
			/echo
			/echo --- Group: [${Ini[${Group_Ini}].Arg[${i},|].Arg[2,_]}] ---
			
			| -Reset 'e' the group member counting variable.
			/varset e 1
			
			:next_memberListing
			
			| -Check that the entry is not a PLACEHOLDER.
			/if (${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]},${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]}].Arg[${e},|]}].NotEqual[PLACEHOLDER]}) {
				/echo Group member#${e}: ${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]},${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]}].Arg[${e},|]}]}
				/varcalc e ${e} + 1
				/if (${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]},${Ini[${Group_Ini},${Ini[${Group_Ini}].Arg[${i},|]}].Arg[${e},|]}].Length}) /goto :next_memberListing
			}
		}	

		/varcalc i ${i} + 1
		/if (${Ini[${Group_Ini}].Arg[${i},|].Length}) /goto :next_groupListing
		/echo
	}

/if (${Debug}) /echo <== listGroups -|
/RETURN



|------------------------------------------------|
|- Recalls groups saved in your Groups.ini.		-|
|------------------------------------------------|
|- Use, /group 'Name', or, /bc Group 'Name'.	-|
|------------------------------------------------|
#EVENT groupUp "<#1#> Group #2#"
SUB EVENT_groupUp(line, ChatSender, eventParams)
/if (${Debug}) /echo |- EVENT_groupUp ==>

| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {

    /if (${ChatSender.Equal[${Me.Name}]}) {
      /squelch /bca //raiddisband
      /squelch /bca //disband
      /delay 5
      /if (${Raid.Members}) /raiddisband
    }
    /delay 5

    /declare groupName string local ${eventParams.Arg[1,/]}
    /if (${groupName.Right[1].Equal[ ]}) /varset groupName ${groupName.Left[${Math.Calc[${groupName.Length}-1]}]}
    /declare groupFound string local
    /declare groupMembers[5] string local
    /declare groupSize int local

    /declare i int local 1
    /declare e int local

    | -Check for groupName in Group_Ini.
    :nextGroupListing
    /if (${${IniMode}[${Group_Ini}].Arg[${i},|].Arg[2,_].Equal[${groupName}]} && ${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#0].NotEqual[DEPRECATED]}) {
      | Disband from a current group
      /if (${Group}) /call EVENT_Disband "PLACEHOLDER" "${Me.CleanName}"

      | check to see if I am the group leader(GroupMember#0)
      /if (${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#0].Equal[${Me.CleanName}]}) {
        /echo Recalling [${groupName}]...
        /delay 15
        | Invite group members while ignoring placeholders, and set
        /for e 1 to 5
        /varset groupMembers[${e}] ${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#${e}]}
        /if (${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#${e}].NotEqual[PLACEHOLDER]}) {
          /varcalc groupSize ${groupSize}+1
          | -Alert PC of incoming group invite.
          /invite ${groupMembers[${e}]}
        }
        /next e

        /delay 6s ${Group} == ${groupSize}
        /if (${Group} != ${groupSize}) {
          /for e 1 to 5
          /if (${groupMembers[${e}].NotEqual[PLACEHOLDER]} && !${Bool[${Group.Member[${groupMembers[${e}]}]}]}) /docommand ${ChatToggle} [${groupMembers[${e}]}] Did not join the group...
          /next e
        }
        /goto :skip
      }
    }
    /varcalc i ${i}+1
    /if (${${IniMode}[${Group_Ini}].Arg[${i},|].Length}) /goto :nextGroupListing
  }
  :skip
:skip
/if (${Debug}) /echo <== EVENT_groupUp -|
/RETURN

|** #EVENT groupUp "<#1#> Group #2#"
SUB EVENT_groupUp(line, ChatSender, eventParams)
  /debug main |- EVENT_groupUp ==>

| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {

    /if (${ChatSender.Equal[${Me.Name}]}) {
      /if (${Raid.Leader}) {
        /bca //raiddisband
        /delay 2s
        /raiddisband
      } else {
        /delay 3s
      }
      /bca //disband
      /delay 2s
    }

    /declare groupName string local ${eventParams.Arg[1,/]}
    /if (${groupName.Right[1].Equal[ ]}) /varset groupName ${groupName.Left[${Math.Calc[${groupName.Length}-1]}]}
    /declare groupFound string local
    /declare groupMembers[5] string local
    /declare groupSize int local

    /declare i int local 1
    /declare e int local

    | -Check for groupName in Group_Ini.
    :nextGroupListing
    /if (${${IniMode}[${Group_Ini}].Arg[${i},|].Arg[2,_].Equal[${groupName}]} && ${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#0].NotEqual[DEPRECATED]}) {
      | Disband from a current group
      /if (${Group}) /call EVENT_Disband "PLACEHOLDER" "${Me.CleanName}"

      | check to see if I am the group leader(GroupMember#0)
      /if (${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#0].Equal[${Me.CleanName}]}) {
        /echo Recalling [${groupName}]...
        /delay 20
        | Invite group members while ignoring placeholders, and set
        /for e 1 to 5
        /varset groupMembers[${e}] ${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#${e}]}
        /if (${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#${e}].NotEqual[PLACEHOLDER]}) {
          /varcalc groupSize ${groupSize}+1
          | -Alert PC of incoming group invite.
          /invite ${groupMembers[${e}]}
        }
        /next e

        /delay 4s ${Group} == ${groupSize}
        /if (${Group} != ${groupSize}) {
          /for e 1 to 5
            /if (${groupMembers[${e}].NotEqual[PLACEHOLDER]} && !${Group.Member[${groupMembers[${e}]}]}) /docommand ${ChatToggle} [${groupMembers[${e}]}] Did not join the group...
          /next e
        }
        /goto :skip
      }
    }
    /varcalc i ${i}+1
    /if (${${IniMode}[${Group_Ini}].Arg[${i},|].Length}) /goto :nextGroupListing
  }
  :skip
  /debug main <== EVENT_groupUp -|
/RETURN
**|
|---------------------------|
#EVENT exp_group "[#1#] ExpGroup #2#"
#EVENT exp_group "<#1#> ExpGroup #2#"
#EVENT exp_group "#1# tells the group, 'ExpGroup #2#'"
#EVENT exp_group "#1# tell your party, 'ExpGroup #2#'"
SUB EVENT_exp_group(line,ChatSender, expToggle)
	|/if (!${Ini[${Character_Ini},Swap Items].Length}) /call WriteToIni "${Character_Ini},Swap Items,;Melee" "Ruby of Determined Assault|Ranged"
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
  |-    |-----------Valid User----------|    |-----------In Zone-------------|	|--------Included Bot-----------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {

    |enable KoS mode
    /if (${Select[${expToggle},On,Yes,True,1]} > 0) {
      /call #INItoArray "${Character_Ini},Melee Abilities,Ability#" MyAbilities
			/if (${MyAbilities.Size} > 0)	/call BuildSpellArray "MyAbilities" "Abilities2D"
      /if (${Me.Class.ShortName.Equal[Rog]}) {
      	/call buff_SpellArrays
				/varset strikeDisc ${strikeDiscStr.Arg[1,/]}
			}
      /if (${Ini[${Character_Ini},Nukes,${KSNukeSet}].Length}) /call event_NukeSet "PLACEHOLDER" "${Me}" ${KSNukeSet}
      /if (${Ini[${Character_Ini},Swap Items,${KSWeaponSet}].Length}) /swap ${KSWeaponSet}
      /echo \ag KS Mode Activated - get exp

		|remove anything that could kos when out of group
    } else /if (${Select[${expToggle},Off,No,False,0]} > 0) {
      /if (${Select[${Me.Class.ShortName},${Ini[${genSettings_Ini},ExpGroup,NoCombatAbil]}]}) {
        /if (${Defined[MyAbilities]}) /deletevar MyAbilities
        /if (${Defined[Abilities2D]}) /deletevar Abilities2D
      }
      /if (${Ini[${Character_Ini},Nukes,${NoKSNukeSet}].Length}) /call event_NukeSet "PLACEHOLDER" "${Me}" ${NoKSNukeSet}
			/if (${Defined[NoKSWeaponSet]} && ${Ini[${Character_Ini},Swap Items,${NoKSWeaponSet}].Length}) /swap ${NoKSWeaponSet}
      /if (${Me.Class.ShortName.Equal[Rog]}) {
				/varset strikeDisc 0
        /declare i int local
        :remove_poisons
          /for i 1 to ${CombatBuffs2D.Size[1]}
            /if (${CombatBuffs2D[${i},${iCastName}].Find[${expNoPoison}]}) {
              /call RemoveSpellArrayElement "CombatBuffs2D" ${i}
              /goto :remove_poisons
            }
          /next i
          /for i 1 to ${SelfBuffs2D.Size[1]}
          /if (${SelfBuffs2D[${i},${iCastName}].Find[${expNoPoison}]}) {
            /call RemoveSpellArrayElement "SelfBuffs2D" ${i}
            /goto :remove_poisons
          }
          /next i
          /for i 1 to ${BotBuffs2D.Size[1]}
          /if (${BotBuffs2D[${i},${iCastName}].Find[${expNoPoison}]}) {
            /call RemoveSpellArrayElement "BotBuffs2D" ${i}
            /goto :remove_poisons
          }
          /next i
        /squelch ${Me.Buff[${expNoPoison}].Remove}
      }
      /echo \ag NoKS Mode Activated - do not get exp
    }
}
/return

|---------------------------|
#EVENT reportMana "<#1#> Report mana#*#"
SUB EVENT_reportMana(line,ChatSender)
/if (${Me.Class.CanCast}) {
	/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_reportMana ==>
		/call verifyEvent "${ChatSender}" "${line}"
		/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
		|-    |-----------Valid User----------|    |-----------In Zone-------------|	|--------Included Bot-----------|
		/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {   	
			/if (${Me.PctMana}<100) /docommand ${ChatToggle} ${Me.PctMana}m
			|${If[${Me.PctMana}!=100, - Full Mana in ${Math.Calc[((${Me.MaxMana}-${Me.CurrentMana})/${Me.ManaRegen})*6]}s,]}
		}
	/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_reportMana [${Me.PctMana}m] -|
}
/RETURN

|---------------------------|
#EVENT campDesktop "<#1#> CampDesktop"
SUB campDesktop(line,ChatSender)
	/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_reportMana ==>
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|-    |-----------Valid User----------|    |-----------In Zone-------------|	|--------Included Bot-----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]}) {
		/camp desktop
		/end
	}
	/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_reportMana [${Me.PctMana}m] -|
/RETURN

Sub basics_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Basics (On/Off)" Debug_Basics bool outer
/if (${Debug} || ${Debug_Basics}) /echo |- basics_Setup ==>

	/if (!${Defined[Following]}) /declare Following bool global FALSE
	/if (!${Defined[FollowTarget]}) /declare FollowTarget string global NULL
	/if (!${Defined[Followers]}) /declare Followers string global NULL

	/if (!${Defined[consentTargets]}) /declare consentTargets string global ${Me.CleanName}

	/declare MeleeClasses string outer BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR
	/declare PriestClasses string outer CLR,DRU,SHM
	/declare CasterClasses string outer ENC,MAG,NEC,WIZ

	/declare event_counter int outer 0

	/declare medBreak bool outer FALSE
	/declare medBreak_Hold bool outer FALSE
	/declare medbreak_Popup_Timer timer outer
	/declare clickitRandomDelay int outer 7

	/if (${Ini[${genSettings_Ini},ExpGroup,NoKS_WeaponSet].Length}) 		/call IniToVar "${genSettings_Ini},ExpGroup,NoKS_WeaponSet" NoKSWeaponSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,KS_WeaponSet].Length}) 			/call IniToVar "${genSettings_Ini},ExpGroup,KS_WeaponSet" KSWeaponSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,NoKS_NukeSet].Length}) 			/call IniToVar "${genSettings_Ini},ExpGroup,NoKS_NukeSet" NoKSNukeSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,KS_NukeSet].Length}) 				/call IniToVar "${genSettings_Ini},ExpGroup,KS_NukeSet" KSNukeSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,NoPoison].Length}) 					/call IniToVar "${genSettings_Ini},ExpGroup,NoPoison" expNoPoison string outer

	/declare MaxResponseDist int outer 250
	/if (${Ini[${genSettings_Ini},General,Max Response Distance].Length} && ${Int[${Ini[${genSettings_Ini},General,Max Response Distance]}]}) /call IniToVar "${genSettings_Ini},General,Max Response Distance" MaxResponseDist int outer

	/declare LeashLength int outer 100
	/if (${Ini[${genSettings_Ini},General,Leash Length].Length} && ${Int[${Ini[${genSettings_Ini},General,Leash Length]}]}) /call IniToVar "${genSettings_Ini},General,Leash Length" LeashLength int outer
	/if (${Ini[${genSettings_Ini},General,End MedBreak in Combat(On/Off)].Length}) /call IniToVar "${genSettings_Ini},General,End MedBreak in Combat(On/Off)" medOn_combatBreak bool outer
  /if (${Ini[${genSettings_Ini},General,AutoMedBreak PctMana].Length}) /call IniToVar "${genSettings_Ini},General,AutoMedBreak PctMana" autoMedPctMana int outer
	| -Add Groups_Ini file path
	/if (!${Ini[${MacroData_Ini},File Paths,Saved Groups].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Saved Groups" "e3 Macro Inis\Saved Groups.ini" 1
	| -Import Groups_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Saved Groups].Length}) {
		/echo ERROR: Could not find designated file path for [Saved Groups], please review [${MacroData_Ini}] and restart.
		/endmacro
	} else {
		/declare Group_Ini string outer ${Ini[${MacroData_Ini},File Paths,Saved Groups]}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== basics_Setup -|
/return



Sub basics_Background_Events
	/doevents Follow
	/doevents Stop
  /doevents MoveHere
|	/if (${event_counter}==3) {
  /doevents clickIt
  /doevents oldclickIt
  /doevents bark
  /doevents reportMana
  /doevents groupUp
  /doevents listGroups
  /doevents saveGroup
  /doevents Disband
  /doevents Invite
  /doevents Force_RaidDisband
  /doevents Request_RaidInvite
  /doevents RaidAccept
  /doevents Evacuate
  /doevents gate
  /doevents ClickYes
  /doevents gatherCorpses
  /doevents flushConsent
  /doevents addConsent
  /doevents Consent
  /doevents fixCorpses
  /doevents set_AAXP
  /doevents medOff
  /doevents medOn
  /doevents EndMacro
  /doevents castItem
  /doevents Swap_Items
  /doevents CountBots
  /doevents FindMissingItem
  /doevents FindItems
  /doevents CheckBuffs
  /doevents LoadSpells
  /doevents Exit
  /doevents dropInvis
  /doevents dropBuff
  /doevents runThruZone
  /doevents macroHelp
  /doevents exp_group
  /doevents campdesktop
|		/varset event_counter 1
|	} else {
|		/varcalc event_counter ${event_counter}+1
|	}
/return

SUB basics_MacroSettings
/if (${Debug} || ${Debug_Basics}) /echo |- basics_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Basics (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},General,Max Response Distance" 500
	/call WriteToIni "${genSettings_Ini},General,Leash Length" 250
	/call WriteToIni "${genSettings_Ini},General,End MedBreak in Combat(On/Off)" On
  /call WriteToIni "${genSettings_Ini},General,AutoMedBreak PctMana" 0

	/call WriteToIni "${genSettings_Ini},ExpGroup,NoKS_WeaponSet" NoExp
	/call WriteToIni "${genSettings_Ini},ExpGroup,NoKS_NukeSet" NoExp
	/call WriteToIni "${genSettings_Ini},ExpGroup,KS_WeaponSet" Main
	/call WriteToIni "${genSettings_Ini},ExpGroup,KS_NukeSet" Exp
	/call WriteToIni "${genSettings_Ini},ExpGroup,NoPoison" "Bite of the Shissar"

/if (${Debug} || ${Debug_Basics}) /echo <== basics_MacroSettings -|
/RETURN

SUB basics_CharacterSettings
/if (${Debug} || ${Debug_Basics}) /echo |- basics_CharacterSettings ==>
	/if (!${Ini[${Character_Ini},Swap Items].Length}) /call WriteToIni "${Character_Ini},Swap Items,;Melee" "Ruby of Determined Assault|Ranged"
  /if (!${Ini[${Character_Ini},Swap Items].Length}) /call WriteToIni "${Character_Ini},Swap Items,;Ranged" "Bow of the Tempest|Ranged"
	/if (!${Ini[${Character_Ini},Use Items].Length}) /call WriteToIni "${Character_Ini},Use Items,;Shrink" "Bracelet of the Shadow Hive"
/if (${Debug} || ${Debug_Basics}) /echo <== basics_CharacterSettings -|
/RETURN

Sub basics_Aliases
/if (${Debug} || ${Debug_Basics}) /echo |- basics_Aliases ==>
	/noparse /squelch /alias /followon /bc Follow ${Me.CleanName}
	/noparse /squelch /alias /followme /bc Follow ${Me.CleanName}
	/noparse /squelch /alias /followit /bc Follow ${Target.CleanName}
	/squelch /alias /followoff /bc Stop
	/squelch /alias /stop /bc Stop
	/squelch /alias /mtm /bc Move to me
  /squelch /alias /movetome /bc Move to me
	/noparse /squelch /alias /mtt /bc Move to ${Target.ID}
  /noparse /squelch /alias /movetotarget /bc Move to ${Target.ID}
	/noparse /squelch /alias /Bark /bc Bark at ${Target.ID}
	|/noparse /squelch /alias /ClickIt /bc Click object at ${Me.Loc.Replace[ ,]}
  /squelch /alias /ClickIt /bc Click It
  /noparse /squelch /alias /oClickIt /bc OldClickit ${Me.Loc.Replace[ ,]}
	/noparse /squelch /alias /RunThruZone /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
	/noparse /squelch /alias /RunThroughZone /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
	/noparse /squelch /alias /RTZ /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
	/squelch /alias /ClickYes /bc Click yes
	/squelch /alias /Yes /bc Click yes
	/squelch /alias /ClickNo /bc Click No
	/squelch /alias /No /bc Click No
	/squelch /alias /DropInvis /bc Drop Invisibility
	/squelch /alias /BotExit /bc Exit
	/squelch /alias /Gate /bc Gate
	/squelch /alias /Evac /bc Evac
	/squelch /alias /LoadSpells /bc Load Spells
	/squelch /alias /DropBuff /bc Drop Buff
	/squelch /alias /CheckBuff /bc Check Buff
	/squelch /alias /FindItem /bc Find Item
  /squelch /alias /FindMissingItem /bc Find Missing Item
	/noparse /squelch /alias /FindIt /bc Find Item ${Cursor}
	/squelch /alias /CountBots /echo Head Count
	/squelch /alias /Count /echo Head Count
	/squelch /alias /BotCount /echo Head Count
	/squelch /alias /RollCall /echo Head Count
	/squelch /alias /Swap /echo Swap
	/squelch /alias /useitem /useitem
	/squelch /alias /castitem /echo Cast Item
	/noparse /squelch /alias /ConsentMe /bc Consent ${Me.CleanName}
	/squelch /alias /BotConsent /bc Consent
	/squelch /alias /Group /bc Group
	/squelch /alias /GroupUp /bc Group
	/squelch /alias /SaveGroup /bc Save Group
	/squelch /alias /SaveGroups /bc Save Group
	/squelch /alias /listgroups /bc List Groups
	/squelch /alias /DisbandAll /bc Disband all
	/squelch /alias /DisbandBots /bc Disband all
	/squelch /alias /medon /bc MedBreak On
	/squelch /alias /medoff /bc MedBreak Off
	/squelch /alias /SetAAXP /bc Set AA XP
	/squelch /alias /PortTo /bc Port to
	/squelch /alias /FixCorpses /bc Fix Corpses
	/squelch /alias /ResetCorpses /bc Fix Corpses
	/squelch /alias /gathercorpses /echo Gather Corpses
	/squelch /alias /mana /bc Report Mana
/if (${Debug} || ${Debug_Basics}) /echo <== basics_Aliases -|
/return