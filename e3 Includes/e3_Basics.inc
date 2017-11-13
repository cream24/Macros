|------------------------------
|- Only bots in the same zone will engage follow.
|- Bots will wait to engage follow until, they can see the FollowTarget, and it is within the MaxResponseDist.
|- If bots are assisting, they will back off and follow until re-engage.
|- @param line: The line that called the event
|- @param ChatSender: The PC who sent the command
|- @param eventParams: Who to follow and /only,/not specifications
|------------------------------
#EVENT Follow "[#1#] Follow"
#EVENT Follow "<#1#> Follow"
#EVENT Follow "#1# tells you, 'Follow#2#'"
#EVENT Follow "#1# tells the group, 'Follow#2#'"
#EVENT Follow "#1# tell your party, 'Follow#2#'"
#EVENT Follow "[#1#] Follow #2#"
#EVENT Follow "<#1#> Follow #2#"
#EVENT Follow "#1# tells you, 'Follow #2#'"
#EVENT Follow "#1# tells the group, 'Follow #2#'"
#EVENT Follow "#1# tell your party, 'Follow #2#'"
SUB EVENT_Follow(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Follow ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${eventParams}]}) /return
  /declare SpawnToFollow string local ${c_eventArgData}
  /if (!${NetBots[${SpawnToFollow}].InZone} || ${SpawnToFollow.Equal[me]} || ${SpawnToFollow.Equal[on]}) /varset SpawnToFollow ${c_eventArgChatSender}
  | -Turn Following on and set the FollowTarget
  /varset FollowTarget ${Spawn[pc =${SpawnToFollow}].CleanName}
  | -If I am the follow target
  /if (${FollowTarget.Equal[${Me}]}) {
    /if (${Stick.Active}) /squelch /stick off
    /if (${NetAdvPath.Following}) /squelch /netfollow off
  } else {
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
          /varset Following TRUE
          /varset rebuffTimer 25
          /call AcquireFollow
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
SUB AcquireFollow
/if (${Debug} || ${Debug_Basics}) /echo |- AcquireFollow ==>
		/if (${SpawnCount[=${FollowTarget} radius ${MaxResponseDist}]}) {
			/if (${FollowTarget.NotEqual[${Me.CleanName}]}) {
				/if (${Stick.Active}) /squelch /stick off
				/if (${NetAdvPath.Following}) /squelch /netfollow off

				/declare followTargetID int local
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
					/if (${Spawn[id ${followTargetID}].LineOfSight}) {
						/call TrueTarget ${followTargetID}
						/squelch /netfollow on
					}
				} else {
					/if (${Spawn[id ${followTargetID}].LineOfSight}) {
						/call TrueTarget ${followTargetID}
						/squelch /stick hold 20 uw
					}
				}
				/varset FollowCheckTimer 20+${Math.Calc[${Math.Rand[${NetBots.Counts}]}/2]}
			}
		}
/if (${Debug} || ${Debug_Basics}) /echo <== AcquireFollow -|
/RETURN

|----------------------------------------------------------------------------|
|- Disengages follow on bots who are in the same zone as the ChatSender.	  -|
|- Use, /stop, in  /group, or /bc.							                            -|
|----------------------------------------------------------------------------|
#bind StopFollowing /StopFollowing
SUB Bind_StopFollowing(yLoc,xLoc)
  /if (${Stick.Active} || ${NetAdvPath.Following} || ${Following}) {
    /if (${Stick.Active}) /squelch /stick off
    /if (${NetAdvPath.Following}) /squelch /netfollow off
    /call MoveToLoc ${yLoc} ${xLoc} 20 20
    /varset FollowTarget
    /varset Following FALSE
  }
/return

|----------------------------------------------------------------------------|
#EVENT Stop	"<#1#> Stop"
#EVENT Stop "#1# tells the group, 'Stop'"
#EVENT Stop "#1# you tell your party, 'Stop'"
SUB EVENT_Stop(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Stop ==>
	/if (${Following}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZR]}) /return
      | -Wait to get close to the followTarget.
    /delay 1s ${Spawn[${FollowTarget}].Distance} < 25
    | -Stop following
    /if (${Stick.Active}) /squelch /stick off
    /if (${NetAdvPath.Following}) /squelch /netfollow off
    /varset FollowTarget NULL
    /varset Following FALSE
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Stop -|
/RETURN STOP

|----------------------------------------------------------------------------|
#EVENT MoveHere "<#1#> Move to #2#"
#EVENT MoveHere "#1# tells you, 'Move to #2#'"
#EVENT MoveHere "#1# tells the group, 'Move to #2#'"
SUB EVENT_MoveHere(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_MoveHere ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${eventParams}]}) /return
  /if (!${Int[${c_eventArgData}]}) /return
  |never move the person who issued the command
  /if (${c_eventArgChatSender.Equal[${Me}]}) /return
  /if (!${SpawnCount[id ${c_eventArgData}]}) /return
  |turn off following, assist
  /if (${Stick.Active})         /squelch /stick off
  /if (${NetAdvPath.Following}) /squelch /netfollow off
  /varset FollowTarget  NULL
  /varset Following     FALSE
  /if (${Assisting}  || ${Me.Combat}) /call AssistOff

  /call MoveToLoc ${Spawn[id ${c_eventArgData}].Loc} 50 15
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_MoveHere -|
/RETURN

