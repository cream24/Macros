|------------------------------------------------------------------------|
|- e3_Macro_Background.inc	( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
|- Contains background functions that update e3 status and variables,	-|
|- along with many minimally invasive macro functions.					-|
|------------------------------------------------------------------------| 

|----------------------------------------------------------------------------|
|- Updates zone related variables, and safeZone via 'SUB check_ZoneType'.	-|
|----------------------------------------------------------------------------|
SUB check_Zone
/if (${Debug} || ${Debug_Background}) /echo |- check_Zone ==>
	| Set current zone ID
	/varset currentZone ${Zone.ID}
	| Update macro timers
	/varset idleTimer ${idleInterval}m
	| Turn off assist / attack
	/call AssistOff
  /if (${use_TargetAE} || ${use_PBAE}) /call event_AEOff "NULL" "${Me.CleanName}"
	| Load Spellset
	/if (${Me.Class.CanCast} && ${Defined[Default_SpellSet]}) /call check_Gems
	| Reset ignored spell list
	/if (${Defined[failureList]}) /varset failureList
	| Check tribute settings for zone
	/call check_ZoneType
	/if (${Defined[tributeOverride]}) /deletevar tributeOverride
	| Check Auto-Melody
  /if (${Defined[autoMelody]} && !${songSet.Length}) /call EVENT_startMelody "<${Me}> melody ${autoMelody}" "${Me}" "${autoMelody}"

	/if (${Me.Pet.ID}) {
		/squelch /pet follow
		/squelch /pet taunt off
		/squelch /pet ghold on
	}
/if (${Debug} || ${Debug_Background}) /echo <== check_Zone -|
/RETURN

|--------------------------------------------------------------------------------------------------------------------|
|- Attempts to load the spellset specified in your Macro_Ini, if you're missing more than 3 gems on your spellbar.  -|
|--------------------------------------------------------------------------------------------------------------------|
SUB check_Gems
/if (${Debug} || ${Debug_Background}) /echo |- check_Gems ==>
	| Count Empty Spell Gems
	/declare i int local
	/declare spellCount int local
	/for i 1 to 9
		/if (${Select[${Me.Gem[${i}]},NULL]}) /varcalc spellCount ${spellCount}+1
	/next i
	| If more than 3 are empty
	/if (${spellCount} > 3) {
		/declare spellsetMem_Timer timer local 300
		/docommand ${ChatToggle} Loading spells...
		/memspellset ${Default_SpellSet}
		:Memloop
		/if (${Debug} || ${Debug_Background}) /echo |- check_Gems -| :Memloop
		
		/call Background_Events
		/if (${Debug} || ${Debug_Background}) /echo spellsetMem_Timer = [${spellsetMem_Timer}]
		/if (${Window[SpellBookWnd].Open} && ${spellsetMem_Timer}) /goto :Memloop
		/if (${Me.Sitting}) /stand	
	}
/if (${Debug} || ${Debug_Background}) /echo <== check_Gems -|
/RETURN

|--------------------------------------------------------------------------------------------|
|- Reports bots who have fallen behind, and attempts to re-aquire follow when it breaks.	-|
|--------------------------------------------------------------------------------------------|
SUB check_Follow
/if (${Debug} || ${Debug_Background}) /echo |- check_Follow ==>
	/if (${FollowTarget.Equal[${Me.CleanName}]}) {
		/if (!${spamTimer}) {
			/declare i int local 1
			:NextFollower
				/if (!${SpamTimer_${Followers.Arg[${i},,]}}) {
					/if (${SpawnCount[pc ${Followers.Arg[${i},,]}]} && !${Spawn[pc ${Followers.Arg[${i},,]} radius ${LeashLength}].ID}) {
						/popup ${Followers.Arg[${i},,]} has fallen behind.
						/call CreateTimer SpamTimer_${Followers.Arg[${i},,]} 100
						/varset spamTimer 10
					}
				}
			/varcalc i ${i} + 1
			/if (${Followers.Arg[${i},,].Length}) /goto :NextFollower
		}
	} else {
		/if (${SpawnCount[pc ${FollowTarget}]} && ${Spawn[pc ${FollowTarget}].Distance} < ${LeashLength} && ${Spawn[pc ${FollowTarget}].LineOfSight}) {
			/if (${NetAdvPath.Status}==0 && !${Stick.Active}) {
			  /if (${Spawn[pc ${FollowTarget}].Distance}< 40 && ${medBreak}) {
          /if (${Debug} || ${Debug_Background}) /echo not following during medbreak and < 40 dist
        } else {
          /call AquireFollow
        }
			}
			/if (${Me.FeetWet}) /squelch /face fast ${FollowTarget}
		}
	}
/if (${Debug} || ${Debug_Background}) /echo <== check_Follow -|
/RETURN

|--------------------------------------------------------------------------------------------------------------------|
|- Checks to see if your character is idle.																			-|
|--------------------------------------------------------------------------------------------------------------------|
|- Updates idleTimer to the idleInterval set in your Macro_Ini, whenever you've recently been active, or in combat.	-|
|- Sets idle=TRUE if you're in a safeZone, or when you have been inactive for longer than the idleInterval.			-|
|--------------------------------------------------------------------------------------------------------------------|
SUB check_Idle
  /if (${Debug} || ${Debug_Background}) /echo |- check_Idle ==>
| Check to see if I have been active recently.  If so, set Idle false, and update the idleTimer
  /if (${activeTimer} || ${Assisting}) {
    /varset idleTimer ${idleInterval}m
    /if (${idle}) /varset idle FALSE
  } else {
    | If I haven't been active, and the idleTimer is spent, set idle TRUE.
    /if (!${idle} && !${idleTimer}) /varset idle TRUE
  }
  /if (${Debug}) /echo <== check_Idle -| idleTimer = [${idleTimer}] || idle = [${idle}]
/RETURN

|--------------------------------------------------------------------------------------------|
|- Checks to see if your character is active, if so pauses macro loop                       -|
|--------------------------------------------------------------------------------------------|			
SUB check_Active
/if (${Debug} || ${Debug_Background}) /echo |- check_Active ==>
	| Check Movement, casting

  /if (${Me.Class.ShortName.NotEqual[BRD]} && ${Me.Casting.ID}) /varset activeTimer 5
	| Check Cursor
	/if (${Cursor.ID}) {	
		| Destroy badItems.
		/if (${Defined[badItems]}) {
			/if (${Select[${Cursor},${badItems}]} || ${Select[${Cursor.ID},${badItems}]}) {
				/echo Destroying ${Cursor}...
				/destroy
			}
		}
		| Check to see if I'm waiting to drop this item, or if I've picked up something new
		/if (${Old_cursorID} != ${Cursor.ID}) {
			/varset cursorTimer ${Math.Calc[${inventoryTimer} * 10].Int}
			/varset Old_cursorID ${Cursor.ID}
		} else {
			/if (!${cursorTimer}) /call clearCursor
		}		
		/varset activeTimer 5
	} else {
		/if (${Old_cursorID}) {
			/varset cursorTimer 0
			/varset Old_cursorID 0
		}
	}
  /if (${Me.Moving} || ${Corpse.Open}) {
    /if (${Me.Class.ShortName.Equal[War]}) {
      /varset activeTimer 5
    } else {
      /varset activeTimer 10
    }
  }
	| Check AutoAttack
	/if (${Me.Combat}) {
		/if (!${Target.ID} || ${Target.Type.Equal[corpse]} && !${Assisting} || (${Spawn[${Target.ID}].Type.Equal[PC]} && !${Toggle-PvP})) /attack off
	}
  | If windows are open
	/if (${Window[BigBankWnd].Open} || ${Window[MerchantWnd].Open} || ${Window[TradeWnd].Open} || ${Window[GuildBankWnd].Open}) {
		/varset activeTimer 30
		/doevents flush
	}
	| Check spellbook
	/if (${Window[SpellbookWnd].Open}) {
		/if (!${spellbook_Timer}) {
			| if the timer isn't defined, make one.
			/if (!${Defined[spellbook_Timer]}) {
				/declare spellbook_Timer timer outer ${spellbookTimer}s
			} else {
				/if (${Window[SpellbookWnd].Open}) /stand
			}
		}
		/varset activeTimer 30
	} else {
		/if (${Defined[spellbook_Timer]}) /deletevar spellbook_Timer
	}
	| Check Feigning
	/if (${Me.Feigning}) {
		/if (!${Select[${Me.Class.ShortName},NEC,SHD,MNK]}) {
			/stand
			/if (${Assisting} && ${StickOn}) /attack on
		} else {
			/if (!${Defined[feign_Timer]}) {
				/docommand ${ChatToggle} I AM FEIGNED.
				/declare feign_Timer timer outer 15s
			}
		}
		/varset activeTimer 30
	} else {
		/if (${Defined[feign_Timer]}) /deletevar feign_Timer
	}

| Check Zoning
  /if (!${SpawnCount[pc ${Me}]}) /varset activeTimer 5s
/if (${Debug} || ${Debug_Background}) /echo <== check_Active -|
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Completes item casting exchanges which were interrupted by macro end/crash/etc
SUB completePendingExchange
/if (${Debug} || ${Debug_Background}) /echo |- completePendingExchanges ==>
	| Check for the item needing to be swapped back.
	/if (${Zone.ID}==306 ) /return
	/declare itemToSwapBack string local
  /declare swapItemSlot string local
  /declare pendingslot int local 0
  |/echo ${itemToSwapBack} ${itemToSwapBack.Length}  ${swapItemSlot} ${swapItemSlot.Length}
  /varset itemToSwapBack ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange].Arg[1,/]}
  /varset swapItemSlot ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange].Arg[2,/]}
  /varset pendingslot 0
  |/echo 0 ${itemToSwapBack} ${swapItemSlot}
  /if (!${Bool[${itemToSwapBack}]} || !${Bool[${swapItemSlot}]}) {
    /varset itemToSwapBack ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1].Arg[1,/]}
    /varset swapItemSlot ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1].Arg[2,/]}
		/varset pendingslot 1
		|/echo 1 ${itemToSwapBack} ${swapItemSlot}
		/if (!${Bool[${itemToSwapBack}]} || !${Bool[${swapItemSlot}]}) {
      /varset itemToSwapBack ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2].Arg[1,/]}
      /varset swapItemSlot ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2].Arg[2,/]}
			/varset pendingslot 2
			|/echo 2 ${itemToSwapBack} ${swapItemSlot}
    }
  }

  /if (${Me.Inventory[${swapItemSlot}].Name.Equal[${itemToSwapBack}]})  {
  	|/echo if1
    /if (${itemToSwapBack.NotEqual[NONE]}) {
			/if (${pendingslot}==0)	{
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" NONE 1
			} else /if (${pendingslot}==1) {
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1" NONE 1
			} else /if (${pendingslot}==2) {
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2" NONE 1
			}
    }
  } else /if (!${FindItemCount[=${itemToSwapBack}]}) {
		|/echo if2
    /if (${Debug} || ${Debug_Background}) {
			/if (${itemToSwapBack.NotEqual[NONE]}) {
				/if (${pendingslot}==0)	{
					/echo Could not find [${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange].Arg[1,/]}], skipping swapback.
				} else /if (${pendingslot}==1) {
					/echo Could not find [${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1].Arg[1,/]}], skipping swapback.
				} else /if (${pendingslot}==2) {
					/echo Could not find [${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2].Arg[1,/]}], skipping swapback.
				}
			}
    }
  } else {
		|/echo if3
		| Set a retry timer in case the first attempt fails
		/declare swapRetryTimer timer local 30
		:SwapBackLoop
		/if (${Debug} || ${Debug_Background}) /echo :SwapBackLoop
		| Try to swap the item
		|/echo ${itemToSwapBack} ${swapItemSlot}
		/call SwapItem "${itemToSwapBack}" "${swapItemSlot}"
		| If the if item was not returned to it's correct slot
		/if (!${Me.Inventory[${swapItemSlot}].Name.Equal[${itemToSwapBack}]}) {
			| If I have time to try again, goto SwapBackLoop.  If not, announce to user.
			/if (${retryTimer}) {
				/goto :SwapBackLoop
			} else {
				/bc I have failed to swap ${itemToSwapBack} back to slot ${swapItemSlot}.
			}
		} else {
			| If the item was returned to it's correct slot
			/if (${pendingslot}==0)	{
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" NONE 1
			} else /if (${pendingslot}==1) {
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1" NONE 1
			} else /if (${pendingslot}==2) {
				/call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2" NONE 1
			}
		}
	}
