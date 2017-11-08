Sub alerts_Setup
/RETURN

SUB alerts_Background_Events
  /doevents charFlag

  /if (${Zone.ShortName.Equal[potactics]}) {
    /doevents tactics_stampede
  } else {
    /doevents tactics_stampede flush
  }

  /if (${Zone.ShortName.Equal[ikkinz]}) {
    /doevents setClass_Warrior
    /doevents setClass_Shaman
    /doevents setClass_Beastlord
    /doevents setClass_Necromancer
    /doevents setClass_Cleric
    /doevents setClass_Shadowknight
    /doevents setClass_Monk
    /doevents setClass_Bard
    /doevents setClass_Paladin
    /doevents setClass_Rogue
    /doevents setClass_Enchanter
    /doevents setClass_Wizard
    /doevents setClass_Berserker
    /doevents setClass_Magician
    /doevents setClass_Druid
    /doevents setClass_Ranger
    /doevents startClass_Priests
    /doevents startClass_Hybrids
    /doevents startClass_Casters
    /doevents startClass_Melees
    /doevents MobSet
  } else {
    /doevents setClass_Warrior flush
    /doevents setClass_Shaman flush
    /doevents setClass_Beastlord flush
    /doevents setClass_Necromancer flush
    /doevents setClass_Cleric flush
    /doevents setClass_Shadowknight flush
    /doevents setClass_Monk flush
    /doevents setClass_Bard flush
    /doevents setClass_Paladin flush
    /doevents setClass_Rogue flush
    /doevents setClass_Enchanter flush
    /doevents setClass_Wizard flush
    /doevents setClass_Berserker flush
    /doevents setClass_Magician flush
    /doevents setClass_Druid flush
    /doevents setClass_Ranger flush
    /doevents startClass_Priests flush
    /doevents startClass_Hybrids flush
    /doevents startClass_Casters flush
    /doevents startClass_Melees flush
  }
  /if (${Zone.ShortName.Equal[inktuta]}) {
    /doevents InktutaDeathtouch
  } else {
    /doevents InktutaDeathtouch flush
  }
  /if (${Zone.ShortName.Equal[uqua]}) {
    /doevents Define_UquaKey
  } else {
    /doevents Define_UquaKey flush
  }

  /if (${Zone.ShortName.Equal[chambersc]}) {
    /doevents goDuck
    /doevents dragornPBAE
    /doevents dragornBlast
    /doevents dragornThorn
    /doevents dragornReflect
  } else {
    /doevents goDuck flush
    /doevents dragornPBAE flush
    /doevents dragornBlast flush
    /doevents dragornThorn flush
    /doevents dragornReflect flush
  }

  /if (${Zone.ShortName.Equal[anguish]}) {
    /doevents angmask
  } else {
    /doevents angmask flush
  }
/RETURN

#event angmask "#*#You feel a gaze of deadly power focusing on you#*#"
Sub event_angmask
  /rs ~~~Mask on Me~~~ Ready: ${Me.ItemReady[Mirrored Mask]}
  /if (!${Bool[${FindItem[=Mirrored Mask]}]}) {
    /bc [+r+] I dont have a Mirrored Mask
    /return
  } else /if (${FindItem[=Mirrored Mask].ItemSlot} >=23 ) {
    /if (${Me.Casting.ID}) /call interrupt
    /delay 3s !${Me.Casting.ID}
    /declare OMM_Mask string local ${Me.Inventory[face].Name}
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${OMM_Mask}/face" 1
    /delay 3
    /echo calling swapitem
    /call SwapItem "Mirrored Mask" "face"
    /delay 5 ${Me.Inventory[face].Name.Equal[Mirrored Mask]}
    /call SwapItem "Mirrored Mask" "face"
    /delay 5 ${Me.Inventory[face].Name.Equal[Mirrored Mask]}
    /call SwapItem "Mirrored Mask" "face"
    /delay 5 ${Me.Inventory[face].Name.Equal[Mirrored Mask]}
  }
  /if (${Me.Inventory[face].Name.Equal[Mirrored Mask]}) {
    /declare numtries int local=0
    /if (${Me.Casting.ID}) /call interrupt
    /delay 3s !${Me.Casting.ID}
    :retry
    /varcalc numtries ${numtries}+1
    /casting "Mirrored Mask" -maxtries|3
    /delay 1s
    /if (!${Bool[${Me.Song[Reflective Skin]}]} && ${numtries} < 8) /goto :retry
  }
  |/if (${OMM_Mask.Length}) /call SwapItem "${OMM_Mask}" "face"
  /rs ~~~Mask on Me~~~ Ready: ${Me.ItemReady[Mirrored Mask]}
/return


#event tactics_stampede "#*#You hear the pounding of hooves#*#"
Sub event_tactics_stampede
	/if (${Zone.ShortName.Equal[potactics]}) {
		/gsay STAMPEDE!
	}
/return

#event charFlag "You receive a character flag#*#"
Sub event_charFlag
	/bc I have recieved a character flag!
/return

#event Define_UquaKey "#*#The #1# must unlock the door to the next room.#*#"
SUB event_Define_UquaKey(string line, string KeyToUse)
	/rs >>^<< The ${KeyToUse} unlocks the door >>^<<
	/popup >>^<< The ${KeyToUse} unlocks the door >>^<<
/return

