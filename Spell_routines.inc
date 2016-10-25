| spell_routines.inc v2.4 
| 
| Originally Written by Rusty~ 
| Upkeep performed by A_Druid_00 
| 
| Includes MQ2Exchange integration for item swapping by BrainDeath 
| Also includes FD fix that would cause an endless loop if you were hit by FD mid-cast by A_Druid_00 
| 
| Features: 
| - Casts spells, clicks items, or uses AA abilities for you 
| - Allows back to back casting without waiting on spell gems to pop all the way up 
| - Will interrupt spell if target dies while casting. If on a mount, it will dismount and duck if the time left 
|   is greater than 7 seconds, else it will move forward a bit to interrupt, then move you back 
|    ** IMPORTANT: if you don't want to interrupt a spell while mounted, put this at the top of your macro: ** 
|    **   /declare noInterrupt int outer 1                                                                  ** 
| - Allows you to use items in bags. Equips item, clicks it, then returns it to its previous location 
| - Lets you set how long you want to keep trying to cast the spell (defaults to 0) 
|   If the spell is interrupted before the given time, it will recast, else it will return CAST_INTERRUPTED 
| - Lets you call a custom subroutine while waiting for spell to finish casting 
|   Try to keep custom subroutines very small. A common use would be to interrupt the spell if a certain condition is true 
| - This file also includes a sub named Interrupt. You can call this to interrupt any spell you're casting instantly. 
| - You can also use the SwapItem sub included in this to swap items to certain slots 
| - Added EquipItem sub to easily equip items in your main Inventory slots. 
| - Note: if you don't want this to cast spells while you're invis, in your main macro have this at the top: 
|      /declare noInvis int outer 1 
|   This will make it return CAST_INVIS if you're invis 
| 
|  Below is a list of outer scope variables you can access in your macros: 
|      refreshTime        - How much time is left till you're done recovering from casting 
|      castEndTime        - How much time left till you're done casting the current spell... usable in custom spell Subs 
|      spellNotHold       - 1 if your last spell didn't take hold, 0 otherwise 
|      spellRecastTime1-9 - How much time left till that spell is back up 
| 
|====================================================================================================================== 
|  EquipItem:  An easier way to equip items you have in bags ( useful for weapons or focus items ) 
|              slot name is optional. If not given, it will equip it in the first possible spot 
| 
|    Usage:    
|        /call EquipItem "item name|slotname" 
| 
|        Returns: "old item name|slotname" 
|    Examples: 
|    
|    To Equip Sharp Ended Broken Lever when you have Serpent of Vindication equiped: 
|        /call EquipItem "Sharp Ended Broken Lever" 
|    It will return "Staff of Vindication|mainhand" 
|    To reequip the original item, you can save the return in a variable, and then use it later like this: 
|       /varset oldPrimary ${Macro.Return} 
|       | ... do stuff here with your new item equiped 
|       /call EquipItem ${oldPrimary} 
| 
|====================================================================================================================== 
|  SwapItem:  a subroutine which is used in the Cast sub itself. You don't need to do this to cast an item in a bag 
|             but you can call it in your macro to SwapItems (such as weapons or focus items) 
|    Usage:    
|        /call SwapItem "item name" slotname|slotID 
|    Examples: 
|    
|    To swap Darkblade of the Warlord to your main hand: 
|        /call SwapItem "Darkblade of the Warlord" mainhand 
| 
|    To swap stat food in one bag with other food in another bag: 
|        /call SwapItem "Bristlebanes Party Platter" ${FindItem[halas 10lb meat pie].InvSlot} 
| 
|====================================================================================================================== 
|  Cast: the main subroutine that casts spells or items for you 
|     Usage: 
|        /call Cast "spellname|itemname|AAname|AA#" [item|alt|gem#] [give up time][s|m] [custom subroutine name] 
|     Examples: 
| 
|     To cast Howl of Tashan and mem it in slot 3 if not memmed: 
|       /call Cast "Howl of Tashan" gem3 
| 
|     To cast Arcane Rune and keep trying for 7 seconds, in case of interrupts. 
|       /call Cast "Arcane Rune" gem5 7s 
| 
|     To click Grim Aura earring that's in a bag: 
|       /call Cast "Shrunken Goblin Skull Earring" item 
| 
|     To use AA ability Eldritch Rune: 
|       /call Cast "Eldritch Rune" alt 
|         or 
|       /call Cast "173" alt 
| 
|     To call a subroutine that interrupts CH if target gets healed before it lands: 
|       /call Cast "Complete Healing" gem1 0 CheckHP 
|     Then in your macro have somewhere: 
|       Sub CheckHP 
|          /if ( ${Target.PctHPs}>=80 ) /call Interrupt 
|       /return 
| 
| Returns these values: 
|----------------------+----------------------------------------------------------------------+ 
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
| CAST_RESISTED        | Your spell was resisted!                                             | 
| CAST_SUCCESS         | Your spell was cast successfully! (yay)                              | 
| CAST_UNKNOWNSPELL    | Spell/Item/Ability was not found                                     | 
| CAST_NOTHOLD         | Spell woundn't take hold on target                                   | 
|----------------------+----------------------------------------------------------------------+ 

#event BeginCast "You begin casting#*#" 
#event Collapse "Your gate is too unstable, and collapses.#*#" 
#event FDFail "#1# has fallen to the ground.#*#" 
#event Fizzle "Your spell fizzles#*#" 
#event Immune "Your target is immune to changes in its attack speed#*#" 
#event Immune "Your target is immune to changes in its run speed#*#" 
#event Immune "Your target cannot be mesmerized#*#" 
#event Interrupt "Your casting has been interrupted#*#" 
#event Interrupt "Your spell is interrupted#*#" 
#event NoHold "Your spell did not take hold#*#" 
#event NoHold "Your spell would not have taken hold#*#" 
#event NoHold "You must first target a group member#*#" 
#event NoHold "Your spell is too powerful for your intended target#*#" 
#event NoLOS "You cannot see your target.#*#" 
#event NoMount "#*#You can not summon a mount here.#*#" 
#event NoTarget "You must first select a target for this spell!#*#" 
#event NotReady "Spell recast time not yet met.#*#" 
#event OutOfMana "Insufficient Mana to cast this spell!#*#" 
#event OutOfRange "Your target is out of range, get closer!#*#" 
#event Recover "You haven't recovered yet...#*#" 
#event Recover "Spell recovery time not yet met#*#" 
#event Resisted "Your target resisted the #1# spell#*#" 
#event Resisted2 "You resist the #1# spell#*#" 
#event Standing "You must be standing to cast a spell#*#" 
#event Stunned "You are stunned#*#" 
#event Stunned "You can't cast spells while stunned!#*#" 
#event Stunned "You *CANNOT* cast spells, you have been silenced!#*#" 

Sub Cast(spellName,spellType,giveUpValue,mySub,int ResistTotal) 
/declare castTime float local 
/if (!${Defined[castReturn]}) /declare castReturn string outer CAST_CANCELLED 
/if (${Me.Invis} && ${noInvis}) /return 
/if (${spellType.Equal[item]}) { 
  /if (!${FindItem[${spellName}].InvSlot}) /return CAST_UNKNOWNSPELL 
  /varset castTime ${FindItem[${spellName}].CastTime} 
} else /if (${spellType.Equal[alt]}) { 
  /if (!${Me.AltAbilityReady[${spellName}]}) /return CAST_NOTREADY 
  /varset castTime ${Me.AltAbility[${spellName}].Spell.MyCastTime} 
} else { 
  /if (!${Me.Book[${spellName}]}) /return CAST_UNKNOWNSPELL 
  /declare spellID int local ${Me.Book[${Me.Book[${spellName}]}].ID} 
  /varset castTime ${Spell[${spellName}].MyCastTime} 
  /if (${Me.CurrentMana}<${Spell[${spellID}].Mana}) /return CAST_OUTOFMANA 
} 
/if (${castTime}>0.1) { 
  /if (${Stick.Status.Equal[ON]}) /stick pause 
  /if (${FollowFlag}) /call PauseFunction 
  /if (${Me.Moving}) /keypress back 
} 
/if (!${Defined[spellType]}) /declare spellType string local spell 
/if (!${Defined[spellRecastTime1]}) { 
  /if (!${Defined[noInterrupt]}) /declare noInterrupt int outer 0 
  /declare ResistCounter int outer 
  /declare moveBack bool outer false 
  /declare selfResist int outer 
  /declare selfResistSpell string outer 
  /declare giveUpTimer timer outer 
  /declare castEndTime timer outer 
  /declare refreshTime timer outer 
  /declare itemRefreshTime float outer 
  /declare i int local 
  /declare spellNotHold int outer 
  /delay 5 
  /for i 1 to 12 
  /declare spellRecastTime${i} timer outer 
  /if (${Me.SpellReady[${i}]}) { 
    /varset spellRecastTime${i} 0 
  } else { 
    /varcalc spellRecastTime${i} 10*${Me.Gem[${i}].RecastTime} 
  } 
  /next i 
} 
/if (${Defined[giveUpValue]}) /varset giveUpTimer ${giveUpValue} 
/if (${Defined[ResistTotal]}) /varset ResistCounter ${ResistTotal} 
/varset spellNotHold 0 
/varset selfResist 0 
:wait_for_stop 
/if (${Me.Casting.ID} || (${Me.Moving} && ${castTime}>0.1)) { 
  /if (${Bool[${mySub}]}) /call ${mySub} ${spellID} 
  /goto :wait_for_stop 
} 
/if (${Window[SpellBookWnd].Open}) /keypress spellbook 
/if (${Me.Ducking}) /keypress duck 
/call DoCastingEvents 
/varset castReturn X 
/if (${spellType.Equal[item]}) /call ItemCast "${spellName}" "${mySub}" 
/if (${spellType.Equal[alt]}) /call AltCast "${spellName}" "${mySub}" 
/if (${spellType.NotEqual[item]} && ${spellType.NotEqual[alt]}) /call SpellCast "${spellType}" "${spellName}" "${mySub}" "${spellID}" "${giveUpValue}" 
/if (${Stick.Status.Equal[PAUSED]}) /squelch /stick unpause 
/if (${PauseFlag}) /call PauseFunction 
/varset giveUpTimer 0 
/varset ResistCounter 0 
/return ${castReturn} 

Sub SpellCast(spellType,spellName,mySub,int spellID,giveUpValue) 
:cast_spell 
/if (!${Me.Gem[${spellName}]}) { 
  /if (${Cursor.ID}) /call ClearCursor 
  /if (${spellType.Left[3].Equal[gem]}) { 
    /if (${spellType.Length}==4) /memspell ${spellType.Right[1]} "${spellName}" 
    /if (${spellType.Length}==5) /memspell ${spellType.Right[2]} "${spellName}" 
  } else { 
    /return CAST_NOTMEMMED 
  } 
  /if (${Bool[${mySub}]}) /call ${mySub} ${spellID} 
  /delay 6s ${Me.Gem[${spellName}]} 
  /if (${Me.Gem[${spellName}]}) /varcalc spellRecastTime${Me.Gem[${spellName}]} 10*${Spell[${spellID}].RecastTime} 
  /if (!${Me.Gem[${spellName}]}) /return CAST_INTERRUPTED 
  :wait_for_mem 
  /if (${Bool[${mySub}]}) /call ${mySub} ${spellID} 
  /delay 15s ${Me.SpellReady[${spellName}]} 
  /if (!${Me.SpellReady[${spellName}]}) { 
    /if (${giveUpTimer}) /goto :wait_for_mem 
    /return CAST_NOTREADY 
  } 
} 
/varset spellType spell 
/if (${spellName.Find[illusion: ]} && ${Me.AltAbilityReady[project illusion]}) /call Cast "project illusion" alt 
/varset giveUpTimer ${giveUpValue} 
/declare recoverWaitTime timer local 30 
:cast_spell_loop 
/if (!${Me.SpellReady[${spellName}]} && (${spellRecastTime${Me.Gem[${spellName}]}}<${giveUpTimer} || ${refreshTime}>0 || ${castReturn.Equal[CAST_RESISTED]})) { 
  /if (${Bool[${mySub}]}) /call ${mySub} ${spellID} 
  /goto :cast_spell_loop 
} else { 
  /if (!${Me.SpellReady[${spellName}]} && !${castReturn.Equal[CAST_RESISTED]}) /return CAST_NOTREADY 
} 
/cast "${spellName}" 
/if (${Me.Casting.ID}) { 
  /varset spellID ${Me.Casting.ID} 
  /varcalc castEndTime ${Me.Casting.MyCastTime}*10 
  /if (${castEndTime}<${Math.Calc[${Me.Casting.CastTime}*5]}) /varcalc castEndTime ${Me.Casting.CastTime}*5 
} 
/varset moveBack false 
/call WaitCast ${mySub} ${spellID} 
/if (${moveBack}) { 
  /keypress back hold 
  /delay 4 
  /keypress back 
  /delay 15 !${Me.Moving} 
} 
/if (${castReturn.Equal[CAST_CANCELLED]}) /return CAST_CANCELLED 
/call DoCastingEvents 
/if (!${castReturn.Equal[CAST_SUCCESS]}) { 
  /if (${castReturn.Equal[CAST_RECOVER]}) { 
    /if (!${recoverWaitTime}) { 
      /varcalc spellRecastTime${Me.Gem[${spellName}]} 10*${Spell[${spellID}].RecastTime} 
      /if (!${giveUpTimer}) /return CAST_NOTREADY 
    } 
    /goto :cast_spell_loop 
  } 
  /if (${castReturn.Equal[CAST_RESTART]} || ${castReturn.Equal[CAST_STUNNED]} || ${castReturn.Equal[CAST_FIZZLE]} || ${castReturn.Equal[CAST_COLLAPSE]} || (${castReturn.Equal[CAST_INTERRUPTED]} && ${giveUpTimer}) || (${castReturn.Equal[CAST_RESISTED]} && ${ResistCounter})) /goto :cast_spell_loop 
} 
/if (!${castReturn.Equal[CAST_CANNOTSEE]} && !${castReturn.Equal[CAST_OUTOFRANGE]} && !${castReturn.Equal[CAST_OUTOFMANA]} && !${castReturn.Equal[CAST_NOTARGET]} && !${castReturn.Equal[CAST_INTERRUPTED]}) { 
  /varcalc refreshTime 10*${Spell[${spellID}].RecoveryTime} 
  /varcalc spellRecastTime${Me.Gem[${spellName}]} 10*${Spell[${spellID}].RecastTime} 
} 
/return 

Sub ItemCast(spellName,mySub) 
/declare charges int local 
/declare oldItemName string local 
/declare slotName string local 
/declare swapItemBack bool local false 
:cast_item 
/if (${FindItem[${spellName}].InvSlot}>22) { 
  /varset swapItemBack true 
/if (${FindItem[${spellName}].WornSlot[1]} && ${FindItem[${spellName}].EffectType.Find[worn]}) { 
    /varset slotName ${FindItem[${spellName}].WornSlot[1].Name} 
  } else /if (${FindItem[${spellName}].InvSlot}>30) { 
    /varset slotName pack8 
  } else { 
    /varset slotName ${FindItem[${spellName}].InvSlot.Name} 
  } 
  /varset oldItemName ${InvSlot[${slotName}].Item.Name} 
  /call SwapItem "${spellName}" ${slotName} 
} 
:wait_item_loop 
/if (${itemRefreshTime}>${MacroQuest.Running}) /goto :wait_item_loop 
/varset itemRefreshTime ${Math.Calc[${MacroQuest.Running}+000]} 
/varset charges ${FindItem[${spellName}].Charges} 
/cast item "${spellName}" 
/if (${Me.Casting.ID}) /varcalc castEndTime ${FindItem[${spellName}].CastTime}*10 
/if (${charges}) /delay 1s ${FindItem[${spellName}].Charges}!=${charges} 
/call WaitCast ${mySub} 
/if (${swapItemBack} && ${FindItem[${oldItemName}].ID}) /call SwapItem "${oldItemName}" ${slotName} 
/if (${castReturn.Equal[CAST_CANCELLED]}) /return CAST_CANCELLED 
/call DoCastingEvents 
/if (${castReturn.Equal[CAST_RESTART]} || ${castReturn.Equal[CAST_STUNNED]} || (${castReturn.Equal[CAST_INTERRUPTED]} && ${giveUpTimer}) || ${castReturn.Equal[CAST_COLLAPSE]} || (${castReturn.Equal[CAST_RESISTED]} && ${ResistCounter})) /goto :cast_item 
/return 

Sub AltCast(spellName,mySub) 
:cast_alt 
/alt activate ${Me.AltAbility[${spellName}].ID} 
/if (${Me.AltAbility[${spellName}].Spell.MyCastTime}>=0.5) /delay 1s ${Me.Casting.ID} 
/call WaitCast ${mySub} 
/if (${castReturn.Equal[CAST_CANCELLED]}) /return CAST_CANCELLED 
/call DoCastingEvents 
/if (${castReturn.Equal[CAST_RESTART]} || ${castReturn.Equal[CAST_STUNNED]} || (${castReturn.Equal[CAST_INTERRUPTED]} && ${giveUpTimer}) || (${castReturn.Equal[CAST_RESISTED]} && ${ResistCounter})) /goto :cast_alt 
/return 

Sub ClearCursor 
/declare i int local 
:auto_inv 
/if (${Cursor.ID}) { 
  /if (${Cursor.Container}) { 
    /for i 1 to 8 
    /if (!${InvSlot[pack${i}].Item.Container}) /nomodkey /itemnotify pack${i} leftmouseup 
    /next i 
  } else { 
    /autoinventory 
  } 
  /goto :auto_inv 
} 
/return 

Sub DoCastingEvents 
/doevents Recover 
/doevents BeginCast 
/doevents Fizzle 
/doevents Interrupt 
/doevents Standing 
/doevents FDFail 
/doevents OutOfRange 
/doevents OutOfMana 
/doevents NoLOS 
/doevents Resisted2 
/doevents Resisted 
/doevents Immune 
/doevents Stunned 
/doevents Collapse 
/doevents NoTarget 
/doevents NotReady 
/doevents NoHold 
/return 

Sub EquipItem(WhatWhere) 
/declare DestName string local 
/declare ItemName string local ${WhatWhere.Arg[1,|]} 
/declare SlotName string local ${WhatWhere.Arg[2,|]} 
/if (${SlotName.Equal[NULL]}) /varset SlotName ${InvSlot[${FindItem[=${ItemName}].WornSlot[1]}].Name} 
/if (${FindItem[=${ItemName}].InvSlot}<23 || !${FindItem[=${ItemName}].WornSlot[${SlotName}]}) /return 
/if (!${InvSlot[${SlotName}].Item.Name.Equal[NULL]}) /varset DestName "${InvSlot[${SlotName}].Item.Name}|${SlotName}" 
/call SwapItem "${ItemName}" "${SlotName}" 
/return ${DestName} 

Sub Interrupt 
/if (${Me.Mount.ID}) /dismount 
/stopcast 
/if (${Defined[castReturn]}) /varset castReturn CAST_CANCELLED 
/return ${castReturn} 

Sub SwapItem(itemName,slotName) 
/declare i int local 
/if (${Cursor.ID}) /call ClearCursor 
/exchange "${itemName}" ${slotName} 
/delay 5s ${InvSlot[${slotName}].Item.Name.Equal[${itemName}]} 
/if (${Cursor.ID}) /call ClearCursor 
/return 

Sub WaitCast(mySub,int spellID) 
/declare currentTarget int local ${Target.ID} 
/declare currentTargetType string local ${Target.Type} 
:wait_cast_loop 
/if (${Bool[${mySub}]}) /call ${mySub} ${spellID} 
/if (${Me.Casting.ID}) { 
  /if (${currentTarget} && !${Spawn[${currentTarget}].Type.Equal[${currentTargetType}]}) { 
    /if (!${Me.Casting.TargetType.Equal[PB AE]} && !${Me.Casting.TargetType.Equal[self]} && !${moveBack} && (!${Me.Mount.ID} || !${noInterrupt})) { 
      /if (!${Me.Mount.ID} || ${castEndTime}>70) { 
        /call Interrupt 
      } else /if (${Me.Casting.RecastTime}>3) { 
        /varset castReturn CAST_CANCELLED 
        /keypress forward hold 
        /delay 6 
        /keypress forward 
        /varset moveBack true 
      } 
    } 
  } 
  /if (${Me.State.Equal[DUCK]}) /varset castReturn CAST_CANCELLED 
  /goto :wait_cast_loop 
} 
/return 

Sub Event_BeginCast 
/if (${Defined[castReturn]}) /varset castReturn CAST_SUCCESS 
/return 

Sub Event_Collapse 
/if (${Defined[castReturn]}) /varset castReturn CAST_COLLAPSE 
/varset giveUpTimer 200 
/return 

Sub Event_FDFail(line,name) 
/if (${name.Equal[${Me.Name}]} && ${Defined[castReturn]}) { 
  /if (!${Me.Standing}) /stand 
  /varset castReturn CAST_RESTART 
} 
/return 

Sub Event_Fizzle 
/if (${Defined[castReturn]}) /varset castReturn CAST_FIZZLE 
/return 

Sub Event_Immune 
/if (${Defined[castReturn]}) /varset castReturn CAST_IMMUNE 
/return 

Sub Event_Interrupt 
/if (${Defined[castReturn]}) /varset castReturn CAST_INTERRUPTED 
/return 

Sub Event_NoHold 
/if (${Defined[spellNotHold]}) /varset spellNotHold 1 
/return 

Sub Event_NoLOS 
/if (${Defined[castReturn]}) /varset castReturn CAST_CANNOTSEE 
/return 

Sub Event_NoMount 
/if (${Defined[castReturn]}) /varset castReturn CAST_NOMOUNT 
/return CAST_NOMOUNT 

Sub Event_NoTarget 
/if (${Defined[castReturn]}) /varset castReturn CAST_NOTARGET 
/return 

Sub Event_NotReady 
/if (${Defined[castReturn]}) /varset castReturn CAST_NOTREADY 
/return 

Sub Event_OutOfMana 
/if (${Defined[castReturn]}) /varset castReturn CAST_OUTOFMANA 
/return 

Sub Event_OutOfRange 
/if (${Defined[castReturn]}) /varset castReturn CAST_OUTOFRANGE 
/return 

Sub Event_Recover 
/if (${Defined[castReturn]}) /varset castReturn CAST_RECOVER 
/return 

Sub Event_Resisted(line,name) 
/if (${selfResist} && ${name.Equal[${selfResistSpell}]}) /varset selfResist 0 
/if (${ResistCounter}) /varcalc ResistCounter ${ResistCounter}-1 
/if (${Defined[castReturn]}) /varset castReturn CAST_RESISTED 
/return 

Sub Event_Resisted2(line,name) 
/if (${Defined[selfResist]}) { 
  /varset selfResist 1 
  /varset selfResistSpell ${name} 
} 
/return 

Sub Event_Standing 
/stand 
/if (${Defined[castReturn]}) /varset castReturn CAST_RESTART 
/return 

Sub Event_Stunned 
/if (${Me.Stunned}) { 
  /delay 3s !${Me.Stunned} 
} else { 
  /delay 7 
} 
/if (${Defined[castReturn]}) /varset castReturn CAST_STUNNED 
/return 


