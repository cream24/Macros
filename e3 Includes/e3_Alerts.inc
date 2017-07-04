|----------------------------------------------------------------|
|- GoD_Alerts.inc ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ									-|
|----------------------------------------------------------------|
|- Contains various alerts.										-|
|----------------------------------------------------------------|
Sub alerts_Setup
	/declare roomFlag bool outer FALSE
	/declare kyvFlag bool outer FALSE
	/declare weaponFlag bool outer FALSE
	/declare ringFlag bool outer FALSE
  /declare maxForesightChecks int outer 50
  /declare numForesightChecks int outer 0
  /declare offhandWeapForesight string outer
  /declare ringLeftForesight string outer
  /declare ringRightForesight string outer

	/declare northLoc string outer 392,-210
	/declare southLoc string outer 159,-210
	/declare eastLoc string outer 272,-320
	/declare westLoc string outer 272,-88
	/declare centerLoc string outer 275,-200
	/declare pbaeloc string outer 300,-265

/RETURN

SUB alerts_Background_Events
  /doevents tactics_stampede
  /doevents charFlag
  /doevents Define_UquaKey
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
  /doevents InktutaDeathtouch
  /doevents GotExp

  /doevents swapNoBlunt
	/doevents swapNoSlash
	/doevents swapNoPierce
	/doevents swapNoH2H

|/echo indoevents
  /doevents goDuck
	/doevents goNorth
  /doevents goSouth
  /doevents goEast
  /doevents goWest
  /doevents goCenter
  /doevents goStandstill
  /doevents goMove
  /doevents goWeapon
  /doevents goRings
  /doevents dragornPBAE
  /doevents dragornBlast
  /doevents dragornThorn
  /doevents dragornReflect
  /doevents angmask
/RETURN

#event angmask "#*#You feel a gaze of deadly power focusing on you#*#"
Sub event_angmask
  /rs ~~~Mask on Me~~~

  /if (!${Bool[${FindItem[=Mirrored Mask]}]}) {
    /bc [+r+] I dont have a Mirrored Mask
    /return
  } else /if (${FindItem[=Mirrored Mask].ItemSlot} >=23 ) {
    /declare OMM_Mask string local ${Me.Inventory[face].Name}
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1" "${OMM_Mask}/face" 1
    /call SwapItem "Mirrored Mask" "face"
    /delay 3s ${Me.Inventory[face].Name.Equal[Mirrored Mask]}
  }
  /if (${Me.Inventory[face].Name.Equal[Mirrored Mask]}) {
    /declare numtries int local=0
    /if (${Me.Casting.ID}) /call interrupt
    :retry
    /varcalc numtries ${numtries}+1
    /casting "Mirrored Mask" -maxtries|3
    /delay 1s
    /if (!${Bool[${Me.Song[Reflective Skin]}]} && ${numtries} < 5) /goto :retry
    /if (${OMM_Mask.Length}) /call SwapItem "${OMM_Mask}" "face"
  }
  /rs ~~~Mask on Me~~~ ${Bool[${Me.Song[Reflective Skin]}]}
/return

|#event swapNoPierce "#*#You are affected by imprecision#*#"
|#event swapNoBlunt "#*#Your movements become rigid#*#"
|#event swapNoSlash "#*#You feel very unlucky#*#"
|#event swapNoH2H "#*#You lose your coordination#*#"

|no fire
SUB event_swapNoPierce
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotPierce
  } else /if (${Select[${Me.Class.ShortName},ENC]}) {
    /assistoff /only|me
  } else {
    |}/if (${Select[${Me.Class.ShortName},WIZ,MAG]}) {
    | /if (${Me.Casting.ID}) /call interrupt
    /delay 2
    /echo nukeset fastcold
}
/return

|no cold
SUB event_swapNoBlunt
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotBlunt
    /delay 2
    /echo nukeset fastfire
  |} else /if (${Select[${Me.Class.ShortName},WIZ,MAG,ENC]}) {
  } else {
    /if (${Me.Casting.ID}) /call interrupt
    /delay 2
    /echo nukeset fastfire
  }
/return

