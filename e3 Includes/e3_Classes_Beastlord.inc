|----------------------------------------------------------------------------|
|- e3_Classes_Beastlord.inc v3.0 ( originally designed to work with e3.mac )-|
|- Written by Killians of PEQ												-|
|----------------------------------------------------------------------------|
Sub BST_Setup
/return

|----------------------------------------------------------------------------|
SUB BST_MacroSettings
/if (${Debug}) /echo |- BST_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#1" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#2" "check_Debuffs" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#3" "check_Heals" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#4" "check_Cures" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#5" "check_Buffs" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#6" "check_Nukes" 1
	/call WriteToIni "${advSettings_Ini},BST Functions,BST Function#7" "check_Pets" 1
  /call WriteToIni "${advSettings_Ini},BST Functions,BST Function#8" "check_DoTs" 1
/if (${Debug}) /echo <== BST_MacroSettings -|
/RETURN

SUB BST_CharacterSettings
/RETURN

Sub BST_Aliases
/return

|----------------------------------------------------------------------------|
#event AE_POS "#*# tells you, 'AEPOS'"
sub Event_AE_POS(string line)
   /if (${Me.AltAbilityReady[Mass Group Buff]} && ${Me.AltAbilityReady[Paragon of Spirit]}) {
      /bc MGB Paragon of Spirit inc...
      /rs MGB Paragon of Spirit inc...
      /casting "Mass Group Buff|alt" "-maxtries|3"
      /delay 5
      /casting "Paragon of Spirit|alt" "-maxtries|3"
   } else /if (!${Me.AltAbilityReady[Mass Group Buff]}) {
      /bc Mass Group Buff is not available...
   } else /bc Paragon of Spirit is not available...
/return

|----------------------------------------------------------------------------|
Sub BST_Background_Events
  /doevents AE_POS
/return