|--------------------------------------------------------------------------------------|
|- Commands bots to bark a message at a target.															          -|
|- Targets can be designated by name, or ID, using -ID| or -Name|                     -|
|- @param line: '<Name> Bark at MobName BarkMsg'														          -|
|- @param msgTarget: The NPC you wish to bark at.														          -|
|- @param barkIt: What to bark at the NPC.																            -|
|- @param ChatSender: The command sender that requested bark.								          -|
|- @param selectedBots: Bots specified to bark at npc.											          -|
|- Use, '/bark MESSAGE', or 'Bark at NAME MESSAGE' can be given in /bc, /g, or /tell. -|
|--------------------------------------------------------------------------------------|
#EVENT bark "<#1#> Bark at #2# #3#"
#EVENT bark "#1# tells the group, 'Bark at #2# #3#'"
#EVENT bark "#1# tell your party, 'Bark at #2# #3#'"
SUB EVENT_bark(line, ChatSender, msgTarget, barkMsg)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Bark ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${barkMsg}]}) /return
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
  /if (${c_eventArgChatSender.Equal[${Me}]}) /delay 2s
  /varset barkTarget ${Spawn[id ${msgTarget}].CleanName}
  /varset barkReturned FALSE

  /declare retryTimer timer local 15s
  /declare startingLoc string local

  /if (${Spawn[id ${msgTarget}].Distance}>${MaxResponseDist}) {
    /echo Bark Failed - ${Spawn[id ${msgTarget}].CleanName} is > ${MaxResponseDist} distance
    /return
  }

  /if (${Stick.Active}) /squelch /stick off
  /if (${NetAdvPath.Following}) /squelch /netfollow off
  /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt

  /doevents barkReturn flush

  :retry_Bark
  /if (${Me.Invis}) /makemevisible
  /call TrueTarget ${msgTarget}
  /if (${Target.Distance} > 5) /call MoveToLoc ${Target.Y} ${Target.X} 50 5
  | -Wait a moment, then bark the message.
  /delay ${Math.Rand[8]}
  /varset startingLoc ${Me.Loc.Replace[ ,]}

  | -Reset barkMsg
  /if (${c_eventArgData.Find[-]} && ${c_eventArgData.Find[|]}) {
    /varset barkMsg ${c_eventArgData.Arg[1,-]}
    /varset barkMsg ${c_eventArgData.Left[${Math.Calc[${c_eventArgData.Length} - 1].Int}]}
  }

  /say ${barkMsg}
  | -Wait up to 2 seconds for something to happen.
  /delay 2s ${Zone.ID} != ${currentZone} || ${Me.Loc.Replace[ ,].NotEqual[${startingLoc}]} || ${Window[ConfirmationDialogBox].Open} || ${Window[LargeDialogWindow].Open}

  | -Click Yes to a confirmation box.
  /if (${Window[ConfirmationDialogBox].Open}) {
    /notify ConfirmationDialogBox Yes_Button leftmouseup
    /delay 1s
  } else /if (${Window[LargeDialogWindow].Open}) {
    /notify LargeDialogWindow LDW_YesButton leftmouseup
    /delay 1s
  }

  /doevents barkReturn
  | -Check to see if the bark was successful.
  /if (${Zone.ID} == ${currentZone} && ${Math.Distance[${startingLoc}]} < 8 && !${barkReturned}) {
    /if (${retryTimer}) {
      /goto :retry_Bark
    } else {
      /docommand ${ChatToggle} Nothing seems to have happened...
    }
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
	/varset barkReturned TRUE
/RETURN

|----------------------------------------------------------------------------|
|- Moves to a given location, and clicks the nearest object.				        -|
|----------------------------------------------------------------------------|
#EVENT clickIt "<#1#> Click It#2#"
#EVENT clickIt "#1# tells the group, 'Click It#2#'"
#EVENT clickIt "#1# tell your party, 'Click It#2#'"
SUB EVENT_clickIt(line, ChatSender, eventParams)
  /declare debug_clickit bool local FALSE
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${eventParams}]}) /return
  /if (${c_eventArgChatSender.Equal[${Me}]}) /delay 2s
  /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
  /declare i int local
  /declare closestDoorID int local 0
  /declare closestDoorDist float local 0
  |load array of doors from doors.ini:  doorid, x, y, z, name
  /call IniToArrayV "e3 Macro Inis\doors.ini,${Zone.ShortName},${Zone.ShortName}#" doorList
  /if (!${Defined[doorList]}) {
    /if (${Me.CleanName.Equal[Creamo]}) /bc I have no doorList for this zone
  } else {
    /if (${debug_clickit}) /echo Doors in zone ${doorList.Size}
    |find the closest door to me
    /for i 1 to ${doorList.Size}
      /if (${debug_clickit}) /echo ${i} ${doorList[${i}]}
      /squelch /doortarget id ${doorList[${i}].Arg[1,,]}
      /if ((${closestDoorDist}==0 || ${DoorTarget.Distance} < ${closestDoorDist}) && ${DoorTarget.DistanceZ} < 75) {
        /varset closestDoorDist ${DoorTarget.Distance}
        /varset closestDoorID ${doorList[${i}].Arg[1,,]}
      }
    /next i
  }
  | if i have a door within 100 distance then move to it and try to click
  /if (${debug_clickit}) /echo closest ${closestDoorID} ${closestDoorDist}
  /if (!${Bool[${closestDoorID}]}) {
    /squelch /doortarget
    /if (${DoorTarget.Distance} < ${MaxResponseDist}) {
      /varset closestDoorID ${Switch.ID}
      /if (${debug_clickit}) /echo no door in file, picked closest door ${Switch.ID}
    } else {
      /echo \ay Aborting ClickIt: I could not find an acceptable door < ${MaxResponseDist} units
      /return
    }
  }
  /squelch /doortarget id ${closestDoorID}
  /if (${DoorTarget.Distance} > 20) /call MoveToLoc ${DoorTarget.Y} ${DoorTarget.X} 50 15
  /if (${DoorTarget.Distance} > 20) {
    /echo \ay Aborting ClickIt: I could not moveto < 20 units of ${DoorTarget.Name} within 5s
    /moveto off
    /return
  } else {
    /declare MyZone int local ${Zone.ID}
    /declare clickTimer timer local 5s
    |add starting loc vs final loc for intrazone click
    /declare clickDelay int local ${Math.Calc[${Math.Rand[${Me.ID}]}%${clickitRandomDelay}|1]}
    /declare startingLoc ${Me.Loc}
    :retryClickDoor
    /delay ${clickDelay}
    /squelch /doortarget id ${closestDoorID}
    /squelch /click left door
    /if (${MyZone}==${Zone.ID} && ${Math.Distance[${startingLoc}]} < 10) {
      /if (${clickTimer}) {
        /goto :retryClickDoor
      } else {
        /echo \ay I have failed to click Door ${Switch.ID} ${DoorTarget.Name}
        /moveto off
        /return
      }
    }
  }
/RETURN

|--------------------------------------------------------------------------|
|- Moves to a given location, and clicks the nearest object.				      -|
|- This is only here to support a situation wheres doors.txt lacks a door -|
|- @param ChatSender: The command sender.									                -|
|- @param locToStart: Where to stand when attempting to click the object.	-|
|--------------------------------------------------------------------------|
#EVENT oldclickIt "<#1#> OldClickIt #2#"
#EVENT oldclickIt "#1# tells the group, 'OldClickIt #2#'"
#EVENT oldclickIt "#1# tell your party, 'OldClickIt #2#'"
SUB EVENT_oldclickIt(line, ChatSender, locToStart)
  /if (${Debug} || ${Debug_Basics}) /echo |- EVENT_clickIt ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${locToStart}]}) /return
  | -Check LoS.
  /if (!${LineOfSight[${Me.Loc.Replace[ ,]}:${c_eventArgData}]}) {
    /docommand ${ChatToggle} I cannot see the object to click.
  } else {
    /if (${locToStart.Find[ ]}) /varset locToStart ${locToStart.Arg[1, ]}
    | -Check range to object.
    /if (${Math.Distance[${locToStart}]} > ${Math.Calc[${MaxResponseDist}*3].Int}) {
      /docommand ${ChatToggle} That object is too far away to click.
    } else {
      /declare retryTimer timer local 15s
      /declare miscTimer timer local
      /declare startZone int local ${Zone.ID}
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Following}) /squelch /netfollow off
      /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /call interrupt
      :retryClick
      /if (${Math.Distance[${locToStart}]} > 10) /call MoveToLoc ${locToStart} 30 20
      /if (${Math.Distance[${locToStart}]} > 10) {
        /docommand ${ChatToggle} I failed to move to the inteded object
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
        /if (${Zone.ID} == ${currentZone} && ${Math.Distance[${locToStart}]} < 50) {
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
  /if (${Debug} || ${Debug_Basics}) /echo <== EVENT_clickIt -|
/RETURN

|----------------------------------------------------------------------------|
|- Points faces bots to your current heading, and runs through a zone line.	-|
|- @param ChatSender: The command sender.									                  -|
|- @param locToStart: Where to stand when trying to click the door.			    -|
|- @param snapHeading: The direction to face when trying to click the door.	-|
|----------------------------------------------------------------------------|
#EVENT runThruZone "<#1#> Run through zone#2#"
#EVENT runThruZone "#1# tells the group, 'Run through zone#2#'"
#EVENT runThruZone "#1# tell your party, 'Run through zone#2#'"
SUB EVENT_runThruZone(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_runThruZone ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${eventParams}]}) /return
  /if (${c_eventArgChatSender.Equal[${Me}]}) /delay 2s
  /declare locToStart string local ${Spawn[pc ${c_eventArgChatSender}].Loc}
  /declare startHeading float local ${Spawn[pc ${c_eventArgChatSender}].Heading.Degrees}
  /declare retryTimer timer local 7s
  /declare miscTimer timer local
  /declare startZone int local ${Zone.ID}
  | -Check that you can see the location to start from.
  /if (!${LineOfSight[${Me.Loc.Replace[ ,]}:${locToStart}]}) {
    /docommand ${ChatToggle} I cannot see the starting location.
  } else {
    | -Check range to object.
    /if (${Math.Distance[${locToStart}]} > ${Math.Calc[${MaxResponseDist}*3].Int}) {
      /docommand ${ChatToggle} I am to far away from that zone line.
    } else {
      /if (${Stick.Active}) /squelch /stick off
      /if (${NetAdvPath.Following}) /squelch /netfollow off
      /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) /delay 5s !${Me.Casting.ID}
    :retryRTZ
      | -Check if I need to move closer.
      /if (${Math.Distance[${locToStart}]} > 10) /call MoveToLoc ${locToStart} 50
      | -Face the zone and run foward for 6 seconds.
      /face fast heading ${Math.Calc[${startHeading}*-1]}
      /delay 2
      /keypress forward hold
      | -Wait to zone.
      /delay 6s !${Zone.ID} || ${Zone.ID} != ${startZone}
      | -Check if I zoned.
      /if (${Zone.ID} == ${startZone}) {
        /if (${retryTimer}) {
          /goto :retryRTZ
        } else {
          /docommand ${ChatToggle} I failed to run through the zoneline in [${Zone}].
          /call MoveToLoc ${locToStart} 30 20
        }
      }
    }
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_runThruZone -|
/RETURN