#event setClass_Warrior "#*#Brute force and brawn#*#"
#event setClass_Shaman "#*#Cringes at the appearance of talismans#*#"
#event setClass_Beastlord "#*#Deep gashes of feral savagery#*#"
#event setClass_Necromancer "#*#Doom of death#*#"
#event setClass_Cleric "#*#Dread of celestial spirit#*#"
#event setClass_Shadowknight "#*#It appears that this creature dreads the strike of death#*#"
#event setClass_Monk "#*#Focused tranquility#*#"
#event setClass_Bard "#*#Foreboding melody#*#"
#event setClass_Paladin "#*#fears a holy blade#*#"
#event setClass_Rogue "#*#Ignores anything behind it#*#"
#event setClass_Enchanter "#*#Mind and body are vulnerable#*#"
#event setClass_Wizard "#*#Falters when struck with the power of the elements#*#"
#event setClass_Berserker "#*#Shies from heavy blades#*#"
#event setClass_Magician "#*#Summoned elements#*#"
#event setClass_Druid "#*#The creature seems weak in the face of the power of nature#*#"
#event setClass_Ranger "#*#True shots and fast blades#*#"

SUB event_setClass_Warrior
  /popup Spawn must be killed by a >>^<< WARRIOR >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< WARRIOR >>^<<
/return

SUB event_setClass_Shaman
	/popup Spawn must be killed by a >>^<< SHAMAN >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< SHAMAN >>^<<
/return

SUB event_setClass_Beastlord
	/popup Spawn must be killed by a >>^<< BEASTLORD >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< BEASTLORD >>^<<
/return

SUB event_setClass_Necromancer
	/popup Spawn must be killed by a >>^<< NECROMANCER >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< NECROMANCER >>^<<
/return

SUB event_setClass_Cleric
	/popup Spawn must be killed by a >>^<< CLERIC >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< CLERIC >>^<<
/return

SUB event_setClass_Shadowknight
	/popup Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<<
/return

SUB event_setClass_Monk
	/popup Spawn must be killed by a >>^<< MONK >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< MONK >>^<<
/return

SUB event_setClass_Bard
	/popup Spawn must be killed by a >>^<< BARD >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< BARD >>^<<
/return

SUB event_setClass_Paladin
  /popup Spawn must be killed by a >>^<< PALADIN >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< PALADIN >>^<<
/return

SUB event_setClass_Rogue
	/popup Spawn must be killed by a >>^<< ROGUE >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< ROGUE >>^<<
/return

SUB event_setClass_Enchanter
	/popup Spawn must be killed by a >>^<< ENCHANTER >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< ENCHANTER >>^<<
/return

SUB event_setClass_Wizard
	/popup Spawn must be killed by a >>^<< WIZARD >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< WIZARD >>^<<
/return

SUB event_setClass_Berserker
	/popup Spawn must be killed by a >>^<< BERSERKER >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< BERSERKER >>^<<
/return

SUB event_setClass_Magician
	/popup Spawn must be killed by a >>^<< MAGICIAN >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< MAGICIAN >>^<<
/return

SUB event_setClass_Druid
	/popup Spawn must be killed by a >>^<< DRUID >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< DRUID >>^<<
/return
SUB event_setClass_Ranger
	/popup Spawn must be killed by a >>^<< RANGER >>^<<
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< RANGER >>^<<
/return

#event startClass_Priests "#*#The creature cannot stand up to the power of healers#*#"
#event startClass_Hybrids "#*#The creature appears weak to the combined effort of might and magic#*#"
#event startClass_Casters "#*#The creature will perish under the strength of intelligent magic#*#"
#event startClass_Melees "#*#The creature appears weak to the combined effort of strength and cunning#*#"
SUB event_startClass_Priests
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< PRIEST >>^<<
/return

SUB event_startClass_Hybrids
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< HYBRID >>^<<
/return

SUB event_startClass_Casters
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< CASTER >>^<<
/return

SUB event_startClass_Melees
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs Spawn must be killed by a >>^<< MELEE >>^<<
/return

#event InktutaDeathtouch "#*#thoughts of a cursed trusik invade your mind#*#"
SUB event_InktutaDeathtouch
  /rs I, >>^<< ${Me.Name} >>^<<, who am about to die, salute you!!
/return

#event goDuck "#*#From the corner of your eye, you notice a Kyv taking aim at your head. You should duck."
#event dragornPBAE "#*#You notice that the Dragorn before you is preparing to cast a devastating close-range spell."
#event dragornBlast "You notice that the Dragorn before you is making preparations to cast a devastating spell.  Doing enough damage to him might interrupt the process."
#event dragornThorn "#*#The Dragorn before you is sprouting sharp spikes."
#event dragornReflect "#*#The Dragorn before you is developing an anti-magic aura."

Sub Event_dragornPBAE
  /popup Dragorn PBAE Inc
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs @@@ PBAE INC @@@
/return

Sub Event_dragornBlast
  /popup Dragorn Blast Inc
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs @@@ BLAST INC @@@
/return

Sub Event_dragornThorn
  /popup Dragorn Thorns Inc
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs ^^^ Thorns ON ^^^
/return

Sub Event_dragornReflect
  /popup Dragorn Reflect Inc
  /if (${Raid.Leader.Name.Equal[${Me}]}) /rs ~~~ Reflect ON ~~~
/return

Sub Event_goDuck
  /echo in goduck
  /call AssistOff
  /declare duckchecks int local 0
  /declare pcthp int local ${Me.PctHPs}
  :checkDuck
  /varcalc duckchecks ${duckchecks}+1
  /if (!${Me.Ducking}) /keypress duck
  /delay 3
  /if (${duckchecks} < 11 ) /goto :checkDuck

  /if (${Me.Ducking}) /keypress duck
  /if (${Me.PctHPs} < ${pcthp}) /echo looks like i failed Event_goDuck: origHP ${pcthp} currHP ${Me.PctHPs}
/return

SUB alerts_CharacterSettings
/return
Sub alerts_Aliases
/return
Sub alerts_MacroSettings
/return