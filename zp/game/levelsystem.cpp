/**
 * ============================================================================
 *
 *  Zombie Plague Mod #3 Generation
 *
 *  File:          levelsystem.cpp
 *  Type:          Game 
 *  Description:   Provides functions for level system.
 *
 *  Copyright (C) 2015-2018 Nikita Ushakov (Ireland, Dublin)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 **/
 
/**
 * Number of max valid levels.
 **/
#define LevelSystemMax 100

/**
 * Arrays to store the level's data.
 **/
int  LevelSystemNum;
static char LevelSystemStats[LevelSystemMax][SMALL_LINE_LENGTH];
 
/**
 * HUD synchronization hadnle.
 **/
Handle hHudLevel;
 
/**
 * Level system module init function.
 **/
void LevelSystemInit(/*void*/)
{
    /*
     * Creates a HUD synchronization object. This object is used to automatically assign and re-use channels for a set of messages.
     * The HUD has a hardcoded number of channels (usually 6) for displaying text. You can use any channel for any area of the screen. 
     * Text on different channels can overlap, but text on the same channel will erase the old text first. This overlapping and overwriting gets problematic.
     * A HUD synchronization object automatically selects channels for you based on the following heuristics: - If channel X was last used by the object, 
     * and hasn't been modified again, channel X gets re-used. - Otherwise, a new channel is chosen based on the least-recently-used channel.
     * This ensures that if you display text on a sync object, that the previous text displayed on it will always be cleared first. 
     * This is because your new text will either overwrite the old text on the same channel, or because another channel has already erased your text.
     * Note that messages can still overlap if they are on different synchronization objects, or they are displayed to manual channels.
     * These are particularly useful for displaying repeating or refreshing HUD text, in addition to displaying multiple message sets in one area of the screen 
     * (for example, center-say messages that may pop up randomly that you don't want to overlap each other).
     */
    hHudLevel = CreateHudSynchronizer();
}

/**
 * Prepare all level data.
 **/
void LevelSystemLoad(/*void*/)
{
    // Resets level's data
    LevelSystemNum = 0;

    // If level system disabled, then stop
    if(!gCvarList[CVAR_LEVEL_SYSTEM].BoolValue)
    {
        return;
    }

    // Initialize level list
    static char sList[PLATFORM_MAX_PATH];
    gCvarList[CVAR_LEVEL_STATISTICS].GetString(sList, sizeof(sList));

    // Check if list is empty, then skip
    if(strlen(sList))
    {
        // Convert list string to pieces
        LevelSystemNum = ExplodeString(sList, " ", LevelSystemStats, sizeof(LevelSystemStats), sizeof(LevelSystemStats[])) - 1;
    }
}

/**
 * Client has been changed class state.
 *
 * @param clientIndex       The client index.
 **/