|no magic
SUB event_swapNoSlash
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotSlash
  |} else /if (${Select[${Me.Class.ShortName},ENC,WIZ]}) {
  } else {
    /if (${Me.Casting.ID}) /call interrupt
    /delay 2
    /echo nukeset fastfire
  }
/return

|no poison/disease
SUB event_swapNoH2H
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotH2H
  } else /if (${Select[${Me.Class.ShortName},WIZ,MAG,ENC]}) {
    /assistoff /only|me
  }
/return


#event tactics_stampede "#*#You hear the pounding of hooves#*#"
Sub event_tactics_stampede
	/if (${Zone.ShortName.Equal[potactics]}) {
		/gsay STAMPEDE!
		/beep
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
#event setClass_Shadowknight "#*#Dreads the strike of dead#*#"
#event setClass_Monk "#*#Focused tranquility#*#"
#event setClass_Bard "#*#Foreboding melody#*#"
#event setClass_Paladin "#*#Holy blade#*#"
#event setClass_Rogue "#*#Ignores anything behind it#*#"
#event setClass_Enchanter "#*#Mind and body vulnerable#*#"
#event setClass_Wizard "#*#Falters when struck with the power of the elements#*#"
#event setClass_Berserker "#*#Shies from heavy blades#*#"
#event setClass_Magician "#*#Summoned elements#*#"
#event setClass_Druid "#*#The creature seems weak in the face of the power of nature#*#"
#event setClass_Ranger "#*#True shots and fast blades#*#"

SUB event_setClass_Warrior
	/popup Spawn must be killed by a >>^<< WARRIOR >>^<<
	/rs Spawn must be killed by a >>^<< WARRIOR >>^<<
/return

SUB event_setClass_Shaman
	/popup Spawn must be killed by a >>^<< SHAMAN >>^<< 
	/rs Spawn must be killed by a >>^<< SHAMAN >>^<< 
/return

SUB event_setClass_Beastlord
	/popup Spawn must be killed by a >>^<< BEASTLORD >>^<< 
	/rs Spawn must be killed by a >>^<< BEASTLORD >>^<< 
/return

SUB event_setClass_Necromancer
	/popup Spawn must be killed by a >>^<< NECROMANCER >>^<< 
	/rs Spawn must be killed by a >>^<< NECROMANCER >>^<< 
/return

SUB event_setClass_Cleric
	/popup Spawn must be killed by a >>^<< CLERIC >>^<< 
	/rs Spawn must be killed by a >>^<< CLERIC >>^<< 
/return

SUB event_setClass_Shadowknight
	/popup Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<< 
	/rs Spawn must be killed by a >>^<< SHADOWKNIGHT >>^<< 
/return

SUB event_setClass_Monk
	/popup Spawn must be killed by a >>^<< MONK >>^<< 
	/rs Spawn must be killed by a >>^<< MONK >>^<< 
/return

SUB event_setClass_Bard
	/popup Spawn must be killed by a >>^<< BARD >>^<< 
	/rs Spawn must be killed by a >>^<< BARD >>^<< 
/return

SUB event_setClass_Paladin
	/if (${Zone.ShortName.Equal[ikkinz]}) {
	/popup Spawn must be killed by a >>^<< PALADIN >>^<< 
	/rs Spawn must be killed by a >>^<< PALADIN >>^<< 
	}
/return

SUB event_setClass_Rogue
	/popup Spawn must be killed by a >>^<< ROGUE >>^<< 
	/rs Spawn must be killed by a >>^<< ROGUE >>^<< 
/return

SUB event_setClass_Enchanter
	/popup Spawn must be killed by a >>^<< ENCHANTER >>^<< 
	/rs Spawn must be killed by a >>^<< ENCHANTER >>^<< 
/return

SUB event_setClass_Wizard
	/popup Spawn must be killed by a >>^<< WIZARD >>^<< 
	/rs Spawn must be killed by a >>^<< WIZARD >>^<< 
/return

SUB event_setClass_Berserker
	/popup Spawn must be killed by a >>^<< BERSERKER >>^<< 
	/rs Spawn must be killed by a >>^<< BERSERKER >>^<< 
/return

SUB event_setClass_Magician
	/popup Spawn must be killed by a >>^<< MAGICIAN >>^<< 
	/rs Spawn must be killed by a >>^<< MAGICIAN >>^<< 
/return

SUB event_setClass_Druid
	/popup Spawn must be killed by a >>^<< DRUID >>^<< 
	/rs Spawn must be killed by a >>^<< DRUID >>^<< 
/return

SUB event_setClass_Ranger
	/popup Spawn must be killed by a >>^<< RANGER >>^<< 
	/rs Spawn must be killed by a >>^<< RANGER >>^<< 
/return

#event startClass_Priests "#*#The creature cannot stand up to the power of healers#*#"
#event startClass_Hybrids "#*#The creature appears weak to the combined effort of might and magic#*#"
#event startClass_Casters "#*#The creature will perish under the strength of intelligent magic#*#"
#event startClass_Melees "#*#The creature appears weak to the combined effort of strength and cunning#*#"

SUB event_startClass_Priests
	/if (!${Defined[murauderKill]}) {
		/declare murauderKill int outer 4
	} else /if (${Defined[murauderKill]}) {
		/varset murauderKill 4
	}
	/rs Spawn must be killed by a >>^<< PRIEST >>^<< 
/return

SUB event_startClass_Hybrids
	/if (!${Defined[murauderKill]}) {
		/declare murauderKill int outer 2
	} else /if (${Defined[murauderKill]}) {
		/varset murauderKill 2
	}
	/rs Spawn must be killed by a >>^<< HYBRID >>^<< 
/return

SUB event_startClass_Casters
	/if (!${Defined[murauderKill]}) {
		/declare murauderKill int outer 1
	} else /if (${Defined[murauderKill]}) {
		/varset murauderKill 1
	}
	/rs Spawn must be killed by a >>^<< CASTER >>^<< 
/return

SUB event_startClass_Melees
	/if (!${Defined[murauderKill]}) {
		/declare murauderKill int outer 3
	} else /if (${Defined[murauderKill]}) {
		/varset murauderKill 3
	}
	/rs Spawn must be killed by a >>^<< MELEE >>^<< 
/return

#event MobSet "#*#shudders as if somehow changing.#*#"
SUB event_MobSet
	/if (${murauderKill} < 4) {
		/if (${Defined[murauderKill]}) /varcalc murauderKill ${murauderKill} + 1
	} else {
		/if (${Defined[murauderKill]}) /varset murauderKill 1
	}
	/if (${murauderKill} == 1) {
		/rs Vrex Xalkaks Marauder must be killed by a  >>^<< CASTER >>^<< 
	} else /if (${murauderKill} == 2) {
		/rs Vrex Xalkaks Marauder must be killed by a >>^<< HYBRID >>^<< 
	} else /if (${murauderKill} == 3) {
		/rs Vrex Xalkaks Marauder must be killed by a >>^<< PRIEST >>^<< 
	} else /if (${murauderKill} == 4) {
		/rs Vrex Xalkaks Marauder must be killed by a >>^<< MELEE >>^<< 
	}
/return


#event InktutaDeathtouch "#*#thoughts of a cursed trusik invade your mind#*#"
SUB event_InktutaDeathtouch
 /if (${Zone.ID} != 296) /return
 /rs I, >>^<< ${Me.Name} >>^<<, who am about to die, salute you!!
/return

#event GotExp "You gain experience!!"
#event GotExp "You gain party experience!!"
SUB event_GotExp
/if (${Group}>1 && ${Me.Name.Equal[${Group.Leader}]}) {
  /bc [+g+] My group got Exp
} else /if (!${Group}) {
	/bc [+y+] I got solo Exp
}
/return


|#event goNorth "#*#The north side looks safe.#*#"
|#event goSouth "#*#The south side looks safe.#*#"
|#event goEast "#*#The east side looks safe.#*#"
|#event goWest "#*#The west side looks safe.#*#"
|#event goCenter "#*#The center looks safe.#*#"
#event goDuck "#*#From the corner of your eye, you notice a Kyv taking aim at your head. You should duck."
|#event goStandstill "From the corner of your eye, you notice a Kyv taking aim near your position. He appears to be leading the target, anticipating your next movement. You should stand still."
|#event goMove "From the corner of your eye, you notice a Kyv taking aim at your position. You should move."
#event goWeapon "#*#hand begins to heat up dramatically. You should remove it."
#event goRings "#*#Your rings constrict and bite into your fingers#*#"

|#event dragornPBAE "#*#You notice that the Dragorn before you is preparing to cast a devastating close-range spell."
|#event dragornBlast "You notice that the Dragorn before you is making preparations to cast a devastating spell.  Doing enough damage to him might interrupt the process."
|#event dragornThorn "#*#The Dragorn before you is sprouting sharp spikes."
|#event dragornReflect "#*#The Dragorn before you is developing an anti-magic aura."

Sub Event_dragornPBAE
  /popup Dragorn PBAE Inc
  /if (${Me.Class.ShortName.Equal[War]}) /gu @@@ PBAE INC @@@
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo in dragomove
    /if (${Me.Feigning}) /stand
    /call AssistOff
    /call MoveTo -Loc|${pbaeloc}
  }