|----------------------------------------|
#EVENT dropInvis "<#1#> Drop Invis#*#"
SUB EVENT_dropInvis(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_dropInvis ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR]}) /return
  /if (${Me.Invis}) /makemevisible
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_dropInvis -|
/RETURN

|----------------------------------------|
#EVENT dropBuff "<#1#> Drop Buff #2#"
SUB EVENT_dropBuff(line, ChatSender, buffName)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_dropInvis ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR,${buffName}]}) /return
  /if (${c_eventArgData.Equal[All]}) {
    /declare i int local 1
    /for i 1 to 25
      /if (${Me.Buff[${i}].ID}) {
        /squelch /removebuff ${Me.Buff[${i}].Name}
        /delay 1
      }
    /next i
  } else {
    /removebuff ${c_eventArgData}
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
  /if (!${checkEventArgs[${ChatSender},${line},U]}) /return
  /docommand ${ChatToggle} Bye!
  /if (${Spawn[${ChatSender}].ID}!=${Me.ID}) /exit
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Exit -|
/RETURN

|------------------------------------------------------------|
|- Attempts to load the spellset listed in e3_Settings.ini.	-|
|------------------------------------------------------------|
#EVENT LoadSpells "<#1#> Load Spells"
SUB EVENT_LoadSpells(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_LoadSpells ==>
  /if (!${checkEventArgs[${ChatSender},${line},U]}) /return
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
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_LoadSpells -|
/RETURN

|--------------------------------------------------------|
|- Checks for specified buffs and debuffs on NetBots.		-|
|- Use, /bc Check buff Name.	                          -|
|--------------------------------------------------------|
#EVENT CheckBuffs "<#1#> Check buff #2#"
SUB EVENT_CheckBuffs(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_CheckBuffs ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${eventParams}]}) /return
  /if (${Bool[${Me.Buff[${c_eventArgData}]}]}) {
      /docommand ${ChatToggle} ${Me.Buff[${c_eventArgData}].Duration.TotalSeconds} seconds left on [${c_eventArgData}].
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_CheckBuffs -|
/RETURN

|--------------------------------------------------------|
|- Searches inventory, and banks, for indicated items.	-|
|- Use, /bc Find 'Name', /echo Find 'Name'.			        -|
|- Accepts item links as 'Name'                         -|
|--------------------------------------------------------|
#EVENT FindItems "<#1#> Find Item #2#"
SUB EVENT_FindItems(line, ChatSender, ItemToFind)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_FindItems ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,${ItemToFind}]}) /return
  /declare reportDelay int local 30
  |parse item link
  /if (${c_eventArgData.Find[000]}) {
    /varset ItemToFind ${c_eventArgData.Right[${Math.Calc[${c_eventArgData.Length} - 57]}].Left[-1]}
  } else {
    /varset ItemToFind ${c_eventArgData}
  }
  /declare i int local
  /declare e int local
  /declare a int local
  /declare totalItems int local 0
  | -Check cursor.
  /if (${Cursor.Name.Equal[${ItemToFind}]}) {
    /varcalc totalItems ${totalItems}+${Cursor.Stack}
    /docommand /timed ${reportDelay} ${ChatToggle} >>> Found ${Cursor.Name}(${Cursor.Stack})-> [Cursor] <<<
  }
  /if (${FindItemCount[${ItemToFind}]}) {
    | -Search Equippped Items.
    /for i 1 to 22
      /if (${InvSlot[${i}].Item.Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+${InvSlot[${i}].Item.Stack}
        /docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item}(${InvSlot[${i}].Item.Stack})-> [${InvSlot[${i}].Name}]
      }
      /for a 0 to ${InvSlot[${i}].Item.Augs}
        /if (${InvSlot[${i}].Item.AugSlot[${a}].Name.Find[${ItemToFind}]}) {
          /varcalc totalItems ${totalItems}+1
          /docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item.AugSlot[${a}]}-> [${InvSlot[${i}].Item}-> ${InvSlot[${i}].Name}]
        }
      /next a
    /next i
    | -Search packslots.
    /for i 1 to 8
      /if (${Me.Inventory[pack${i}].Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Stack}
        /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}]}(${Me.Inventory[pack${i}].Stack})-> InvSlot${i}]
      }
      /if (${Me.Inventory[pack${i}].Container}) {
        /for e 1 to ${Me.Inventory[pack${i}].Container}
          /if (${Me.Inventory[pack${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
            /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Item[${e}].Stack}
            /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}]}(${Me.Inventory[pack${i}].Item[${e}].Stack})-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
          }
          /for a 0 to ${Me.Inventory[pack${i}].Item[${e}].Augs}
            /if (${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
              /varcalc totalItems ${totalItems}+1
              /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Inventory[pack${i}].Item[${e}]}]-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
            }
          /next a
        /next e
      }
    /next i
  }
  | -Search bank packslots.
    /for i 1 to 26
      /if (${Me.Bank[${i}].Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Stack}
        /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}]}(${Me.Bank[${i}].Stack}) ->[BankSlot${i}]
      }
      /if (${Me.Bank[${i}].Container}) {
        /for e 1 to ${Me.Bank[${i}].Container}
          /if (${Me.Bank[${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
            /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Item[${e}].Stack}
            /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}]}(${Me.Bank[${i}].Item[${e}].Stack})-> [${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}]
          }
          /for a 0 to ${Me.Bank[${i}].Item[${e}].Augs}
            /if (${Me.Bank[${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
              /varcalc totalItems ${totalItems}+1
              /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Bank[${i}].Item[${e}]}]-> ${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}
            }
          /next a
        /next e
      }
    /next i
  /if (${totalItems} > 0) /docommand ${ChatToggle} >> Found (${totalItems}) : [${ItemToFind}] <<

/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_FindItems -|
/RETURN
|--------------------------------------------------------|
#EVENT FindItemCompact "<#1#> Find ItemC #2#"
SUB EVENT_FindItemCompact(line, ChatSender, ItemToFind)
  /if (${Debug} || ${Debug_Basics}) /echo |- EVENT_FindItems ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,${ItemToFind}]}) /return
  /declare reportDelay int local 1
  /declare reportArray[50] string local NIL
  /declare raINDX int local 1