void LevelSystemOnClientUpdate(int clientIndex)
{
    // If level system disabled, then stop
    if(!gCvarList[CVAR_LEVEL_SYSTEM].BoolValue)
    {
        return;
    }
    
    // Validate level amount
    if(!LevelSystemNum)
    {
        return;
    }

    // Validate real client
    if(!IsFakeClient(clientIndex))
    {
        // Sets timer for player level HUD
        delete gClientData[clientIndex][Client_LevelTimer];
        gClientData[clientIndex][Client_LevelTimer] = CreateTimer(1.0, LevelSystemOnHUD, GetClientUserId(clientIndex), TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
    }
}

/**
 * Sets the client's level and prevent it from overloading.
 *
 * @param clientIndex       The client index.
 * @param nLevel            The level amount.
 **/
void LevelSystemOnSetLvl(int clientIndex, int nLevel)
{
    // If level system disabled, then stop
    if(!gCvarList[CVAR_LEVEL_SYSTEM].BoolValue)
    {
        return;
    }

    // Validate level amount
    if(!LevelSystemNum)
    {
        return;
    }

    // If amount below 0, then stop
    if(nLevel < 0)
    {
        return;
    }

    // Sets the level
    gClientData[clientIndex][Client_Level] = nLevel;

    // Validate level
    if(gClientData[clientIndex][Client_Level] > LevelSystemNum)
    {
        // Update it
        gClientData[clientIndex][Client_Level] = LevelSystemNum;
    }
    else
    {
        // Emit levelup sound
        if(IsPlayerExist(clientIndex)) SoundsInputEmitToClient(clientIndex, SNDCHAN_STATIC, gCvarList[CVAR_GAME_CUSTOM_SOUND_LEVEL].IntValue, "LEVEL_UP_SOUNDS");
    }
}

/**
 * Sets the client's experience, increasing level if it reach level's experience limit and prevent it from overloading.
 *
 * @param clientIndex       The client index.
 * @param nExperience       The experience amount.
 **/
void LevelSystemOnSetExp(int clientIndex, int nExperience)
{
    // If level system disabled, then stop
    if(!gCvarList[CVAR_LEVEL_SYSTEM].BoolValue)
    {
        return;
    }

    // Validate level amount
    if(!LevelSystemNum)
    {
        return;
    }

    // If amount below 0, then stop
    if(nExperience < 0)
    {
        return;
    }

    // Sets the experience
    gClientData[clientIndex][Client_Exp] = nExperience;

    // Give experience to the player
    if(gClientData[clientIndex][Client_Level] == LevelSystemNum && gClientData[clientIndex][Client_Exp] > StringToInt(LevelSystemStats[gClientData[clientIndex][Client_Level]]))
    {
        gClientData[clientIndex][Client_Exp] = StringToInt(LevelSystemStats[gClientData[clientIndex][Client_Level]]);
    }
    else
    {
        // Loop throught experience
        while(gClientData[clientIndex][Client_Level] < LevelSystemNum && gClientData[clientIndex][Client_Exp] >= StringToInt(LevelSystemStats[gClientData[clientIndex][Client_Level]]))
        {
            // Increase level
            LevelSystemOnSetLvl(clientIndex, gClientData[clientIndex][Client_Level] + 1);
        }
    }
}

/**
 * Main timer for show HUD text within information about client's level and experience.
 *
 * @param hTimer            The timer handle.
 * @param userID            The user id.
 **/
public Action LevelSystemOnHUD(Handle hTimer, int userID)
{
    // Gets the client index from the user ID
    int clientIndex = GetClientOfUserId(userID);

    // Validate client
    if(clientIndex)
    {
        // Initialize variables
        static char sInfo[SMALL_LINE_LENGTH]; static int iRed, iGreen, iBlue;
    
        // Validate zombie hud
        if(gClientData[clientIndex][Client_Zombie])
        {
            // Validate nemesis hud
            if(gClientData[clientIndex][Client_Nemesis])
            {
                strcopy(sInfo, sizeof(sInfo), "Nemesis");
            }
            else
            {
                // Gets zombie name
                ZombieGetName(gClientData[clientIndex][Client_ZombieClass], sInfo, sizeof(sInfo));
            }
            
            // Gets colors 
            iRed = gCvarList[CVAR_LEVEL_HUD_ZOMBIE_R].IntValue;
            iGreen = gCvarList[CVAR_LEVEL_HUD_ZOMBIE_G].IntValue;
            iBlue = gCvarList[CVAR_LEVEL_HUD_ZOMBIE_B].IntValue;
        }
        // Otherwise, show human hud
        else
        {
            // Validate survivor hud
            if(gClientData[clientIndex][Client_Survivor])
            {
                strcopy(sInfo, sizeof(sInfo), "Survivor");
            }
            else
            {
                // Gets human name
                HumanGetName(gClientData[clientIndex][Client_HumanClass], sInfo, sizeof(sInfo));
            }
            // Gets colors 
            iRed = gCvarList[CVAR_LEVEL_HUD_HUMAN_R].IntValue;
            iGreen = gCvarList[CVAR_LEVEL_HUD_HUMAN_G].IntValue;
            iBlue = gCvarList[CVAR_LEVEL_HUD_HUMAN_B].IntValue;
        }

        // Print hud text to client
        TranslationPrintHudText(hHudLevel, clientIndex, 0.02, 0.885, 1.1, iRed, iGreen, iBlue, 255, 0, 0.0, 0.0, 0.0, "Level info", GetClientArmor(clientIndex), sInfo, gClientData[clientIndex][Client_Level], gClientData[clientIndex][Client_Exp], LevelSystemStats[gClientData[clientIndex][Client_Level]]);
    
        // Allow timer
        return Plugin_Continue;
    }
    
    // Clear timer
    gClientData[clientIndex][Client_LevelTimer] = INVALID_HANDLE;
    
    // Destroy timer
    return Plugin_Stop;
}