/if (${Debug} || ${Debug_Background}) /echo <== completePendingExchanges -|
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Updates safeZones_Ini entries to 'safe' or 'raid' via /bc Set Raid Zone, or /bc Set Safe Zone.	-|
|----------------------------------------------------------------------------------------------------|
SUB check_autoTribute
/if (${Debug}) 	/echo |- check_autoTribute -| Idle = ${idle} -- SafeZone = ${safeZone}
	| If idle is TRUE, and my remaining TributeTimer is < 10, turn off Tribute.
	/if (!${tributeZone} && ${Me.TributeActive}) {
	  /if (${Me.TributeTimer} <= 10 && ${idle}) {
      /echo AutoTribute - I'm idle.  Turning tribute off.
      /call toggleTribute
    }
  } else /if (${tributeZone} && !${Me.TributeActive}) {
    /if (${Me.CurrentFavor} > 500) {
      /echo |- AutoTribute -| I've entered a tribute zone, turning tribute on.
      /call toggleTribute
    }
  }
/if (${Debug}) /echo <== check_autoTribute -|
/RETURN

|----------------------------------------------------------------------------------------------------|
SUB toggleTribute
/if (${Debug}) /echo |- toggleTribute ==>
  /keypress TOGGLE_TRIBUTEBENEFITWIN
  /notify TBW_PersonalPage TBWP_ActivateButton leftmouseup
  /keypress TOGGLE_TRIBUTEBENEFITWIN
  /if (${Me.TributeActive} && !${HUD_tributeTimer}) /varset HUD_tributeTimer ${HUD_tributeTimerFlashInterval}