|parse item link
  /if (${c_eventArgData.Find[000]}) {
    /varset ItemToFind ${c_eventArgData.Right[${Math.Calc[${c_eventArgData.Length} - 57]}].Left[-1]}
  } else {
    /varset ItemToFind ${c_eventArgData}
  }
  /declare i int local
  /declare e int local
  /declare a int local
  /declare r int local
  /declare totalItems int local 0
| -Check cursor.
  /if (${Cursor.Name.Equal[${ItemToFind}]}) {
    /varcalc totalItems ${totalItems}+${Cursor.Stack}
    /varset reportArray[${raINDX}] [Cursor]${Cursor.Name}(${Cursor.Stack}
    /varcalc raINDX ${raINDX}+1
    |/docommand /timed ${reportDelay} ${ChatToggle} >>> Found ${Cursor.Name}(${Cursor.Stack})-> [Cursor] <<<
  }
  /if (${FindItemCount[${ItemToFind}]}) {
    | -Search Equippped Items.
    /for i 1 to 22
      /if (${InvSlot[${i}].Item.Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+${InvSlot[${i}].Item.Stack}
        /varset reportArray[${raINDX}] [Worn]${InvSlot[${i}].Item}(${InvSlot[${i}].Item.Stack}
        |/echo ${raINDX} ${reportArray[${raINDX}]} ${InvSlot[${i}].Item}
        /varcalc raINDX ${raINDX}+1
        |/docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item}(${InvSlot[${i}].Item.Stack})-> [${InvSlot[${i}].Name}]
      }
      /for a 0 to ${InvSlot[${i}].Item.Augs}
        /if (${InvSlot[${i}].Item.AugSlot[${a}].Name.Find[${ItemToFind}]}) {
          /varcalc totalItems ${totalItems}+1
          /varset reportArray[${raINDX}] [Worn]${InvSlot[${i}].Item.AugSlot[${a}]}(1
          /varcalc raINDX ${raINDX}+1
          |/docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item.AugSlot[${a}]}-> [${InvSlot[${i}].Item}-> ${InvSlot[${i}].Name}]
        }
      /next a
    /next i
    | -Search packslots.
    |/echo p${raINDX}
    /for i 1 to 8
    /if (${Me.Inventory[pack${i}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Stack}
      /varset reportArray[${raINDX}] [Pack]${Me.Inventory[pack${i}]}(${Me.Inventory[pack${i}].Stack}
      |/echo [Pack]${Me.Inventory[pack${i}]}
      /varcalc raINDX ${raINDX}+1
      |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}]}(${Me.Inventory[pack${i}].Stack})-> InvSlot${i}]
    }
    /if (${Me.Inventory[pack${i}].Container}) {
      /for e 1 to ${Me.Inventory[pack${i}].Container}
        /if (${Me.Inventory[pack${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
          /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Item[${e}].Stack}
          /varset reportArray[${raINDX}] [Bag]${Me.Inventory[pack${i}].Item[${e}]}(${Me.Inventory[pack${i}].Item[${e}].Stack}
          |/echo [Bag]${Me.Inventory[pack${i}].Item[${e}]}
          /varcalc raINDX ${raINDX}+1
          |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}]}(${Me.Inventory[pack${i}].Item[${e}].Stack})-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
        }
        /for a 0 to ${Me.Inventory[pack${i}].Item[${e}].Augs}
        /if (${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
          /varcalc totalItems ${totalItems}+1
          /varset reportArray[${raINDX}] [Bag]${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}]}(1
          |/echo [Bagaug]${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}]}
          /varcalc raINDX ${raINDX}+1
          |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Inventory[pack${i}].Item[${e}]}]-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
        }
        /next a
      /next e
    }
    /next i
  }
