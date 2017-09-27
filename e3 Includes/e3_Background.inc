|------------------------------------------------------------------------|
|- Contains background functions that update e3 status and variables,	  -|
|- along with many minimally invasive macro functions.					        -|
|------------------------------------------------------------------------| 

SUB check_Zone
/if (${Debug} || ${Debug_Background}) /echo |- check_Zone ==>
  /if (${Zone.ID}==190 || ${MacroQuest.GameState.Equal[CHARSELECT]}) /endmacro
	/varset currentZone ${Zone.ID}
	/varset idleTimer ${idleInterval}m
  /if (${Assisting}) /call AssistOff
  /if (${use_TargetAE} || ${use_PBAE}) /call Event_AEs "NULL" "${Me.CleanName}" "All" "Off"
	/if (${Me.Class.CanCast} && ${Defined[Default_SpellSet]}) /call check_Gems
	/if (${Defined[failureList]}) /varset failureList

	/if (${Me.Pet.ID}) {
		/squelch /pet follow
		/squelch /pet taunt off
		/squelch /pet ghold on
	}
	/call check_tribZone
  |reload cure arrays, supports only checking cures per zone
	/if (${Macro.RunTime} > 15 && ${Select[${Me.Class.ShortName},DRU,SHM,CLR,PAL,BST,RNG]}) /call cure_spellArrays
  /if (${Defined[autoMelody]}) {
    /if (!${songSet.Length}) /call EVENT_startMelody "<${Me}> melody ${autoMelody}" "${Me}" "${autoMelody}"
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
	/if (${FollowTarget.NotEqual[${Me}]}) {
	  /if (${SpawnCount[=${FollowTarget}]}) {
      /declare followTgtDistance float local ${Spawn[=${FollowTarget}].Distance}
      /declare followTgtLoS bool local ${Spawn[=${FollowTarget}].LineOfSight}
      /if (${followTgtDistance} <= ${LeashLength}) {
        /if (!${NetAdvPath.Status} && !${Stick.Active}) {
          /if (${followTgtDistance}< 40 && ${medBreak}) {
            /if (${Debug} || ${Debug_Background}) /echo not following during medbreak and < 40 dist
          } else {
            /call AcquireFollow
          }
        }
        /if (${Me.FeetWet}) /squelch /face fast ${FollowTarget}
        |FollowCheckTimer
      } else /if (${followTgtDistance} > ${LeashLength}) {
        /if (!${FollowCheckTimer}) {
          /if (!${NetAdvPath.Status} && !${Stick.Active}) {
            /bct ${FollowTarget} //popup ${Me} stopped moving! Dist:${followTgtDistance},LoS:${followTgtLoS}
          } else {
            /bct ${FollowTarget} //popup ${Me} is falling behind. Dist:${followTgtDistance},LoS:${followTgtLoS}
          }
          /varset FollowCheckTimer 60+${Math.Rand[${NetBots.Counts}]}
        }
      }
	  }
	}
/if (${Debug} || ${Debug_Background}) /echo <== check_Follow -|
/RETURN

|--------------------------------------------------------------------------------------------------------------------|
|- Checks to see if your character is idle.																			-|
|--------------------------------------------------------------------------------------------------------------------|
|- Updates idleTimer to the idleInterval set in your Macro_Ini, whenever you've recently been active, or in combat.	-|
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

  |prevent guys from getting stuck trying to buff while moving
	/if (${Me.Moving}) {
	  /if (${Me.CombatState.Equal[COMBAT]}) {
      /varset activeTimer 2
      /varset rebuffTimer 10
	  }  else {
      /varset activeTimer 6
      /varset rebuffTimer 25
	  }
  }

  /if (${Corpse.Open}) /varset activeTimer 2s
  /if (${Me.Ducking}) /varset activeTimer 2s

  | If windows are open
	/if (${Window[BigBankWnd].Open} || ${Window[MerchantWnd].Open} || ${Window[TradeWnd].Open} || ${Window[GuildBankWnd].Open}) {
		/varset activeTimer 3s
		/doevents flush
	}

	| Check spellbook
	/if (${Window[SpellbookWnd].Open}) {
    /if (!${Defined[spellbook_Timer]}) {
      /declare spellbook_Timer timer outer ${spellbookTimer}s
		} else {
      /if (!${spellbook_Timer}) {
        /if (${Window[SpellbookWnd].Open}) /stand
        /deletevar spellbook_Timer
      }
		}
		/varset activeTimer 3s
	} else {
		/if (${Defined[spellbook_Timer]}) /deletevar spellbook_Timer
	}
| Check Zoning
  /if (!${SpawnCount[pc ${Me}]}) /varset activeTimer 5s
/if (${Debug} || ${Debug_Background}) /echo <== check_Active -|
/RETURN

|------------------------------------------------------------|
|- loot, check exp allocation, announce exp	-|
#event YourKill "You gain experience!!"
#event YourKill "You gain party experience!!"
#event YourKill "You gained raid experience!"
SUB Event_YourKill(line)
  /if (!${line.Find[raid]}) {
    /if (${Group}>1 && ${Me.Name.Equal[${Group.Leader}]}) {
      /bc [+g+] My group got Exp
    } else /if (!${Group}) {
      /bc [+y+] I got solo Exp
    }
  }
  /if (${autoSetPctAAExp}) {
    /if ((${Me.Level}<70 || ${Me.PctExp}<=25) && ${Window[AAWindow].Child[AAW_PercentCount].Text.Left[-1]}>0) {
      /squelch /alternateadv on 0
      /bc [+y+] Auto setting AA Exp to 0%, at ${Me.PctExp}% of Level ${Me.Level}
    } else /if (${Me.Level}==70 && ${Me.PctExp}>=50 && ${Window[AAWindow].Child[AAW_PercentCount].Text.Left[-1]}<100) {
      /squelch /alternateadv on 100
      /bc [+y+] Auto setting AA Exp to 100%, at ${Me.PctExp}% of Level ${Me.Level}
    }
  }
  /if (${Auto_Loot} && (${Me.CombatState.NotEqual[Combat]} || ${combatLooting})) /call loot_Area
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Completes item casting exchanges which were interrupted by macro end/crash/etc
SUB completePendingExchange
/if (${Debug} || ${Debug_Background}) /echo |- completePendingExchanges ==>
	| Check for the item needing to be swapped back.
  |/echo ${itemToSwapBack} ${itemToSwapBack.Length}  ${swapItemSlot} ${swapItemSlot.Length}
  |${${IniMode}[${MacroData_Ini},${Me.CleanName}-PEQTGC,Pending Exchange]}
  /declare itemToSwapBack string local ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange].Arg[1,/]}
  /declare swapItemSlot string local ${Ini[${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange].Arg[2,/]}

  /if (${itemToSwapBack.NotEqual[NONE]} && ${itemToSwapBack.NotEqual[NULL]}) {
    /if (${Me.Inventory[${swapItemSlot}].Name.Equal[${itemToSwapBack}]}) {
      /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" NONE 1
    } else /if (!${FindItemCount[=${itemToSwapBack}]}) {
      /echo Could not find [${itemToSwapBack}], skipping swapback.
    } else {
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
        /if (${swapRetryTimer}) {
          /goto :SwapBackLoop
        } else {
          /bc I have failed to swap ${itemToSwapBack} back to slot ${swapItemSlot}.
        }
      } else {
        | If the item was returned to it's correct slot
          /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" NONE 1
      }
    }
	}