/if (${Debug}) /echo <== toggleTribute -|			
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Turn Tribute on
|----------------------------------------------------------------------------------------------------|
#EVENT manualTributeSet "[#1#] Tribute #2#"
#EVENT manualTributeSet "<#1#> Tribute #2#"
SUB EVENT_manualTributeSet(line, ChatSender, eventParams)
/if (${Debug}) /echo |- EVENT_manualTributeSet ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		/declare tributeToggle string local ${eventParams.Arg[1,|]}
		/declare tributeTimerSetting timer local ${eventParams.Arg[2,|]}
		/varset eventParams ${eventParams.Arg[1, ]}
		| set toggle based on keywords.  If param2 does not match any of the keywords, the command is skipped.
		/if (${tributeToggle.Equal[on]} || ${tributeToggle.Equal[true]} || ${tributeToggle.Equal[start]} || ${tributeToggle.Equal[activate]}) {
			/varset tributeZone TRUE
			/if (!${tributeTimerSetting}) {
				/echo Turning tribute [ON]
			} else {
				/echo Turning tribute [ON] for [${eventParams.Arg[2,|]}].
				/docommand /timed ${tributeTimerSetting} /varset tributeZone FALSE
				/docommand /timed ${tributeTimerSetting} /echo Tribute Timer is up.  Deactivating tribute on the next upkeep.
			}
		} else /if (${tributeToggle.Equal[off]} || ${tributeToggle.Equal[false]} || ${tributeToggle.Equal[end]} || ${tributeToggle.Equal[deactivate]}) {
			/varset tributeZone FALSE
			/if (!${tributeTimerSetting}) {
				/echo Turning tribute [OFF] when 10 ticks left on tribute
			} else {
				/echo Turning tribute [OFF] for [${eventParams.Arg[2,|]}].
				/docommand /timed ${tributeTimerSetting} /varset tributeZone FALSE
				/docommand /timed ${tributeTimerSetting} /echo Tribute Timer is up.  Activating tribute on the next upkeep.
			}
		}
	}

