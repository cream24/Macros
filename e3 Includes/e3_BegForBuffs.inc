|----------------------------------------------------------------------------------------------------|
|- e3_Macro_BegForBuffs.inc	v2.0 ( originally designed to work with e3.mac )						-|
|- Written by Killians of PEQ																		-|
|----------------------------------------------------------------------------------------------------|
|- Allows chat senders to request buffs by their full name, or alias listed in the BuffAliases_Ini.	-|
|----------------------------------------------------------------------------------------------------|
| Make an event to change buff request toggle
| Adding restricted spells
| add a -group function, to cast on all group members
| Revamp begforbuffs to work off castReturns set in Casting, rather than checking range mana etc before hand. (this could make it easier to add items and abilities to the begforbuffs function.)


|------------------------------------------------------------------------|
|- Request spells by name or alias.										-|
|------------------------------------------------------------------------|
|- @param ChatChannel: The channel a chat command was given through.	-|
|- @param ChatSender: The PC who sent the chat message.					-|
|- @param ChatText: The message that was sent.							-|
|------------------------------------------------------------------------|
| Specify chat channels to monitor.
|#chat group
|#chat tell
SUB event_Chat(ChatChannel, ChatSender, ChatText)
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- Event_Chat ==>
  |/echo ${ChatChannel} ${ChatSender} ${ChatText}
  /declare newChatSender string local ${ChatSender}
	| Make sure that ChatText is not an integer, this will cause an unparsable calculation.
	/if (!${Int[${ChatText}]}) {
		/declare whatToCast string local
		/declare spellBeingCast string local

	  |/echo ChatSender ${ChatSender} ${ChatChannel} ${ChatText}
		| If the ChatText is not found in the spellbook, check SpellAliases_Ini.
		:Retry_Alias
		/if (${Me.Book[${ChatText}]}) {
			/varset whatToCast ${Spell[${ChatText}]}
			/varset spellBeingCast ${Spell[${ChatText}]}
			
		} else /if (${FindItemCount[=${ChatText}]}) {
			
			/varset whatToCast ${FindItem[=${ChatText}]}
			/varset spellBeingCast ${FindItem[=${ChatText}].Spell}

		} else {
			
			| Replace :'s with ;'s to adjust for inis
			/if (${ChatText.Find[:]}) /varset ChatText ${ChatText.Replace[:,;]}
			
			| Check the ini for an alias
			/if (${Ini[${SpellAliases_Ini},${Me.Class},${ChatText}].Length}) {
			
				| Disable follow before porting.
				/if (${ChatText.Find[Port]}) {
					/if (${Stick.Active}) /squelch /stick off
					/if (${NetAdvPath.Status}==1) /squelch /netfollow off
					/varset FollowTarget NULL
					/varset Following FALSE
					/delay 30 !${Stick.Active} && ${NetAdvPath.Status}==0
				}

				| Reset variables using data from SpellAliases_Ini
				/varset ChatText ${Ini[${SpellAliases_Ini},${Me.Class},${ChatText}].Replace[;,:]}
				/if (${Debug} || ${Debug_BegForBuffs}) /echo |- Event_Chat -| :Retry_Alias
				/goto :Retry_Alias
			}
		}
		
		| If I found something to cast.
		/if (${Bool[${whatToCast}]}) {
		
			| Verify the event
			/call verifyEvent "${newChatSender}" "${ChatText}"
			/if (${Defined[newChatSender]}) /varset newChatSender ${Macro.Return.Arg[1,_]}
		
			| If the ChatSender IS a NetBot
			/if (${Bool[${Macro.Return.Arg[2,_]}]}) {
				
				:allowAccess
				
				| Check zone
				/if (!${SpawnCount[pc ${newChatSender}]}) {
					/if (${ChatChannel.Equal[TELL]}) {
						/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Zone].Length}) {
							/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Zone]}
						} else {
							/tell ${newChatSender} We're not in the same zone, fool!
						}
					}
				} else {
				
					| Check the request was a beneficial spell
					/if (${Spell[${ChatText}].SpellType.Left[10].NotEqual[Beneficial]}) {
						/if (${ChatChannel.Equal[TELL]}) {
							/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Not Beneficial].Length}) {
								/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Not Beneficial]}
							} else {
								/tell ${newChatSender} You're only allowed to request happy spells, fool!
							}
						} else {
							/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Not Beneficial].Length}) {
								/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Not Beneficial]}
							} else {
								/g You're only allowed to request happy spells, fool!
							}
						}
					} else {
				
						| Check restricted spells list
						/if (${Select[${ChatText},${Ini[${Character_Ini},Buffs,Restricted Spells]}]} || ${ChatText.Find[Gate]} || ${ChatText.Find[Gate]} || ${ChatText.Find[Evacuate]} || ${ChatText.Find[portal]} || ${ChatText.Find[Relocation]} || ${ChatText.Find[Succor]} || ${ChatText.Find[Circle of]} || ${ChatText.Find[Ring of]} || ${ChatText.Find[Wind of]}) {
							/if (${NetBots[${newChatSender}].ID}) {
								/goto :castRestricted
							} else {
								/if (${ChatChannel.Equal[TELL]}) {
									/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted Spell].Length}) {
										/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted Spell]}
									} else {
										/tell ${newChatSender} You're only allowed to request happy spells, fool!
									}
								} else {
									/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted Spell].Length}) {
										/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted Spell]}
									} else {
										/g You're only allowed to request happy spells, fool!
									}
								}
							}
						} else {

							:castRestricted
						
							| Check for self only spells
							/if (${Spell[${spellBeingCast}].TargetType.Find[self]}) {
								/if (${ChatChannel.Equal[TELL]}) {
									/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Self Buff].Length}) {
										/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Self Buff]}
									} else {
										/tell ${newChatSender} That's a self only buff, fool!
									}
								} else {
									/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Self Buff].Length}) {
										/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Self Buff]}
									} else {
										/g That's a self only buff, fool!
									}
								}
							} else {
							
								| Check range.
								/if (${Spell[${spellBeingCast}].MyRange} && ${Spawn[pc ${newChatSender}].Distance} > ${Spell[${spellBeingCast}].MyRange}) {
									/if (${ChatChannel.Equal[TELL]}) {
										/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range].Length}) {
											/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range]}
										} else {
											/tell ${newChatSender} You're out of range, fool!
										}
									} else {
										/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range].Length}) {
											/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range]}
										} else {
											/g You're out of range, fool!
										}
									}
								} else {
								
									| Check mana.
									/if (${whatToCast.Arg[1,/].Equal[${spellBeingCast}]} && ${Spell[${spellBeingCast}].Mana} > ${Me.CurrentMana}) {
										/if (${ChatChannel.Equal[TELL]}) {
											/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Mana].Length}) {
												/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Mana]}
											} else {
												/tell ${newChatSender} I don't have enough mana to cast ${spellBeingCast}, Try again later.
											}
										} else {
											/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Mana].Length}) {
												/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Mana]}
											} else {
												/g I don't have enough mana to cast ${spellBeingCast}, Try again later.
											}
										}
									} else {

										| Cast the requested spell, on the ChatSender.
										/if (${Target.ID}) /declare oldTarget int local ${Target.ID}
										/if (${ChatChannel.Equal[TELL]}) {
											/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Casting Buff].Length}) {
												/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Casting Buff]}
											} else {
												/tell ${newChatSender} ${spellBeingCast} inc...
											}
										} else {
											/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Casting Buff].Length}) {
												/g ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Casting Buff]}
											} else {
												/g ${spellBeingCast} inc...
											}
										}

                    /if (${Me.Book[${whatToCast}]} && !${Bool[${Me.Gem[${whatToCast}]}]}) /call memorize_spell "${whatToCast}" ${DefaultGem}
                    |/delay 5s ${Me.SpellReady[${spellName}]}
                    /delay 20s ${Me.SpellReady[${spellName}]} || ${Me.ItemReady[=${spellName}]} || ${Me.AbilityReady[${spellName}]}
										/casting "${whatToCast}" ${DefaultGem} "-targetid|${Spawn[pc =${newChatSender}].ID}" -maxtries|3
										/delay 20s !${Cast.Status.Find[C]}
									}
								}		
							}
						}
					}
				}
				
			| If the ChatSender is NOT a NetBot	
			} else {
		
				| Check Restricted PCs
				/if (${Select[${newChatSender},${Ini[${genSettings_Ini},Buff Requests,Restricted PCs (When Requests [On])]}]}) {
					/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted PC].Length}) {
						/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted PC]}
					} else {
						/tell ${newChatSender} You are an asshole.  I will not buff you, because you are an asshole.
					}
				} else {
				
					| Check genSettings_Ini > [Buff Requests] > Allow Buff Requests (On/Off)
					/call IniToVar "${genSettings_Ini},Buff Requests,Allow Buff Requests (On/Off)" allow_BuffRequests bool local
					/if (${Bool[${Macro.Return}]}) {
						/goto :allowAccess
					} else {
						
						| Check genSettings_Ini > [Buff Requests] > Allowed PCs (When Requests [Off])
						/if (${Select[${newChatSender},${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} || ${Select[Guild,${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} && ${Spawn[pc ${ChatSender}].Guild.Equal[${Me.Guild}]} || ${Select[Raid,${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} && ${Raid.Member[${ChatSender}].Level}) {
							/goto :allowAccess
						} else {
							/if (${ChatChannel.Equal[TELL]}) {
								/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Requests Off].Length}) {
									/tell ${newChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Requests Off]}
								} else {
									/tell ${newChatSender} Sorry, I'm not buffing right now.
								}
							}
						}
					}
				}
			}
		}
	}
	