| -Search bank packslots.
  |/echo b${raINDX}
  /for i 1 to 26
  /if (${Me.Bank[${i}].Name.Find[${ItemToFind}]}) {
    /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Stack}
    /varset reportArray[${raINDX}] [Bank]${Me.Bank[${i}]}(${Me.Bank[${i}].Stack}
    /varcalc raINDX ${raINDX}+1
    |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}]}(${Me.Bank[${i}].Stack}) ->[BankSlot${i}]
  }
  /if (${Me.Bank[${i}].Container}) {
    /for e 1 to ${Me.Bank[${i}].Container}
    /if (${Me.Bank[${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Item[${e}].Stack}
      /varset reportArray[${raINDX}] [Bank]${Me.Bank[${i}].Item[${e}]}(${Me.Bank[${i}].Item[${e}].Stack}
      /varcalc raINDX ${raINDX}+1
      |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}]}(${Me.Bank[${i}].Item[${e}].Stack})-> [${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}]
    }
    /for a 0 to ${Me.Bank[${i}].Item[${e}].Augs}
    /if (${Me.Bank[${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+1
      /varset reportArray[${raINDX}] [Bag]${Me.Bank[${i}].Item[${e}].AugSlot[${a}]}(1
      /varcalc raINDX ${raINDX}+1
      |/docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Bank[${i}].Item[${e}]}]-> ${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}
    }
    /next a
    /next e
  }
  /next i

  |/echo ${raINDX} ${reportArray[1]}
  /for r 1 to ${raINDX}
    /if (${reportArray[${r}].NotEqual[NIL]}) /docommand ${ChatToggle} ${reportArray[${r}]}/${totalItems})
  /next r
  |/if (${totalItems} > 0) /docommand ${ChatToggle} >> Found (${totalItems}) : [${ItemToFind}] <<

  /if (${Debug} || ${Debug_Basics}) /echo <== EVENT_FindItems -|
/RETURN
|--------------------------------------------------------|
|- Finditem with no summary data for easier viewing
|--------------------------------------------------------|
SUB FindItemSub(ItemToFind,reportTotal,reportDelay)
  /declare i int local
  /declare e int local
  /declare a int local
  /declare totalItems int local 0
  | -Check cursor.
  /if (${Cursor.Name.Equal[${ItemToFind}]}) {
    /varcalc totalItems ${totalItems}+${Cursor.Stack}
    /docommand /timed ${reportDelay} ${ChatToggle} >>> Found ${Cursor.Name}(${Cursor.Stack})-> [Cursor] <<<
  }
  /if (${FindItemCount[${ItemToFind}]}) {
    | -Search Equippped Items.
    /for i 1 to 22
    /if (${InvSlot[${i}].Item.Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+${InvSlot[${i}].Item.Stack}
      /docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item}(${InvSlot[${i}].Item.Stack})-> [${InvSlot[${i}].Name}]
    }
    /for a 0 to ${InvSlot[${i}].Item.Augs}
    /if (${InvSlot[${i}].Item.AugSlot[${a}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+1
      /docommand /timed ${reportDelay} ${ChatToggle} ${InvSlot[${i}].Item.AugSlot[${a}]}-> [${InvSlot[${i}].Item}-> ${InvSlot[${i}].Name}]
    }
    /next a
    /next i
    | -Search packslots.
    /for i 1 to 8
    /if (${Me.Inventory[pack${i}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Stack}
      /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}]}(${Me.Inventory[pack${i}].Stack})-> InvSlot${i}]
    }
    /if (${Me.Inventory[pack${i}].Container}) {
      /for e 1 to ${Me.Inventory[pack${i}].Container}
      /if (${Me.Inventory[pack${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+${Me.Inventory[pack${i}].Item[${e}].Stack}
        /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}]}(${Me.Inventory[pack${i}].Item[${e}].Stack})-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
      }
      /for a 0 to ${Me.Inventory[pack${i}].Item[${e}].Augs}
      /if (${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
        /varcalc totalItems ${totalItems}+1
        /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Inventory[pack${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Inventory[pack${i}].Item[${e}]}]-> [${Me.Inventory[pack${i}]}-> InvSlot${i}-> PackSlot${e}]
      }
      /next a
      /next e
    }
    /next i
  }
  | -Search bank packslots.
  /for i 1 to 26
  /if (${Me.Bank[${i}].Name.Find[${ItemToFind}]}) {
    /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Stack}
    /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}]}(${Me.Bank[${i}].Stack}) ->[BankSlot${i}]
  }
  /if (${Me.Bank[${i}].Container}) {
    /for e 1 to ${Me.Bank[${i}].Container}
    /if (${Me.Bank[${i}].Item[${e}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+${Me.Bank[${i}].Item[${e}].Stack}
      /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}]}(${Me.Bank[${i}].Item[${e}].Stack})-> [${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}]
    }
    /for a 0 to ${Me.Bank[${i}].Item[${e}].Augs}
    /if (${Me.Bank[${i}].Item[${e}].AugSlot[${a}].Name.Find[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+1
      /docommand /timed ${reportDelay} ${ChatToggle} ${Me.Bank[${i}].Item[${e}].AugSlot[${a}]}-> [${Me.Bank[${i}].Item[${e}]}]-> ${Me.Bank[${i}]}-> BankSlot${i}-> PackSlot${e}
    }
    /next a
    /next e
  }
  /next i
  /if (${Bool[${reportTotal}]} && ${totalItems} > 0) /docommand ${ChatToggle} >> Found (${totalItems}) : [${ItemToFind}] <<
/RETURN
|------------------------------------------------------------------|
|- Reports which character lacks a specified item                 -|
|- 'Name' must be the full itemname, accepts item links as 'Name' -|
|- Use, /bc FindMissingItem 'Name', /echo FindMissingItem 'Name'	-|
|------------------------------------------------------------------|
#EVENT FindMissingItem "<#1#> Find Missing Item #2#"
SUB EVENT_FindMissingItem(line, ChatSender, ItemToFind)
  /if (!${checkEventArgs[${ChatSender},${line},U,${ItemToFind}]}) /return
  |parse item link
  /if (${c_eventArgData.Find[000]}) {
    /varset ItemToFind ${c_eventArgData.Right[${Math.Calc[${c_eventArgData.Length} - 57]}].Left[-1]}
  } else {
    /varset ItemToFind ${c_eventArgData}
  }
  /declare i int local
  /declare e int local
  /declare a int local
  /declare totalItems int local ${FindItemCount[=${ItemToFind}]}
  /varcalc totalItems ${totalItems}+${Cursor.Name.Equal[=${ItemToFind}]}

  /for i 1 to 26
    /if (${Me.Bank[${i}].Name.Equal[${ItemToFind}]}) {
      /varcalc totalItems ${totalItems}+1
    }
    /if (${Me.Bank[${i}].Container}) {
      /for e 1 to ${Me.Bank[${i}].Container}
        /if (${Me.Bank[${i}].Item[${e}].Name.Equal[${ItemToFind}]}) {
          /varcalc totalItems ${totalItems}+1
        }
        /for a 0 to ${Me.Bank[${i}].Item[${e}].Augs}
          /if (${Me.Bank[${i}].Item[${e}].AugSlot[${a}].Name.Equal[${ItemToFind}]}) {
            /varcalc totalItems ${totalItems}+1
          }
        /next a
      /next e
    }
  /next i

  /if (${totalItems}==0) /docommand ${ChatToggle} >> I did not find ${ItemToFind} <<
/RETURN

|------------------------------------------------------------|
|**
SUB LPAD_NAME_BC(myName, textToPad)
  /declare i int local
  /declare maxLength int local 0
  /declare numToPad int local
  /declare paddedText string local
  /for i 1 to ${NetBots.Counts}
    /if(${NetBots.Client[${i}].Length} > ${maxLength}) /varset maxLength ${NetBots.Client[${i}].Length}
  /next i

  /varcalc numToPad ${maxLength}-${Me.Nam.Length}
  /for i 1 to ${numToPad}
    /varset paddedText _${paddedText}
    /echo ${paddedText}
  /next i
/RETURN ${paddedText}
**|
|------------------------------------------------------------|
#EVENT FindSlot "<#1#> Find Slot #2#"
SUB EVENT_FindSlot(line, ChatSender, SlotToFind)
  /if (!${checkEventArgs[${ChatSender},${line},U,${SlotToFind}]}) /return
  /if (${Select[${c_eventArgData},charm,leftear,head,face,rightear,neck,shoulder,arms,back,leftwrist,rightwrist,ranged,hands,mainhand,offhand,leftfinger,rightfinger,chest,legs,feet,waist,powersource,ammo]}) {
    /say ${c_eventArgData}: ${LinkDB[${InvSlot[${c_eventArgData}].Item}]}
  } else {
    /if (${c_eventArgChatSender.Equal[${Me}]}) {
      /echo Valid Slots: charm, leftear, head, face, rightear, neck, shoulder, arms, leftwrist, rightwrist, ranged, hands, mainhand, offhand, ammo, back, leftfinger, rightfinger, chest, legs, feet, waist, powersource
    }
  }
/RETURN
|------------------------------------------------------------|
|- Checks for bots who are out of range, or out of zone.	  -|
|- Use, /count, or, /bc Count bots							            -|
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
|- Preforms item swaps, listed in Character_Inis.	  -|
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
		/if (${Bool[${Me.Casting}]} && ${Me.Class.ShortName.NotEqual[BRD]}) {
		  /delay 3s !${Me.Casting.ID}
      /call interrupt
      /delay 3s !${Me.Casting.ID}
    }
		/declare i int local 1
		/declare itemToSwap string local
		/declare slotToSwap string local
		
		:SwapLoop
		/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Swap_Items -| Swaploop.
		
		/varset itemToSwap ${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Arg[1,|]}
		/varset slotToSwap ${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Arg[2,|]}
		
		| If I don't have the item in my inventory.
		|/echo ${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[1,/].Length}
		/if (!${FindItemCount[${itemToSwap}]}) {
			/echo |- Event Swap_Items -| #*# Error #*# [${itemToSwap}] was not found.
      /varcalc i ${i} + 1
			/if (${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Length}) /goto :SwapLoop
		} else {
			| If I'm swapping in a 2 hander, while dual wielding.
			/if (${slotToSwap.Equal[Mainhand]} && ${FindItem[${itemToSwap}].Type.Left[2].Equal[2H]} && ${Bool[${InvSlot[Offhand].Item}]}) {
			  /unequip Offhand
			  /delay 2
      }
      /if (${slotToSwap.Equal[Mainhand]} && ${FindItem[${itemToSwap}].ItemSlot}==14) {
        /unequip Offhand
        /delay 2
      }
      /if (${slotToSwap.Equal[Offhand]} && ${FindItem[${itemToSwap}].ItemSlot}==13) {
        /unequip Mainhand
        /delay 2
      }
			| Swap in item.
      |/if (${Debug} || ${Debug_Basics})
      /echo Equipping [${itemToSwap}] ${slotToSwap}
			/call SwapItem "${itemToSwap}" ${slotToSwap}
			/delay 2
			/varcalc i ${i} + 1
			/if (${Ini[${Character_Ini},Swap Items,${IniEntry}].Arg[${i},/].Length}) /goto :SwapLoop
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== Swap_Items -|
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
  /if (!${checkEventArgs[${ChatSender},${line},UZR]}) /return
  /if (${Me.Class.ShortName.NotEqual[BRD]}) {
      /docommand ${ChatToggle} Meditating...
      /varset medBreak TRUE
      /if (${line.Find[ Hold]}) {
        /varset medBreak_Hold TRUE
      } else {
        /varset medBreak_Hold FALSE
      }
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_medOn -|
/RETURN

|----------------------------------------------------|
|- disengages MedBreak mode, on bots who can cast.	-|
|----------------------------------------------------|
#EVENT medOff "<#1#> MedBreak Off"
SUB EVENT_medOff(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_medOff ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZR]}) /return
  /if (${medBreak}) {
    /docommand ${ChatToggle} Ending Medbreak.
    /varset medBreak FALSE
    /varset medBreak_Hold FALSE
    /if (${Me.Sitting}) /stand
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_medOff -|
/RETURN

|--------------------------------------------------------------------------------|
|- Automatically disengages and re-engages MedBreak when conditions are met.	-|
|- Ends MedBreak mode at 99% mana, unless, 'MedBreak hold command is given.		-|
|--------------------------------------------------------------------------------|
SUB check_MedBreak
/if (${Debug} || ${Debug_Basics}) /echo |- check_MedBreak ==>
	| If End MedBreak in Combat (On/Off)=On, and a netbot is in combat, call medOff
	/if (${medOn_combatBreak} && ${Me.CombatState.Equal[COMBAT]}) {
    /docommand ${ChatToggle} Ending Medbreak.
    /varset medBreak FALSE
    /varset medBreak_Hold FALSE
    /if (${Me.Sitting}) /stand
	}
	/if (!${Me.Feigning}) {
		/if (!${medbreak_Popup_Timer}) {
			/popup ***Meditating***
			/varset medbreak_Popup_Timer 30
		}
		| end MedBreak when we are full mana or end for melee, unless hold is engaged
		/if (${medBreak} && !${medBreak_Hold} && (${Me.PctMana} >= 99 || (${Me.MaxMana} ==0 && ${Me.PctEndurance}>=99))) {
			/docommand ${ChatToggle} Full Mana/Endurance, ending MedBreak.
			/if (${Me.Sitting}) /stand
			/varset medBreak FALSE
			/varset medBreak_Hold FALSE				
		} else {
      /if (!${Me.Sitting} && !${Me.Casting.ID} && !${Me.Moving}) /sit
		}
	}
/if (${Debug} || ${Debug_Basics}) /echo <== check_MedBreak -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT set_AAXP "<#1#> Set AA XP #2#"
#EVENT set_AAXP "[#1#] Set AA XP #2#"
SUB EVENT_set_AAXP(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_Basics}) /echo |- set_AAXP ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,${eventParams}]}) /return
  /varset eventParams ${Int[${c_eventArgData}]}
  /echo Setting AA XP [${eventParams}%].
  /alternateadv ON ${eventParams}