/return

Sub Event_dragornBlast
  /popup Dragorn Blast Inc
  /if (${Me.Class.ShortName.Equal[War]}) /gu @@@ BLAST INC @@@
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /if (${Me.Feigning}) /stand
    /call AssistOff
    /call MoveTo -Loc|${pbaeloc}
  }
/return

Sub Event_dragornThorn
  /popup Dragorn Thorns Inc
  /if (${Me.Class.ShortName.Equal[War]}) /gu ^^^ Thorns ON ^^^
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /call AssistOff
    /call MoveTo -Loc|${pbaeloc}
  }
/return

Sub Event_dragornReflect
  /popup Dragorn Reflect Inc
  /if (${Me.Class.ShortName.Equal[War]}) /gu ~~~ Reflect ON ~~~
  /if (${Select[${Me.Class.ShortName},ENC,MAG,WIZ,NEC]}) {
    /call AssistOff
  }
/return

|#event stopRoom "#*#The room explodes with chaotic energy.#*#"
|#event stopRoom "#*#You escape the blast.#*#"
|#event stopKyv "#*#arrow narrowly misses#*#"
|#event stopWeapon "#*#Your weaponry cools down.#*#"
|#event stopWeapon "#*#Your weaponry becomes incredibly hot, searing your hands!#*#"
|#event stopRings "#*#Your rings clamp down#*#"
|#event stopRings "#*#Your regain the use of your fingers#*#"