/if (${Debug} || ${Debug_BegForBuffs}) /echo <== Event_Chat -|
/RETURN
	
	

|--------------------------------------------------------|
|- Relays buff requests from /tell and /group to EQBC.	-|
|--------------------------------------------------------|
|- @param line: 'Name tells you, 'Buff me'.'			-|
|- @param buffTarget: The target to cast buffs on.		-|
|--------------------------------------------------------|
#event buffRequestRelay "#1# tells you, 'Buff #*#'"
#event buffRequestRelay "#1# tells you, 'Buff #*# |-|#2#'"
SUB event_buffRequestRelay(line, ChatSender, eventParams)
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- event_buffRequestRelay ==>

	| Verify the event
	/call verifyEvent "${ChatSender}" "${ChatText}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}

	|    |-----------Valid User----------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]}) {
	
		:allowAccess

		| Check zone
		/if (!${SpawnCount[pc ${ChatSender}]}) {
			/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Zone].Length}) {
				/tell ${ChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Zone]}
			} else {
				/tell ${ChatSender} We're not in the same zone, fool!
			}
		} else {
		
			| Check range.
			/if (${Spawn[pc ${ChatSender}].Distance} > 200) {
				/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range].Length}) {
					/tell ${ChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Out of Range]}
				} else {
					/tell ${ChatSender} You're out of range, fool!
				}
			} else {
		
				| -------------------Find buffTarget------------------- |
				/declare buffTarget int local
				/declare castName string local
				/declare gemNumber string local gem${DefaultGem}
				/declare groupList string local
				
				/declare i int local
				/declare x int local

				/if (${Int[${line.Arg[3, ]}]}) {
					/varset buffTarget ${line.Arg[3, ]}
				} else /if (${line.Find[Buff me]}) {
					/varset buffTarget ${Spawn[pc =${ChatSender}].ID}
				} else /if (${line.Find[Buff pet]} || ${line.Find[Buff my pet]}) {
					/varset buffTarget ${Spawn[pet ${Spawn[pc =${ChatSender}].Pet}].ID}
				} else  /if (${SpawnCount[pc ${line.Arg[3, ]}]}) {
					/varset buffTarget ${Spawn[pc =${line.Arg[3, ]}].ID}
				} else /if (${line.Find['s pet]}) {
					/varset buffTarget ${Spawn[pet ${Spawn[pc ${line.Arg[3, ].Arg[1,']}].Pet}].ID}
				} else /if (${line.Find[s pet]}) {
					/varset buffTarget ${Spawn[pet ${Spawn[pc ${line.Arg[3, ].Left[${Math.Calc[${line.Arg[3, ].Length} - 1].Int}]}].Pet}].ID}
				}
		
				| -------------------Verify the buffTarget------------------- |
				/if (${GroupBuffs.Size}) {
					/tell ${ChatSender} Casting buffs on ${Spawn[id ${buffTarget}].CleanName}...
					/bc ${If[${Defined[eventParams]},Buff ${buffTarget} -${eventParams},Buff ${buffTarget}]}
					
				| If I don't have buffs, but others might.
				} else /if (!${GroupBuffs.Size} && ${NetBots.Counts}>1) {
					/tell ${ChatSender} Requesting buffs on ${Spawn[id ${buffTarget}].CleanName}...
					/bc ${If[${Defined[eventParams]},Buff ${buffTarget} -${eventParams},Buff ${buffTarget}]}
					
				| If I'm not a buffer, and no other bots are on.
				} else /if (!${GroupBuffs.Size} && ${NetBots.Counts}<=1) {
					/tell ${ChatSender} Sorry, I'm not currently set up to cast any buffs, try again later. (This is an automated response)
				}
			}
		}
	} else {
	
		| Check Restricted PCs
		/if (${Select[${ChatSender},${Ini[${genSettings_Ini},Buff Requests,Restricted PCs (When Requests [On])]}]}) {
			/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted PC].Length}) {
				/tell ${ChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Request-Restricted PC]}
			} else {
				/tell ${ChatSender} You are a giant ass.  I will not buff you, because you are an ass.
			}
		} else {
			
			| If user is not valid, check Allow Buff Requests
			/call IniToVar "${genSettings_Ini},Buff Requests,Allow Buff Requests (On/Off)" allow_BuffRequests bool local
			
			/if (${Bool[${Macro.Return}]}) {
				/goto :allowAccess
			} else {
			
				/if (${Select[${ChatSender},${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} || ${Select[Guild,${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} && ${Spawn[pc ${ChatSender}].Guild.Equal[${Me.Guild}]} || ${Select[Raid,${Ini[${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])]}]} && ${Raid.Member[${ChatSender}].Level}) {
					/goto :allowAccess
				} else {
					/if (${Ini[${genSettings_Ini},User Defined Responses,Buff Requests Off].Length}) {
						/tell ${ChatSender} ${Ini[${genSettings_Ini},User Defined Responses,Buff Requests Off]}
					} else {
						/tell ${ChatSender} Sorry, I'm not buffing right now.
					}	
				}
			}
		}
	}

/if (${Debug} || ${Debug_BegForBuffs}) /echo <== event_buffRequestRelay -|
/RETURN



#event portRelay "<#*#> Port to #1#"
SUB event_portRelay(line, destination)
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- EVENT_portRelay ==>

	/if (${Group} && ${Me.CleanName.Equal[${Group.Leader}]}) {
		/g Port ${destination}
	}

/if (${Debug} || ${Debug_BegForBuffs}) /echo <== EVENT_portRelay -|
/RETURN



#EVENT makeSpellList "[MQ2] Make Spell List"
#EVENT makeSpellList "<#*#> Make Spell Lists"
SUB EVENT_makeSpellList
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- EVENT_makeSpellList ==>

	/if (${Me.Class.CanCast} && ${Me.Class.ShortName.NotEqual[BRD]}) {
		/if (${Bool[${Ini[${SpellList_Ini}]}]}) {
			/echo Creating a list of ${Me.Class} spells...
		} else {
			/echo Adding spells to [${SpellList_Ini}] please wait...
		}

		/call Build_Alphabetized_Ini "${SpellList_Ini}"
		
		/declare i int local
		/declare popupVar string local .
		/declare spellName string local
		/declare miscTimer timer local 15
		
		/for i 1 to 720
		
			/if (!${miscTimer}) {
				/if (${popupVar.Length} == 6) /varset popupVar . 
				/popup ${popupVar}
				/varset popupVar ${popupVar}.
				/varset miscTimer 5
			}
		
			/if (${Bool[${Me.Book[${i}]}]} && ${Spell[${Me.Book[${i}]}].SpellType.NotEqual[Detrimental]} && ${Spell[${Me.Book[${i}]}].TargetType.NotEqual[Self]} && ${Spell[${Me.Book[${i}]}].TargetType.NotEqual[Pet]}) {
				/varset spellName ${Me.Book[${i}]}
				/if (!${Bool[${Ini[${SpellList_Ini},${spellName.Left[1]},${spellName}]}]}) /call WriteToIni "${SpellList_Ini},${spellName.Left[1]},${spellName}"
			}
		
		/next i

		/echo Finished adding spells.  Please edit [${SpellList_Ini}], and then use /importspellaliases to import your changes.
	}

/if (${Debug} || ${Debug_BegForBuffs}) /echo <== EVENT_makeSpellList -|
/RETURN



#EVENT import_SpellAliases "[MQ2] Import Spell Aliases"
#EVENT import_SpellAliases "<#*#> Import Spell Aliases"
SUB EVENT_import_SpellAliases
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- EVENT_import_SpellAliases ==>

	/if (!${Bool[${Ini[${SpellList_Ini}]}]}) {
		/echo You must create a spell list file using, /MakeSpellList.  After editing the newly created file, use /importspellaliases to import your changes.
	} else {
		
		/declare i int local
		/declare e int local
		
		/for i 1 to 26
		
			/if (${Bool[${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}]}]}) {
			
				/varset e 1
				:next_Alias
				/if (${Bool[${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|]}]}) {
					/if (${Bool[${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]},${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|]}]}]}) /call WriteToIni "${SpellAliases_Ini},${Me.Class},${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]},${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|]}]}" "${If[${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|].Find[;]},${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|].Replace[;,:]},${Ini[${SpellList_Ini},${Ini[${SpellList_Ini}].Arg[${i},|]}].Arg[${e},|]}]}" 1
					/varcalc e ${e} + 1
					/goto :next_Alias
				}
			}
		/next i
		
		/echo New spell aliases have been imported.
	}
	