/if (${Debug}) /echo <== EVENT_manualTributeSet -|
/RETURN

|----------------------------------------------------------------------------------------|
|- Reads safeZones_Ini for zone settings and updates safeZones variable. 				-|
|----------------------------------------------------------------------------------------|
|- When new zones are entered, they are added to the safeZones_Ini, and set to 'safe'.	-|
|----------------------------------------------------------------------------------------|
SUB check_ZoneType
/if (${Debug}) /echo |- check_ZoneType ==>
	| If a safeZones_Ini entry for this zone does not exist, create one, and default to 'safe'.
	/if (!${Ini[${safeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}].Length}) {
		/call WriteToIni "${safeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" "Off"
		/varset tributeZone FALSE
	} else {
		/call IniToVar "${safeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" UNDEFINED bool local
		/if (${Bool[${Macro.Return}]}) {
			/varset tributeZone TRUE
		} else {
			/varset tributeZone FALSE
		}
	}
/if (${Debug}) /echo <== check_ZoneType -|
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Updates safeZones_Ini entries to 'safe' or 'raid' via /bc Set Raid Zone, or /bc Set Safe Zone.	-|
|----------------------------------------------------------------------------------------------------|
|- @param line: <'Name'> Set 'Setting' Zone.														-|
|- @param zoneSetting: Holds 'Raid' or 'Safe' zone settings.										-|
|- @param charSender: The PC setting the zone type.													-|
|----------------------------------------------------------------------------------------------------|
#event Set_zoneType "<#1#> Set tribute zone #2#"
#event Set_zoneType "[#1#] Set tribute zone #2#"
SUB event_Set_zoneType(line, chatSender, zoneSetting)
/if (${SpawnCount[pc ${chatSender}]} || !${Defined[chatSender]}) {

	/if (${zoneSetting.Equal[off]}  ) {
		/echo ${Zone} has been set as a safe zone.
		/call WriteToIni "${safeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" Off 1
	} else /if (${zoneSetting.Equal[on]}) {
		/echo ${Zone} has been listed as a raid zone.
		/call WriteToIni "${safeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" On 1
	}
	/call check_Zone
}
/RETURN

