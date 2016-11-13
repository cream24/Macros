|----------------------------------------------------------------|
|- GoD_Alerts.inc ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ									-|
|----------------------------------------------------------------|
|- Contains various alerts.										-|
|----------------------------------------------------------------|

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
/RETURN

#event swapNoPierce "#*#You are affected by imprecision#*#"
|no fire
SUB event_swapNoPierce
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotPierce
  } else /if (${Select[${Me.Class.ShortName},WIZ,MAG]}) {
    /if (${Me.Casting.ID}) /call interrupt
    /assistoff /only|me
  } else /if (${Select[${Me.Class.ShortName},ENC]}) {
    /assistoff /only|me
}
/return

#event swapNoBlunt "#*#Your movements become rigid#*#"
|no cold
SUB event_swapNoBlunt
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotBlunt
  } else /if (${Select[${Me.Class.ShortName},WIZ,MAG,ENC]}) {
    /assiston /only|me
  }
/return

#event swapNoSlash "#*#You feel very unlucky#*#"
|no magic
SUB event_swapNoSlash
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotSlash
  } else /if (${Select[${Me.Class.ShortName},ENC,WIZ]}) {
    /if (${Me.Casting.ID}) /call interrupt
    /assistoff /only|me
  }
/return

#event swapNoH2H "#*#You lose your coordination#*#"
|no poison/disease
SUB event_swapNoH2H
  /if (${Select[${Me.Class.ShortName},BRD,BER,BST,MNK,PAL,RNG,ROG,SHD,WAR]}) {
    /echo swap NotH2H
  } else /if (${Select[${Me.Class.ShortName},WIZ,MAG,ENC]}) {
    /assiston /only|me
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
	/bc My group got Exp
} else /if (!${Group}) {
	/bc I got solo Exp
}
/return