/if (${Debug} || ${Debug_BegForBuffs}) /echo <== EVENT_import_SpellAliases -|
/RETURN



#EVENT createSpellAlias "[#1#] Set Spell Alias #2# #3#"
#EVENT createSpellAlias "<#1#> Set Spell Alias #2# #3#"
SUB EVENT_createSpellAlias(line, ChatSender, newAlias, spellName)
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- EVENT_createSpellAlias ==>
	/if (${ChatSender.Equal[${Me.CleanName}]} || ${ChatSender.Equal[MQ2]}) {
		/docommand ${ChatToggle} Adding spell alias [${newAlias}] to cast [${spellName}] for [${Me.Class}s].
		/call WriteToIni "${SpellAliases_Ini},${Me.Class},${newAlias}" "${spellName}" 1
	}
/if (${Debug} || ${Debug_BegForBuffs}) /echo <== EVENT_createSpellAlias -|
/RETURN



|--------------------------------|
|- Setup BegForBuff variables.	-|
|--------------------------------|
SUB begForBuffs_Setup
/call IniToVar "${advSettings_Ini},Debug,Debug BegForBuffs (On/Off)" Debug_BegForBuffs bool outer
/if (${Debug} || ${Debug_BegForBuffs}) /echo |- setup_BegForBuffs ==>
	|Add SpellAlias File Path.
	/if (!${Ini[${MacroData_Ini},File Paths,Spell Aliases File].Length}) /call WriteToIni "${MacroData_Ini},File Paths,Spell Aliases File" "e3 Macro Inis\Spell Aliases.ini" 1
	|Add Spell List File Path.
	/if (!${Ini[${MacroData_Ini},File Paths,Spell List Files].Length}) {
		/noparse /ini "e3 Macro Inis\e3 Data.ini" "File Paths" "Spell List Files" "Macro Inis\Spell List-${Me.Class.ShortName}.ini"
	}
	|Import SpellAliases_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Spell Aliases File].Length}) {
		/echo ERROR: Could not find designated file path for [Spell Aliases], please review [${MacroData_Ini}] and restart.
		/endmacro
	} else {
		/declare SpellAliases_Ini string outer ${Ini[${MacroData_Ini},File Paths,Spell Aliases File]}
	}
	|Import SpellList_Ini.
	/if (!${Ini[${MacroData_Ini},File Paths,Spell List Files].Length}) {
		/echo ERROR: Could not find designated file path for [Spell List Files], please review [${MacroData_Ini}] and restart.
		/endmacro
	} else {
		/declare SpellList_Ini string outer ${Ini[${MacroData_Ini},File Paths,Spell List Files]}
	}
	/if (!${Ini[${SpellAliases_Ini}].Length}) /call MakeSpellAliases_Ini