/if (${Debug} || ${Debug_Basics}) /echo <== set_AAXP -|
/RETURN

|--------------------------------------------------------------------------------|
#EVENT fixCorpses "<#*#> Fix Corpses"
SUB EVENT_fixCorpses
	/echo Attempting to reset corpse positions...
	/hidecorpse all
	/delay 2
	/hidecorpse none
	/delay 2
	/hidecorpse looted
/RETURN

|----------------------------------------|
|- Consents a PC to drag your corpse.	-|
|----------------------------------------|
#event Consent "#1# tells you, 'Consent #2#'"
#event Consent "#1# tells the raid#*#Consent #2#'"
#event Consent "<#1#> Consent #2#"
SUB Event_Consent(line, ChatSender, ConsentTarget)
  /if (${ConsentTarget.NotEqual[${Me.CleanName}]}) /consent ${ConsentTarget}
/RETURN

|--------------------------------------------------------------------------------|
#event gatherCorpses "[MQ2] Gather corpse#*#"
SUB event_gatherCorpses
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_gatherCorpses ==>
  /if (${Raid.Members}) {
    /rs Consent ${Me.CleanName}
  } else {
    /bc Consent ${Me.CleanName}
  }
	/declare i int local
	/for i 1 to ${SpawnCount[pccorpse radius 100]}
    /call TrueTarget ${Spawn[${NearestSpawn[${i},pccorpse radius 100]}].ID}
    /delay 2
    /corpse
    /delay 1s ${Target.Distance} < 20
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
  /if (!${checkEventArgs[${ChatSender},${line},U,${eventParams}]}) /return
  /declare buttonToClick string local ${If[${c_eventArgData.Find[yes]},Yes,No]}
  /declare miscTimer timer local 5s
  /if (${c_eventArgChatSender.Equal[${Me}]}) /delay 2s
  :clickButton
  /if (${Window[ConfirmationDialogBox].Open}) {
    /notify ConfirmationDialogBox ${buttonToClick}_Button leftmouseup
  } else /if (${Window[LargeDialogWindow].Open}) {
    /notify LargeDialogWindow LDW_${buttonToClick}Button leftmouseup
  }
  /if (${miscTimer}) {
    /if (${Window[ConfirmationDialogBox].Open} || ${Window[LargeDialogWindow].Open}) /goto :clickButton
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
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (!${Me.Book[gate]}) {
    /if (${FindItem[Philter of Major Translocation].ID}) {
      /docommand ${ChatToggle} Preparing to gate...
      /casting "Philter of Major Translocation" -maxtries|2
    } else /if (${FindItem[Vial of Swirling Smoke].ID}) {
      /docommand ${ChatToggle} Preparing to gate...
      /casting "Vial of Swirling Smoke" -maxtries|2
    } else {
      /docommand ${ChatToggle} I am not able to gate at this time.
    }
  } else {
    /docommand ${ChatToggle} Preparing to gate...
    /call interrupt
    /delay 3s !${Me.Casting.ID}
    /casting "Gate" -maxtries|2
    /delay 3s !${Me.Casting.ID}
  }
/RETURN

|--------------------------------------------------------|
|- Request evac from bots set to Evac_OnCommand.		-|
|--------------------------------------------------------|
|- Use, /bc Evac, /tell 'Name' Evac
|--------------------------------------------------------|
#EVENT Evacuate "[#1#] Evac"
#EVENT Evacuate	"<#1#> Evac"
#EVENT Evacuate "#1# tells you, 'Evac'"
#EVENT Evacuate "#1# tell your party, 'Evac'"
#EVENT Evacuate "#1# tells the group, 'Evac'"
SUB EVENT_Evacuate(line, ChatSender)
/if (${Debug} || ${Debug_Basics}) /echo |- EVENT_Evacuate ==>
  /if (${Select[${Me.Class.ShortName},DRU,WIZ]}) {
    /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
    /if (${Bool[${Group.Member[${c_eventArgChatSender}]}]} && ${NetBots.Counts}>6) /delay 1s
    /if (${Me.AltAbilityReady[Exodus]}) {
      /delay 3
      /docommand ${ChatToggle} Evacuating!
      /call interrupt
      /delay 3s !${Me.Casting.ID}
      /casting "Exodus" alt
    } else /if (${Me.Book[${Evac_Spell.Arg[1,/]}]}) {
      /docommand ${ChatToggle} Evacuating!
      /call interrupt
      /delay 3s !${Me.Casting.ID}
      /casting "${Evac_Spell.Arg[1,/]}" -maxtries|2
      /delay 3s !${Me.Casting.ID}
    } else {
      /docommand ${ChatToggle} I am not able to Evac at this time.
    }
  }
/if (${Debug} || ${Debug_Basics}) /echo <== EVENT_Evacuate -|
/RETURN

|----------------------------------------|
|- Automatically accepts raid invites.	-|
|----------------------------------------|
#EVENT RaidAccept "#*#invites you to join a raid.#*#"
SUB EVENT_RaidAccept
  /delay 5
  /notify ConfirmationDialogBox Yes_Button leftmouseup
  /delay 5
  /raidaccept
/RETURN

|----------------------------------------|
|- Automatically accepts group invites.	-|
|----------------------------------------|
#EVENT Invite "#1# invites you to join a group."
SUB EVENT_Invite(line, ChatSender)
/if (${Debug}) /echo |- EVENT_Invite ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,""]}) /return
  /if (${Me.Casting.ID} && ${Me.Class.ShortName.NotEqual[BRD]}) {
    /call interrupt
    /delay 5s !${Me.Casting.ID}
  }
  /declare retryTimer timer local 5s
  :retry_Invite
  /squelch /target clear
  /delay 3
  /invite
  /delay 1s ${Bool[${Group}]}
  /if (!${Bool[${Group}]}) {
    /if (${retryTimer}) {
      /goto :retry_Invite
    } else {
      /echo Failed to join the group.
    }
  }
