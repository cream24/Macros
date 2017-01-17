|------------------------------------------------------------------------|
|- e3_Classes_CLR.inc v3.0 ( originally designed to work with e3.mac )	-|
|- Written by Killians of PEQ											-|
|------------------------------------------------------------------------|
Sub BER_Setup
/return

Sub BER_Background_Events
/return

SUB BER_MacroSettings
/if (${Debug}) /echo |- BER_MacroSettings ==>
	/call WriteToIni "${advSettings_Ini},BER Functions,BER Function" "check_Burns" 1
	/call WriteToIni "${advSettings_Ini},BER Functions,BER Function" "check_Buffs" 1
/if (${Debug}) /echo <== BER_MacroSettings -|
/RETURN

SUB BER_CharacterSettings
/RETURN

Sub BER_Aliases
/return