/if (${Debug} || ${Debug_Background}) /echo <== completePendingExchanges -|
/RETURN

|----------------------------------------------------------------------------------------------------|
SUB check_autoTribute
/if (${Debug}) 	/echo |- check_autoTribute -| Idle = ${idle}
	| If idle is TRUE, and my remaining TributeTimer is < 10, turn off Tribute.
	/if (!${tributeZone} && ${Me.TributeActive}) {
      /echo AutoTribute - Not a tribute zone.  Turning tribute off.
      /call toggleTribute
  } else /if (${tributeZone} && !${Me.TributeActive}) {
    /if (${Me.CurrentFavor} > 500) {
      /echo |- AutoTribute -| I've entered a tribute zone, turning tribute on.
      /call toggleTribute
    }
  } else /if (${tributeZone} && ${Me.TributeActive}) {
    /if (${Me.TributeTimer} <= 10 && ${idle}) {
      /echo AutoTribute - I'm idle. Turning tribute off.
      /call toggleTribute
    }
  }
/if (${Debug}) /echo <== check_autoTribute -|
/RETURN

|----------------------------------------------------------------------------------------|
SUB check_tribZone
  /if (${Debug}) /echo |- check_tribZone ==>
| If Tribute Settings.ini entry for this zone does not exist, create one, and default to 'off'.
  /if (!${Ini[${tributeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}].Length}) {
    /call WriteToIni "${tributeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" "Off"
    /varset tributeZone FALSE
  } else {
    /if (${Ini[${tributeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}].Equal[On]}) {
      /varset tributeZone TRUE
    } else {
      /varset tributeZone FALSE
    }
  }
  /if (${Debug}) /echo <== check_tribZone -|
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Updates Tribute Settings.ini for the current zone
|----------------------------------------------------------------------------------------------------|
#event Set_tribZone "<#1#> Set tribute zone #2#"
#event Set_tribZone "[#1#] Set tribute zone #2#"
SUB event_Set_tribZone(line, chatSender, zoneSetting)
  /if (${SpawnCount[pc ${chatSender}]} || !${Defined[chatSender]}) {
    /if (${zoneSetting.Equal[off]}  ) {
      /echo Tribute is turned [Off] for: ${Zone}
      /call WriteToIni "${tributeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" Off 1
    } else /if (${zoneSetting.Equal[on]}) {
      /echo Tribute is turned [On] for: ${Zone}
      /call WriteToIni "${tributeZones_Ini},${Zone.ShortName.Left[1]},${Zone.ShortName}" On 1
    }
    /call check_Zone
  }