/if (${Debug} || ${Debug_BegForBuffs}) /echo <== setup_BegForBuffs -|
/RETURN



Sub begForBuffs_Background_Events
	/doevents createSpellAlias
	/doevents import_SpellAliases
	/doevents portRelay
	/doevents buffRequestRelay
	/doevents Chat
/return



SUB begForBuffs_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug BegForBuffs (On/Off)" Off
	/call WriteToIni "${genSettings_Ini},Buff Requests,Allow Buff Requests (On/Off)" On
	/call WriteToIni "${genSettings_Ini},Buff Requests,Restricted PCs (When Requests [On])"
	/call WriteToIni "${genSettings_Ini},Buff Requests,Allowed PCs (When Requests [Off])" "Guild,Raid"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Out of Zone"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Out of Range"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Out of Mana"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Self Buff"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Not Beneficial"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Restricted Spell"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Casting Buff"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Request-Restricted PC"
	/call WriteToIni "${genSettings_Ini},User Defined Responses,Buff Requests Off"
	/call WriteToIni "${Character_Ini},Buffs,Restricted Spells"
/RETURN



SUB begForBuffs_CharacterSettings
/RETURN



Sub begForBuffs_Aliases
	/noparse /squelch /alias /giveme /tell ${Target.CleanName}
	/noparse /squelch /alias /addalias /echo Set Spell Alias