/if (${Debug}) /echo <== EVENT_Invite -|
/RETURN

|--------------------------------------------------------------------------------------------|
|- Groupleaders save current group formations to the Groups.ini file, to be recalled later.	-|
|--------------------------------------------------------------------------------------------|
|- Use, /savegroup 'Name', /bc Save Group 'Name', or /bc SetGroup 'Name'					-|
|--------------------------------------------------------------------------------------------|
#EVENT saveGroup "<#1#> Save Group #2#"
SUB EVENT_saveGroup(line, ChatSender, eventParams)
/if (${Debug}) /echo |- EVENT_saveGroup ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,${eventParams}]}) /return
  /if (!${Select[${Group.Leader},${Me}]}) {
    /if (${Debug}) /echo I am not a group leader, skipping.
  } else {
    /echo Preparing to save...
    /declare groupName string local ${c_eventArgData}
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
    /delay 2s
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
SUB EVENT_groupUp(line, ChatSender, createGroup)
/if (${Debug}) /echo |- EVENT_groupUp ==>
  /if (!${checkEventArgs[${ChatSender},${line},U,${createGroup}]}) /return
  /if (${c_eventArgChatSender.Equal[${Me.Name}]}) {
    /squelch /bca //raiddisband
    /squelch /bca //disband
    /delay 5
    /if (${Raid.Members}) /raiddisband
  }
  /delay 5

  /declare groupName string local ${c_eventArgData}
  /declare groupFound string local
  /declare groupMembers[5] string local
  /declare groupSize int local
  /declare i int local 1
  /declare e int local

  | -Check for groupName in Group_Ini.
  :nextGroupListing
  /if (${${IniMode}[${Group_Ini}].Arg[${i},|].Arg[2,_].Equal[${groupName}]} && ${${IniMode}[${Group_Ini},${${IniMode}[${Group_Ini}].Arg[${i},|]},GroupMember#0].NotEqual[DEPRECATED]}) {
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

      /delay 8s ${Group} == ${groupSize}
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
 :skip
/if (${Debug}) /echo <== EVENT_groupUp -|
/RETURN

|---------------------------|
#EVENT exp_group "[#1#] ExpGroup #2#"
#EVENT exp_group "<#1#> ExpGroup #2#"
#EVENT exp_group "#1# tells the group, 'ExpGroup #2#'"
#EVENT exp_group "#1# tell your party, 'ExpGroup #2#'"
SUB EVENT_exp_group(line, ChatSender, expToggle)
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${expToggle}]}) /return

  /if (${Select[${c_eventArgData},On,Yes,True,1]} > 0) {
    /call IniToArrayV "${Character_Ini},Melee Abilities,Ability#" MyAbilities
    /if (${Defined[MyAbilities]})	/call BuildSpellArray "MyAbilities" "Abilities2D"
    /if (${Me.Class.ShortName.Equal[Rog]}) {
      /call buff_SpellArrays
      /if (${Defined[strikeDiscStr]}) {
        /varset strikeDisc ${strikeDiscStr.Arg[1,/]}
      }
    }
    /if (${Ini[${Character_Ini},Nukes,${KSSpellSet}].Length}) /call event_SpellSet "PLACEHOLDER" "${Me}" ${KSSpellSet}
    /if (${Ini[${Character_Ini},Swap Items,${KSWeaponSet}].Length}) /swap ${KSWeaponSet}
    /echo \ag KS Mode Activated - get exp

  |remove anything that could kos when out of group
  } else /if (${Select[${c_eventArgData},Off,No,False,0]} > 0) {
    /if (${Select[${Me.Class.ShortName},${Ini[${genSettings_Ini},ExpGroup,NoCombatAbil]}]}) {
      /if (${Defined[MyAbilities]}) /deletevar MyAbilities
      /if (${Defined[Abilities2D]}) /deletevar Abilities2D
    }
    /if (${Ini[${Character_Ini},Nukes,${NoKSSpellSet}].Length}) /call event_SpellSet "PLACEHOLDER" "${Me}" ${NoKSSpellSet}
    /if (${Defined[NoKSWeaponSet]} && ${Ini[${Character_Ini},Swap Items,${NoKSWeaponSet}].Length}) /swap ${NoKSWeaponSet}
    /if (${Me.Class.ShortName.Equal[Rog]}) {
      /varset strikeDisc 0
      /declare poisonInUse string local
      /declare i int local
      :remove_poisons
        /if (${Defined[${poisonPR}]}) {
          /if (${Bool[${Me.Buff[${FindItem[=${poisonPR}].Spell}]}]}) /varset poisonInUse ${poisonPR}
        }
        /if (${Defined[${poisonFR}]}) {
          /if (${Bool[${Me.Buff[${FindItem[=${poisonFR}].Spell}]}]}) /varset poisonInUse ${poisonFR}
        }
        /if (${Defined[${poisonCR}]}) {
          /if (${Bool[${Me.Buff[${FindItem[=${poisonCR}].Spell}]}]}) /varset poisonInUse ${poisonCR}
        }
        /if (${Defined[CombatBuffs2D]}) {
          /for i 1 to ${CombatBuffs2D.Size[1]}
            /if (${CombatBuffs2D[${i},${iCastName}].Find[${poisonInUse}]}) {
              /call RemoveSpellArrayElement "CombatBuffs2D" ${i}
              /goto :remove_poisons
            }
          /next i
        }
      /if (${Defined[SelfBuffs2D]}) {
        /for i 1 to ${SelfBuffs2D.Size[1]}
          /if (${SelfBuffs2D[${i},${iCastName}].Find[${poisonInUse}]}) {
            /call RemoveSpellArrayElement "SelfBuffs2D" ${i}
            /goto :remove_poisons
          }
        /next i
      }
      /squelch ${Me.Buff[${FindItem[=${poisonInUse}].Spell}].Remove}
    }
    /echo \ag NoKS Mode Activated - do not get exp
  }
/return

|---------------------------|
#EVENT reportMana "<#1#> Report mana#*#"
SUB EVENT_reportMana(line,ChatSender)
/if (${Me.Class.CanCast}) {
  /if (!${checkEventArgs[${ChatSender},${line},UZ,""]}) /return
  /if (${Me.PctMana}<100) /docommand ${ChatToggle} ${Me.PctMana}m
}
/RETURN

|---------------------------|
#EVENT campDesktop "<#1#> CampDesktop"
SUB campDesktop(line,ChatSender)
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${expToggle}]}) /return
  /camp desktop
  /endmacro
/RETURN