/RETURN

|----------------------------------------------------------------------------------------------------|
SUB toggleTribute
/if (${Debug}) /echo |- toggleTribute ==>
  /keypress TOGGLE_TRIBUTEBENEFITWIN
  /notify TBW_PersonalPage TBWP_ActivateButton leftmouseup
  /keypress TOGGLE_TRIBUTEBENEFITWIN
/if (${Debug}) /echo <== toggleTribute -|
/RETURN

|----------------------------------------------------------------------------------------------------|
|- Turn Tribute on/off
|----------------------------------------------------------------------------------------------------|
#EVENT manualTributeSet "[#1#] Tribute #2#"
#EVENT manualTributeSet "<#1#> Tribute #2#"
SUB EVENT_manualTributeSet(line, ChatSender, eventParams)
/if (${Debug}) /echo |- EVENT_manualTributeSet ==>
  /if (!${checkEventArgs[${ChatSender},${line},UZ,${eventParams}]}) /return

  /declare tributeToggle string local ${c_eventArgData.Arg[1,|]}
  /declare tributeTimerSetting timer local ${c_eventArgData.Arg[2,|]}
  /varset eventParams ${c_eventArgData.Arg[1, ]}
  | set toggle based on keywords.  If param2 does not match any of the keywords, the command is skipped.
  /if (${tributeToggle.Equal[on]} || ${tributeToggle.Equal[true]} || ${tributeToggle.Equal[start]} || ${tributeToggle.Equal[activate]}) {
    /varset tributeZone TRUE
    /if (!${tributeTimerSetting}) {
      /echo Turning tribute [ON]
    } else {
      /echo Turning tribute [ON] for [${tributeTimerSetting}].
      /docommand /timed ${tributeTimerSetting} /varset tributeZone FALSE
      /docommand /timed ${tributeTimerSetting} /echo Tribute Timer is up.  Deactivating tribute on the next upkeep.
    }
  } else /if (${tributeToggle.Equal[off]} || ${tributeToggle.Equal[false]} || ${tributeToggle.Equal[end]} || ${tributeToggle.Equal[deactivate]}) {
    /varset tributeZone FALSE
    /if (!${tributeTimerSetting}) {
      /echo Turning tribute [OFF] when 10 ticks left on tribute
    } else {
      /echo Turning tribute [OFF] for [${tributeTimerSetting}].
      /docommand /timed ${tributeTimerSetting} /varset tributeZone FALSE
      /docommand /timed ${tributeTimerSetting} /echo Tribute Timer is up.  Activating tribute on the next upkeep.
    }
  }
/if (${Debug}) /echo <== EVENT_manualTributeSet -|
/RETURN