|----------------------------------------------------------------------------------------------------|
Sub background_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Background (On/Off)" Debug_Background bool outer
/if (${Debug} || ${Debug_Background}) /echo |- background_Setup ==>

	| Create outer variables.
	/if (!${Defined[idle]}) /declare idle bool outer false
	/if (!${Defined[idleTimer]}) /declare idleTimer timer outer
	
	/declare ActionTaken bool outer FALSE
	/declare spamTimer timer outer
	
	/declare activeTimer timer outer
	
	/declare combatTimer timer outer
	/declare combatDelay int outer 100
	
	/declare cursorTimer timer outer
	/declare Old_cursorID int outer 0
	
	/declare rebuffTimer timer outer
	/declare rebuffDelay int outer 100
	
	/if (!${Defined[tributeZone]}) /declare tributeZone string global
	/declare HUD_tributeTimer timer outer
	/declare HUD_tributeTimerFlashInterval int outer 50
	/declare currentZone int outer 0
	
	| Add Tribute Settings File Path
	/if (!${Ini[${MacroData_Ini},File Paths,Tribute Settings].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Tribute Settings" "e3 Macro Inis\Tribute Settings.ini"
	| Import itemTimers_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Tribute Settings].Length}) {
		/echo ERROR: Could not find designated file path for [Tribute Settings], please review [${MacroData_Ini}] and restart.
		/endmacro
	} else {
		/declare safeZones_Ini string outer ${Ini[${MacroData_Ini},File Paths,Tribute Settings]}
	}
	| Build a SafeZones.ini.
	/if (!${Ini[${safeZones_Ini},A].Length}) {
		/echo Creating Tribute Settings file...
		/call Build_Alphabetized_Ini "${safeZones_Ini}"
	}
	| Clear Cursor Timer
	/if (${Ini[${genSettings_Ini},Background,Auto-Inventory Timer (Sec)].Length}) {
		/call IniToVar "${genSettings_Ini},Background,Auto-Inventory Timer (Sec)" inventoryTimer string outer
	} else {
		/echo Warning: Could not find a setting for [Auto-Inventory Timer] in [${genSettings_Ini}].  [Auto-Inventory Timer] has been set to 30 seconds.
		/declare inventoryTimer int outer 30
	}
	| Close Spellbook Timer
	/if (${Ini[${genSettings_Ini},Background,Close Spellbook Timer (Sec)].Length}) {
		/call IniToVar "${genSettings_Ini},Background,Close Spellbook Timer (Sec)" spellbookTimer string outer
	} else {
		/echo Warning: Could not find a setting for [Close Spellbook Timer] in [${genSettings_Ini}].  [Close Spellbook Timer] has been set to 1 minute.
		/declare spellbookTimer int outer 60
	}
	| Idle Timer
	/if (${Ini[${genSettings_Ini},Background,Idle Time Out (Min)].Length}) {
		/call IniToVar "${genSettings_Ini},Background,Idle Time Out (Min)" idleInterval int outer
	} else {
		/echo Warning: Could not find a setting for [Idle Time Out] in [${genSettings_Ini}].  [Idel Time Out] has been set to 10 minutes.
		/declare idleInterval int outer 10
	}
	| Auto-Destroy Items
	/if (${Ini[${genSettings_Ini},Background,Auto-Destroy List].Length}) /call IniToVar "${genSettings_Ini},Background,Auto-Destroy List" badItems string outer
	| Auto-Tribute
	/call IniToVar "${genSettings_Ini},Auto-Tribute,Auto-Tribute (On/Off)" autoTribute bool outer
	| Import NetBots HUD array
	/call #IniToArray "${advSettings_Ini},NetBots HUD,NetBot#" netBotsHUDArray