Sub basics_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Basics (On/Off)" Debug_Basics bool outer
/if (${Debug} || ${Debug_Basics}) /echo |- basics_Setup ==>

	/if (!${Defined[Following]}) /declare Following bool global FALSE
	/if (!${Defined[FollowTarget]}) /declare FollowTarget string global NULL
	/declare FollowCheckTimer timer outer

  /if (!${Defined[barkTarget]}) /declare barkTarget string outer
  /if (!${Defined[barkReturned]}) /declare barkReturned bool outer FALSE

	/declare event_counter int outer 0

  /declare c_checkSelectedBots  bool outer
	/declare c_eventArg           bool outer
	/declare c_eventArgChatSender string outer
  /declare c_eventArgData       string outer
  /declare c_argueString        string outer

	/declare medBreak bool outer FALSE
	/declare medBreak_Hold bool outer FALSE
	/declare medbreak_Popup_Timer timer outer
	/declare clickitRandomDelay int outer 7

	/if (${Ini[${genSettings_Ini},ExpGroup,NoKS_WeaponSet].Length}) 		/call iniToVarV "${genSettings_Ini},ExpGroup,NoKS_WeaponSet" NoKSWeaponSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,KS_WeaponSet].Length}) 			/call iniToVarV "${genSettings_Ini},ExpGroup,KS_WeaponSet" KSWeaponSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,NoKS_SpellSet].Length}) 			/call iniToVarV "${genSettings_Ini},ExpGroup,NoKS_SpellSet" NoKSSpellSet string outer
	/if (${Ini[${genSettings_Ini},ExpGroup,KS_SpellSet].Length}) 				/call iniToVarV "${genSettings_Ini},ExpGroup,KS_SpellSet" KSSpellSet string outer

	/declare MaxResponseDist int outer 250
	/if (${Ini[${genSettings_Ini},General,Max Response Distance].Length} && ${Int[${Ini[${genSettings_Ini},General,Max Response Distance]}]}) /call iniToVarV "${genSettings_Ini},General,Max Response Distance" MaxResponseDist int outer

	/declare LeashLength int outer 100
	/if (${Ini[${genSettings_Ini},General,Leash Length].Length} && ${Int[${Ini[${genSettings_Ini},General,Leash Length]}]}) /call iniToVarV "${genSettings_Ini},General,Leash Length" LeashLength int outer
	/if (${Ini[${genSettings_Ini},General,End MedBreak in Combat(On/Off)].Length}) /call iniToVarV "${genSettings_Ini},General,End MedBreak in Combat(On/Off)" medOn_combatBreak bool outer
  /if (${Ini[${genSettings_Ini},General,AutoMedBreak PctMana].Length}) /call iniToVarV "${genSettings_Ini},General,AutoMedBreak PctMana" autoMedPctMana int outer
	| -Add Groups_Ini file path
	/if (!${Ini[${MacroData_Ini},File Paths,Saved Groups].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Saved Groups" "e3 Macro Inis\Saved Groups.ini" 1
	| -Import Groups_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Saved Groups].Length}) {
		/echo ERROR: Could not find designated file path for [Saved Groups], please review [${MacroData_Ini}] and restart.
    /beep
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
  /doevents clickIt
  /doevents oldclickIt
  /doevents bark
  /doevents reportMana
  /doevents groupUp
  /doevents listGroups
  /doevents saveGroup
  /doevents Invite
  /doevents RaidAccept
  /doevents Evacuate
  /doevents gate
  /doevents ClickYes
  /doevents gatherCorpses
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
  /doevents FindItemCompact
  /doevents FindSlot
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
/if (${Debug}) /echo |- basics_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Basics (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},General,Max Response Distance" 500
	/call WriteToIni "${genSettings_Ini},General,Leash Length" 250
	/call WriteToIni "${genSettings_Ini},General,End MedBreak in Combat(On/Off)" On
  /call WriteToIni "${genSettings_Ini},General,AutoMedBreak PctMana" 0

	/call WriteToIni "${genSettings_Ini},ExpGroup,NoKS_WeaponSet" NoExp
	/call WriteToIni "${genSettings_Ini},ExpGroup,NoKS_SpellSet" NoExp
	/call WriteToIni "${genSettings_Ini},ExpGroup,KS_WeaponSet" Main
	/call WriteToIni "${genSettings_Ini},ExpGroup,KS_SpellSet" Exp

/if (${Debug}) /echo <== basics_MacroSettings -|
/RETURN

SUB basics_CharacterSettings
/RETURN

Sub basics_Aliases
/if (${Debug}) /echo |- basics_Aliases ==>
	/noparse /squelch /alias /followon /bc [+g+]Follow ${Me.CleanName}
	/noparse /squelch /alias /followme /bc [+g+]Follow ${Me.CleanName}
	/noparse /squelch /alias /followit /bc [+g+]Follow ${Target.CleanName}
  /squelch /alias /followoff /bc [+r+]Stop
	/squelch /alias /stop /bc [+r+]Stop
  /noparse /squelch /alias /stopf /bcaa //StopFollowing ${Me.Y} ${Me.X}
  /noparse /squelch /alias /mtm /bc Move to ${Me.ID}
  /noparse /squelch /alias /movetome /bc Move to ${Me.ID}
	/noparse /squelch /alias /mtt /bc Move to ${Target.ID}
  /noparse /squelch /alias /movetotarget /bc Move to ${Target.ID}
	/noparse /squelch /alias /Bark /bc Bark at ${Target.ID}
  /squelch /alias /ClickIt /bc Click It
  /noparse /squelch /alias /oClickIt /bc OldClickit ${Me.Loc.Replace[ ,]}
	/noparse /squelch /alias /RunThruZone /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
	/noparse /squelch /alias /RunThroughZone /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
	|/noparse /squelch /alias /RTZ /bc Run through zone at ${Me.Loc.Replace[ ,]},${Me.Heading.Degrees}
  /noparse /squelch /alias /RTZ /bc Run through zone
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
  /squelch /alias /FindItemC /bc Find ItemC
	/squelch /alias /FindItem /bc Find Item
  /squelch /alias /FindSlot /bc Find Slot
  /squelch /alias /fds /bc Find Slot
  /squelch /alias /fdi /bc Find Item
  /squelch /alias /fic /bc Find ItemC
  /squelch /alias /FindMissingItem /bc Find Missing Item
  /squelch /alias /fmi /bc Find Missing Item
	/noparse /squelch /alias /FindIt /bc Find Item ${Cursor}
	/squelch /alias /CountBots /echo Head Count
	/squelch /alias /Count /echo Head Count
	/squelch /alias /BotCount /echo Head Count
	/squelch /alias /RollCall /echo Head Count
	/squelch /alias /Swap /echo Swap
	/squelch /alias /useitem /useitem
	/noparse /squelch /alias /ConsentMe /bc Consent ${Me.CleanName}
	/squelch /alias /BotConsent /bc Consent
	/squelch /alias /Group /bc Group
	/squelch /alias /SaveGroup /bc Save Group
	/squelch /alias /SaveGroups /bc Save Group
	/squelch /alias /listgroups /bc List Groups
	/squelch /alias /medon /bc MedBreak On
	/squelch /alias /medoff /bc MedBreak Off
	/squelch /alias /SetAAXP /bc Set AA XP
	/squelch /alias /PortTo /bc Port to
	/squelch /alias /FixCorpses /bc Fix Corpses
	/squelch /alias /ResetCorpses /bc Fix Corpses
	/squelch /alias /gathercorpses /echo Gather Corpses
	/squelch /alias /mana /bc Report Mana
/if (${Debug}) /echo <== basics_Aliases -|
/return