|---------------- Emote Trigger ------------------------|
Sub Event_goNorth
  /echo in gonorth
  /call AssistOff
  /if (${Me.Feigning}) /stand
  /declare pcthp int local ${Me.PctHPs}
  /if (${Me.Class.ShortName.Equal[War]}) /gu Moving to North
  /call MoveTo -Loc|${northLoc}
  /delay 2s
  /if (!${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
    /delay 2s
    /call MoveTo -Loc|${northLoc}
  }
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goNorth: origHP ${pcthp} currHP ${Me.PctHPs}
/return

Sub Event_goSouth
  /echo in gosouth
  /call AssistOff
  /if (${Me.Feigning}) /stand
  /declare pcthp int local ${Me.PctHPs}
  /if (${Me.Class.ShortName.Equal[War]}) /gu Moving to South
  /call MoveTo -Loc|${southLoc}
  /delay 2s
  /if (!${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
    /delay 2s
    /call MoveTo -Loc|${southLoc}
  }
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goSouth: origHP ${pcthp} currHP ${Me.PctHPs}
/return

Sub Event_goEast
  /echo in goeast
  /call AssistOff
  /if (${Me.Feigning}) /stand
  /declare pcthp int local ${Me.PctHPs}
  /if (${Me.Class.ShortName.Equal[War]}) /gu Moving to East
  /call MoveTo -Loc|${eastLoc}
  /delay 2s
  /if (!${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
    /delay 2s
    /call MoveTo -Loc|${eastLoc}
  }
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goEast: origHP ${pcthp} currHP ${Me.PctHPs}
/return

Sub Event_goWest
  /echo in gowest
  /call AssistOff
  /if (${Me.Feigning}) /stand
  /declare pcthp int local ${Me.PctHPs}
  /if (${Me.Class.ShortName.Equal[War]}) /gu Moving to West
  /call MoveTo -Loc|${westLoc}
  /delay 2s
  /if (!${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
    /delay 2s
    /call MoveTo -Loc|${westLoc}
  }
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goWest: origHP ${pcthp} currHP ${Me.PctHPs}
/return

Sub Event_goCenter
  /echo in gocenter
  /call AssistOff
  /if (${Me.Feigning}) /stand
  /declare pcthp int local ${Me.PctHPs}
  /if (${Me.Class.ShortName.Equal[War]}) /gu Moving to Center
  /call MoveTo -Loc|${centerLoc}
  /delay 2s
  /if (!${Select[${Me.Class.ShortName},CLR,DRU,SHM]}) {
    /delay 2s
    /call MoveTo -Loc|${centerLoc}
  }
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goCenter: origHP ${pcthp} currHP ${Me.PctHPs}
/return

| kyv stuff
Sub Event_goMove
  /echo in gomove
  /if (${Me.Feigning}) /stand
  /call AssistOff
  /declare pcthp int local ${Me.PctHPs}
  /call MoveTo -Loc|${pbaeloc}
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goMove: origHP ${pcthp} currHP ${Me.PctHPs}
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
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goDuck: origHP ${pcthp} currHP ${Me.PctHPs}
/return

Sub Event_goStandstill
  /echo in gostandstill
  /call AssistOff
  /declare pcthp int local ${Me.PctHPs}
  /gu KyvStand
  /if (${NetAdvPath.Status}>0) /netfollow pause
  /if (${Stick.Active}) /stick pause
  /delay 3s
  /if (${NetAdvPath.Status}>0) /netfollow unpause
  /if (${Stick.Active}) /stick unpause
  /if (${Me.PctHPs} < ${pcthp}) /gu looks like i failed Event_goStandstill: origHP ${pcthp} currHP ${Me.PctHPs}
/return

|-------------------------------------------------------------------
|equipments
Sub Event_goWeapon
  /if (${Bool[${Me.Inventory[offhand].Name}]}) {
    /varset weaponFlag TRUE
    /echo GoWeapon weaponFlag ${weaponFlag}
    /varset maxForesightChecks 0
    /varset offhandWeapForesight ${Me.Inventory[offhand].Name}
    /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange" "${offhandWeapForesight}/offhand" 1
    /delay 2
    /itemnotify offhand leftmouseup
    /delay 2
    /call PlaceInFirstFreeSlot
    /delay 2s
    /call SwapItem "${offhandWeapForesight}" offhand
  }
/return

Sub Event_goRings
  /if (${Bool[${Me.Inventory[leftfinger].Name}]} || ${Bool[${Me.Inventory[rightfinger].Name}]}) {
    /echo GoRings
    /if (${Bool[${Me.Inventory[leftfinger].Name}]}) {
      /varset ringLeftForesight ${Me.Inventory[leftfinger].Name}
      /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#1" "${ringLeftForesight}/leftfinger" 1
      /delay 1
      /itemnotify leftfinger leftmouseup
      /call PlaceInFirstFreeSlot
    }
    /if (${Bool[${Me.Inventory[rightfinger].Name}]}) {
      /varset ringRightForesight ${Me.Inventory[rightfinger].Name}
      /call WriteToIni "${MacroData_Ini},${Me.CleanName}-${MacroQuest.Server},Pending Exchange#2" "${ringRightForesight}/rightfinger" 1
      /delay 1
      /itemnotify rightfinger leftmouseup
      /call PlaceInFirstFreeSlot
    }
   | /delay 2s
   | /call SwapItem "${ringLeftForesight}" leftfinger
   | /delay 2
   | /call SwapItem "${ringRightForesight}" rightfinger
   |/delay 2
  }
/return


  |dragorns = {
  |[1] = { "You notice that the Dragorn before you is preparing to cast a devastating close-range spell.", 5693, cast_5693 },
  |[2] = { "The Dragorn before you is developing an anti-magic aura.", 5699, self_cast },
  |[3] = { "The Dragorn before you is sprouting sharp spikes.", 5698, self_cast },

SUB alerts_CharacterSettings
/return
Sub alerts_Aliases
/return
Sub alerts_MacroSettings
/return