/if (${Debug} || ${Debug_Background}) /echo <== background_Setup -|
/return

|----------------------------------------------------------------------------------------------------|
SUB background_MacroSettings
/if (${Debug} || ${Debug_Background}) /echo |- background_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},Debug,Debug Background (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Background,Auto-Destroy List"
	/call WriteToIni "${genSettings_Ini},Background,Idle Time Out (Min)" 10
	/call WriteToIni "${genSettings_Ini},Background,Auto-Inventory Timer (Sec)" 30
	/call WriteToIni "${genSettings_Ini},Background,Close Spellbook Timer (Sec)" 30	
	/call WriteToIni "${genSettings_Ini},Auto-Tribute,Auto-Tribute (On/Off)" Off
/RETURN

|----------------------------------------------------------------------------------------------------|
SUB background_CharacterSettings
/RETURN

|----------------------------------------------------------------------------------------------------|
Sub background_Aliases
	/noparse /squelch /alias /AddAutoDestroy /echo Add Auto-Destro item ${Cursor.Name}
	/noparse /squelch /alias /RemoveAutoDestroy /echo Remove Auto-Destro item ${Cursor.Name}
	/squelch /alias /tributezone /bc Set Tribute Zone
	/noparse /squelch /alias /reporttrib /noparse /bcaa //gu ${Me.CurrentFavor}
	/noparse /squelch /alias /reportAA /noparse /bcaa //gu ${Me.AAPoints}

/return

|----------------------------------------------------------------------------------------------------|
Sub background_Background_Events
  /doevents manualTributeSet
  /doevents Set_zoneType
	/doevents updateAliases
/return