|----------------------------------------------------------------------------------------------------|
Sub background_Setup
/call iniToVarV "${advSettings_Ini},Debug,Debug Background (On/Off)" Debug_Background bool outer
/if (${Debug} || ${Debug_Background}) /echo |- background_Setup ==>
	/declare idle bool outer false
	/declare idleTimer timer outer
	/declare ActionTaken bool outer FALSE
	/declare spamTimer timer outer
	/declare activeTimer timer outer
	/declare combatTimer timer outer
	/declare combatDelay int outer 3s
	/declare cursorTimer timer outer
	/declare Old_cursorID int outer 0
	/declare rebuffTimer timer outer
	/declare rebuffDelay int outer 3s
	
	/if (!${Defined[tributeZone]}) /declare tributeZone string global FALSE
  /if (!${Ini[${MacroData_Ini},File Paths,Tribute Settings].Length}) {
    /echo ERROR: Could not find designated file path for [Tribute Settings], please review [${MacroData_Ini}] and restart.
    /beep
    /endmacro
  } else {
    /declare tributeZones_Ini string outer ${Ini[${MacroData_Ini},File Paths,Tribute Settings]}
  }

  /if (!${Ini[${tributeZones_Ini},A].Length}) {
    /echo Creating Tribute Settings file...
    /call Build_Alphabetized_Ini "${tributeZones_Ini}"
	}

	/declare currentZone int outer 0

  /if (!${Defined[genSettings_Ini]}) /declare genSettings_Ini string outer ${${IniMode}[${MacroData_Ini},File Paths,General Settings]}
	
	| Add Tribute Settings File Path
	/if (!${Ini[${MacroData_Ini},File Paths,Tribute Settings].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Tribute Settings" "e3 Macro Inis\Tribute Settings.ini"
	| Import itemTimers_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Tribute Settings].Length}) {
		/echo ERROR: Could not find designated file path for [Tribute Settings], please review [${MacroData_Ini}] and restart.
    /beep
		/endmacro
	}
	| Clear Cursor Timer
	/if (${Ini[${genSettings_Ini},Background,Auto-Inventory Timer (Sec)].Length}) {
		/call iniToVarV "${genSettings_Ini},Background,Auto-Inventory Timer (Sec)" inventoryTimer string outer
	} else {
		/echo Warning: Could not find a setting for [Auto-Inventory Timer] in [${genSettings_Ini}].  [Auto-Inventory Timer] has been set to 30 seconds.
		/declare inventoryTimer int outer 30
	}
	| Close Spellbook Timer
	/if (${Ini[${genSettings_Ini},Background,Close Spellbook Timer (Sec)].Length}) {
		/call iniToVarV "${genSettings_Ini},Background,Close Spellbook Timer (Sec)" spellbookTimer string outer
	} else {
		/echo Warning: Could not find a setting for [Close Spellbook Timer] in [${genSettings_Ini}].  [Close Spellbook Timer] has been set to 1 minute.
		/declare spellbookTimer int outer 60
	}
	| Idle Timer
	/if (${Ini[${genSettings_Ini},Background,Idle Time Out (Min)].Length}) {
		/call iniToVarV "${genSettings_Ini},Background,Idle Time Out (Min)" idleInterval int outer
	} else {
		/echo Warning: Could not find a setting for [Idle Time Out] in [${genSettings_Ini}].  [Idel Time Out] has been set to 10 minutes.
		/declare idleInterval int outer 10
	}
	| Auto-Destroy Items
	/if (${Ini[${genSettings_Ini},Background,Auto-Destroy List].Length}) /call iniToVarV "${genSettings_Ini},Background,Auto-Destroy List" badItems string outer
	| Auto-Tribute
	/call iniToVarV "${genSettings_Ini},Auto-Tribute,Auto-Tribute (On/Off)" autoTribute bool outer
	| Import NetBots HUD array
	/call IniToArrayV "${advSettings_Ini},NetBots HUD,NetBot#" netBotsHUDArray

  /call iniToVarV "${genSettings_Ini},Background,AutoSetPctAAExp (On/Off)" autoSetPctAAExp bool outer

/if (${Debug} || ${Debug_Background}) /echo <== background_Setup -|
/return

|----------------------------------------------------------------------------------------------------|
SUB background_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug Background (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Background,Auto-Destroy List"
	/call WriteToIni "${genSettings_Ini},Background,Idle Time Out (Min)" 10
	/call WriteToIni "${genSettings_Ini},Background,Auto-Inventory Timer (Sec)" 30
	/call WriteToIni "${genSettings_Ini},Background,Close Spellbook Timer (Sec)" 30
  /call WriteToIni "${genSettings_Ini},Background,AutoSetPctAAExp (On/Off)" On
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
	/doevents updateAliases
  /doevents YourKill
/return