/return



|----------------------------|
|- Build SpellAliases_Ini.	-|
|----------------------------|
| Needs to be updated
SUB MakeSpellAliases_Ini

	/echo Creating spell aliases file...
	
	| Beastlord Aliases.
	/call WriteToIni "${SpellAliases_Ini},Beastlord,Beastlord Spells"
	/call WriteToIni "${SpellAliases_Ini},Beastlord,SD" "Spiritual Dominion"
	/call WriteToIni "${SpellAliases_Ini},Beastlord,SV" "Spiritual Vigor"
	
	| Clerc Aliases.
	/call WriteToIni "${SpellAliases_Ini},Cleric,Cleric Spells"
	/call WriteToIni "${SpellAliases_Ini},Cleric,Kazad" "Kazad's Mark"
	/call WriteToIni "${SpellAliases_Ini},Cleric,Virtue" "Virtue"
	/call WriteToIni "${SpellAliases_Ini},Cleric,Virt" "Virtue"
	/call WriteToIni "${SpellAliases_Ini},Cleric,HoV" "Hand of Virtue"	
	/call WriteToIni "${SpellAliases_Ini},Cleric,BoR" "Blessing of Reverence"	
	/call WriteToIni "${SpellAliases_Ini},Cleric,CHeal" "Complete Heal"	
	/call WriteToIni "${SpellAliases_Ini},Cleric,HoT" "Holy Elixir"
	/call WriteToIni "${SpellAliases_Ini},Cleric,Heal" "Holy Light"
	/call WriteToIni "${SpellAliases_Ini},Cleric,QHeal" "Supernal Remedy"
	/call WriteToIni "${SpellAliases_Ini},Cleric,GroupHeal" "Word of Replenishment"
	/call WriteToIni "${SpellAliases_Ini},Cleric,GroupHoT" "Ethereal Elixir"	
	/call WriteToIni "${SpellAliases_Ini},Cleric,Temp" "Temperance"
	
	| Druid Aliases.
	/call WriteToIni "${SpellAliases_Ini},Druid,Druid Spells"
	/call WriteToIni "${SpellAliases_Ini},Druid,FoE" "Flight of Eagles"
	/call WriteToIni "${SpellAliases_Ini},Druid,DS" "Legacy of Bracken"
	/call WriteToIni "${SpellAliases_Ini},Druid,BoT9" "Blessing of the Nine"	
	/call WriteToIni "${SpellAliases_Ini},Druid,Heal" "Sylvan Infusion"
	/call WriteToIni "${SpellAliases_Ini},Druid,Regen" "Blessing of Replenishment"
	/call WriteToIni "${SpellAliases_Ini},Druid,CoS" "Circle of Seasons"
	/call WriteToIni "${SpellAliases_Ini},Druid,Str" "Nature's Might"
	/call WriteToIni "${SpellAliases_Ini},Druid,GroupWolf" "Share Form of the Great Wolf"
	/call WriteToIni "${SpellAliases_Ini},Druid,GroupInvis" "Foliage Shield"
	/call WriteToIni "${SpellAliases_Ini},Druid,Evac" "Succor"

	/call WriteToIni "${SpellAliases_Ini},Druid,GPortIceclad" "Circle of Iceclad"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortNK" "Circle of Karana"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortLavastorm" "Circle of Lavastorm"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortSRo" "Circle of Ro"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortMisty" "Circle of Misty"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortFeerrott" "Circle of Feerrott"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortCommons" "Circle of Commons"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortEK" "Succor: East"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortSureFall" "Circle of Surefall Glade"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortButcher" "Circle of Butcher"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortSteamfont" "Circle of Steamfont"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortSkyfire" "Wind of the North"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortEJ" "Wind of the South"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortDawnshroud" "Circle of Dawnshroud"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortDSP" "Circle of Dawnshroud"	
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortTwilight" "Circle of Twilight"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortGrimling" "Circle of Grimling"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortNexus" "Circle of the Nexus"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortStonebrunt" "Circle of Stonebrunt"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortTox" "Circle of Toxxulia"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortBarindu" "Circle of Barindu"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortNatimbi" "Circle of Natimbi"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortPoK" "Circle of Knowledge"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortKnowledge" "Circle of Knowledge"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortCS" "Circle of Cobalt Scar"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortWL" "Circle of Wakening Lands"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortGD" "Circle of Great Divide"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortCombines" "Circle of the Combines"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortDreadlands" "Circle of the Combines"
	/call WriteToIni "${SpellAliases_Ini},Druid,GPortDL" "Circle of the Combines"
  /call WriteToIni "${SpellAliases_Ini},Druid,GPortWallOfSlaughter" "Circle of Slaughter"
  /call WriteToIni "${SpellAliases_Ini},Druid,GPortWoS" "Circle of Slaughter"
  /call WriteToIni "${SpellAliases_Ini},Druid,GPortBloodfields" "Circle of Bloodfields"
  /call WriteToIni "${SpellAliases_Ini},Druid,GPortBF" "Circle of Bloodfields"

	| Enchanter Aliases.
	/call WriteToIni "${SpellAliases_Ini},Enchanter,Enchanter Spells"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,Haste" "Vallon's Quickening"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,VQ" "Vallon's Quickening"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,VoQ" "Voice Of Quellious"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,C5" "Voice Of Quellious"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,KEI" "Koadic's Endless Intellect"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,C4" "Koadic's Endless Intellect"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,GoD" "Guard of Druzzil"
	/call WriteToIni "${SpellAliases_Ini},Enchanter,CHA" "Radiant Visage"
	
	| Magician Aliases.
	/call WriteToIni "${SpellAliases_Ini},Magician,Magician Spells"
	/call WriteToIni "${SpellAliases_Ini},Magician,DS" "Maelstrom of Ro"
	/call WriteToIni "${SpellAliases_Ini},Magician,CoH" "Call of the Hero"

	| Necromancer Aliases.
	/call WriteToIni "${SpellAliases_Ini},Necromancer,Necromancer Spells"
	/call WriteToIni "${SpellAliases_Ini},Necromancer,DMF" "Dead Men Floating"
	/call WriteToIni "${SpellAliases_Ini},Necromancer,IVU" "Invisibility versus Undead"
	/call WriteToIni "${SpellAliases_Ini},Necromancer,SummonCorpse" "Conjure Corpse"
	/call WriteToIni "${SpellAliases_Ini},Necromancer,Sac" "Sacrifice"
	/call WriteToIni "${SpellAliases_Ini},Necromancer,Bind" "Bind Affinity"

	| Paladin Aliases.
	/call WriteToIni "${SpellAliases_Ini},Paladin,Paladin Spells"
	/call WriteToIni "${SpellAliases_Ini},Paladin,BSS" "Brell's Stalwart Shield"

	| Ranger Aliases.
	/call WriteToIni "${SpellAliases_Ini},Ranger,Ranger Spells"
	/call WriteToIni "${SpellAliases_Ini},Ranger,SoTP" "Spirit of the Predator"
	/call WriteToIni "${SpellAliases_Ini},Ranger,Pred" "Spirit of the Predator"
	/call WriteToIni "${SpellAliases_Ini},Ranger,SoT" "Stringth of Tunare"
	/call WriteToIni "${SpellAliases_Ini},Ranger,Tunare" "Strength of Tunare"
	/call WriteToIni "${SpellAliases_Ini},Ranger,Rathe" "Call of the Rathe"
	/call WriteToIni "${SpellAliases_Ini},Ranger,CoR" "Call of the Rathe"
	/call WriteToIni "${SpellAliases_Ini},Ranger,SoE" "Spirit of Eagle"	
	/call WriteToIni "${SpellAliases_Ini},Ranger,SoW" "Spirit of Wolf"
	/call WriteToIni "${SpellAliases_Ini},Ranger,DS" "Shield of Spikes"
	/call WriteToIni "${SpellAliases_Ini},Ranger,CoS" "Circle of Summer"	
	/call WriteToIni "${SpellAliases_Ini},Ranger,CoW" "Circle of Winter"	

	| Shadow Knight Aliases.
	/call WriteToIni "${SpellAliases_Ini},Shadow Knight,Shadow Knight Spells"
	/call WriteToIni "${SpellAliases_Ini},Shadow Knight,Voice" "Voice of Thule"
	/call WriteToIni "${SpellAliases_Ini},Shadow Knight,VoT" "Voice of Thule"
	/call WriteToIni "${SpellAliases_Ini},Shadow Knight,Thule" "Voice of Thule"
	
	| Shaman Aliases.
	/call WriteToIni "${SpellAliases_Ini},Shaman,Shaman Spells"
	/call WriteToIni "${SpellAliases_Ini},Shaman,HoT" "Breath of Trushar"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Heal" "Daluda's Mending"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Fo7" "Focus of the Seventh"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Focus" "Focus of Soul"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Haste" "Talisman of Alacrity"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Billy" "Spirit of Bih`li"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Bih`li" "Spirit of Bih`li"
	/call WriteToIni "${SpellAliases_Ini},Shaman,SoB" "Spirit of Bih`li"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Regen" "Blessing of Replenishment"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Agi" "Talisman of the Wrulan"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Dex" "Talisman of the Raptor"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Sta" "Talisman of the Boar"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Str" "Talisman of the Diaku"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Tribunal" "Talisman of the Tribunal"
	/call WriteToIni "${SpellAliases_Ini},Shaman,ToT" "Talisman of the Tribunal"	
	/call WriteToIni "${SpellAliases_Ini},Shaman,Jag" "Spirit of the Jaguar"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Shrink" "Shrink"
	/call WriteToIni "${SpellAliases_Ini},Shaman,GroupShrink" "Tiny Terror"
	/call WriteToIni "${SpellAliases_Ini},Shaman,GShrink" "Tiny Terror"
	/call WriteToIni "${SpellAliases_Ini},Shaman,RGC" "Remove Greater Curse"
	/call WriteToIni "${SpellAliases_Ini},Shaman,Cha" "Glamour"

	| Wizard Aliases.
	/call WriteToIni "${SpellAliases_Ini},Wizard,Wizard Spells"
	/call WriteToIni "${SpellAliases_Ini},Wizard,Decess" "Decession"
	/call WriteToIni "${SpellAliases_Ini},Wizard,Evac" "Decession"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TL" "Teleport"
	/call WriteToIni "${SpellAliases_Ini},Wizard,DS" "O'Keils Radiation"
	
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortWK" "West Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortNRo" "Ro Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortCommons" "Common Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortCazic" "Cazic Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortNek" "Nek Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortNektulos" "Nek Portal"	
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortNK" "North Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortGfay" "Fay Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortEJ" "Markar's Relocation"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortEmerald" "Markar's Relocation"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportSkyfire" "Tishan's Relocation"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportDL" "Combine Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportDreadlands" "Combine Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportCombines" "Combine Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportDawnshroud" "Dawnshroud Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportDSP" "Dawnshroud Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportTwilight" "Twilight Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportGrimling" "Grimling Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportNexus" "Nexus Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GroupTL" "Translocate: Group"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportTx" "Tox Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportBarindu" "Barindu Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportNatimbi" "Natimbi Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportPoK" "Knowledge Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GportKnowledge" "Knowledge Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortSky" "Alter Plane: Sky"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortHate" "Alter Plane: Hate"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortCS" "Cobalt Scar Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortWL" "Wakening Lands Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortGD" "Great Divide Portal"
	/call WriteToIni "${SpellAliases_Ini},Wizard,GPortIceclad" "Iceclad Portal"
  /call WriteToIni "${SpellAliases_Ini},Wizard,GPortWallOfSlaughter" "Slaughter Portal"
  /call WriteToIni "${SpellAliases_Ini},Wizard,GPortWoS" "Slaughter Portal"
  /call WriteToIni "${SpellAliases_Ini},Wizard,GPortBloodfields" "Bloodfields Portal"
  /call WriteToIni "${SpellAliases_Ini},Wizard,GPortBF" "Bloodfields Portal"

	/call WriteToIni "${SpellAliases_Ini},Wizard,TLDawnshroud" "Translocate: Dawnshroud"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLCazic" "Translocate: Cazic"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLNRo" "Translocate: Ro"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLWC" "Translocate: West"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLNek" "Translocate: Nek"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLCommon" "Translocate: Common"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLFay" "Translocate: Fay"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLCombine" "Translocate: Combine"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLTwilight" "Translocate: Twilight"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLGrimling" "Translocate: Grimling"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLNexus" "Translocate: Nexus"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLGroup" "Translocate"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLTox" "Translocate: Tox"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLNatimbi" "Translocate: Natimbi"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLBarindu" "Translocate: Barindu"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLPoK" "Translocate: Knowledge"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLCS" "Translocate: Cobalt Scar"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLWL" "Translocate: Wakening Lands"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLGD" "Translocate: Great Divide"
	/call WriteToIni "${SpellAliases_Ini},Wizard,TLIceclad" "Translocate: Iceclad"
	
/RETURN