/**
 * ============================================================================
 *
 *  Zombie Plague
 *
 *  File:          tools_functions.cpp
 *  Type:          Module 
 *  Description:   API for offsets/signatures exposed in tools.cpp
 *
 *  Copyright (C) 2015-2019 Nikita Ushakov (Ireland, Dublin)
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
 * @section Hud elements flags.
 **/
#define HIDEHUD_WEAPONSELECTION     (1<<0)   // Hide ammo count & weapon selection
#define HIDEHUD_FLASHLIGHT          (1<<1)
#define HIDEHUD_ALL                 (1<<2)
#define HIDEHUD_HEALTH              (1<<3)   // Hide health & armor / suit battery
#define HIDEHUD_PLAYERDEAD          (1<<4)   // Hide when local player's dead
#define HIDEHUD_NEEDSUIT            (1<<5)   // Hide when the local player doesn't have the HEV suit
#define HIDEHUD_MISCSTATUS          (1<<6)   // Hide miscellaneous status elements (trains, pickup history, death notices, etc)
#define HIDEHUD_CHAT                (1<<7)   // Hide all communication elements (saytext, voice icon, etc)
#define HIDEHUD_CROSSHAIR           (1<<8)   // Hide crosshairs
#define HIDEHUD_VEHICLE_CROSSHAIR   (1<<9)   // Hide vehicle crosshair
#define HIDEHUD_INVEHICLE           (1<<10)
#define HIDEHUD_BONUS_PROGRESS      (1<<11)  // Hide bonus progress display (for bonus map challenges)
/**
 * @endsection
 **/
 
 /**
 * @section Entity effects flags.
 **/
#define EF_BONEMERGE                (1<<0)     // Performs bone merge on client side
#define EF_BRIGHTLIGHT              (1<<1)     // DLIGHT centered at entity origin
#define EF_DIMLIGHT                 (1<<2)     // Player flashlight
#define EF_NOINTERP                 (1<<3)     // Don't interpolate the next frame
#define EF_NOSHADOW                 (1<<4)     // Disables shadow
#define EF_NODRAW                   (1<<5)     // Prevents the entity from drawing and networking
#define EF_NORECEIVESHADOW          (1<<6)     // Don't receive shadows
#define EF_BONEMERGE_FASTCULL       (1<<7)     // For use with EF_BONEMERGE. If this is set, then it places this ents origin at its parent and uses the parent's bbox + the max extents of the aiment. Otherwise, it sets up the parent's bones every frame to figure out where to place the aiment, which is inefficient because it'll setup the parent's bones even if the parent is not in the PVS.
#define EF_ITEM_BLINK               (1<<8)     // Makes the entity blink
#define EF_PARENT_ANIMATES          (1<<9)     // Always assume that the parent entity is animating
#define EF_FOLLOWBONE               (1<<10)    
/**
 * @endsection
 **/
 
/**
 * @brief Creates commands for tools module.
 **/
void ToolsOnCommandInit(/*void*/)
{
    // Hook messages
    HookUserMessage(GetUserMessageId("TextMsg"), ToolsOnMessageHook, true);
}

/**
 * @brief Hook tools cvar changes.
 **/
void ToolsOnCvarInit(/*void*/)
{
    // Creates cvars
    gCvarList[CVAR_LIGHT_BUTTON]   = FindConVar("zp_light_button");  
    gCvarList[CVAR_MESSAGES_HELP]  = FindConVar("zp_messages_help");
    gCvarList[CVAR_MESSAGES_BLOCK] = FindConVar("zp_messages_block");
    
    // Hook cvars
    HookConVarChange(gCvarList[CVAR_LIGHT_BUTTON], ToolsFOnCvarHook);
    
    // Load cvars
    ToolsOnCommandLoad();
}

/**
 * @brief Load tools listeners changes.
 **/
void ToolsOnCommandLoad(/*void*/)
{
    // Initialize command char
    static char sCommand[SMALL_LINE_LENGTH];
    
    // Validate alias
    if(hasLength(sCommand))
    {
        // Unhook listeners
        RemoveCommandListener2(ToolsOnCommandListened, sCommand);
    }
    
    // Gets flashlight command alias
    gCvarList[CVAR_LIGHT_BUTTON].GetString(sCommand, sizeof(sCommand));
    
    // Validate alias
    if(!hasLength(sCommand))
    {
        // Unhook listeners
        RemoveCommandListener2(ToolsOnCommandListened, sCommand);
        return;
    }
    
    // Hook listeners
    AddCommandListener(ToolsOnCommandListened, sCommand);
}


/**
 * Cvar hook callback (zp_light_button)
 * @brief Flashlight module initialization.
 * 
 * @param hConVar           The cvar handle.
 * @param oldValue          The value before the attempted change.
 * @param newValue          The new value.
 **/
public void ToolsFOnCvarHook(ConVar hConVar, const char[] oldValue, const char[] newValue)
{
    // Validate new value
    if(!strcmp(oldValue, newValue, false))
    {
        return;
    }
    
    // Forward event to modules
    ToolsOnCommandLoad();
}

/**
 * Listener command callback (any)
 * @brief Usage of the on/off flashlight/nvgs.
 *
 * @param clientIndex       The client index.
 * @param commandMsg        Command name, lower case. To get name as typed, use GetCmdArg() and specify argument 0.
 * @param iArguments        Argument count.
 **/
public Action ToolsOnCommandListened(const int clientIndex, const char[] commandMsg, const int iArguments)
{
    // Validate client 
    if(IsPlayerExist(clientIndex))
    {
        // Gets class overlay
        static char sOverlay[PLATFORM_LINE_LENGTH];
        ClassGetOverlay(gClientData[clientIndex].Class, sOverlay, sizeof(sOverlay));

        // Validate nvgs
        if(ClassIsNvgs(gClientData[clientIndex].Class) || hasLength(sOverlay)) 
        {
            // If mode doesn't ended yet, then stop
            if(gServerData.RoundEnd) /// Avoid reset round end overlays
            {
                // Block command
                return Plugin_Handled;
            }
            
            // Switch on/off nightvision  
            VOverlayOnClientNvgs(clientIndex);
            
            // Forward event to modules
            SoundsOnClientNvgs(clientIndex);
        }
        else
        {
            // Switch on/off flashlight
            ToolsSetClientFlashLight(clientIndex, true);
            
            // Forward event to modules
            SoundsOnClientFlashLight(clientIndex);
        }
        
        // Block command
        return Plugin_Handled;
    }
    
    // Allow command
    return Plugin_Continue;
}

/**
 * @brief Hook client messages.
 *
 * @param iMessage          The message index.
 * @param hBuffer           Handle to the input bit buffer.
 * @param iPlayers          Array containing player indexes.
 * @param playersNum        Number of players in the array.
 * @param bReliable         True if message is reliable, false otherwise.
 * @param bInit             True if message is an initmsg, false otherwise.
 **/
public Action ToolsOnMessageHook(UserMsg iMessage, BfRead hBuffer, const int[] iPlayers, const int playersNum, const bool bReliable, const bool bInit)
{
    // Initialize engine message
    static char sTxtMsg[PLATFORM_LINE_LENGTH]; 
    PbReadString(hBuffer, "params", sTxtMsg, sizeof(sTxtMsg), 0); 

    // Initialize block message list
    static char sBlockMsg[PLATFORM_LINE_LENGTH];
    gCvarList[CVAR_MESSAGES_BLOCK].GetString(sBlockMsg, sizeof(sBlockMsg)); 

    // Block messages on the matching
    return (StrContains(sBlockMsg, sTxtMsg, false) != -1) ? Plugin_Handled : Plugin_Continue; 
}

/*
 * Stocks tools API.
 */

/**
 * @brief Respawn a player.
 *
 * @param clientIndex       The client index.
 * @return                  True on success, false otherwise. 
 **/
bool ToolsForceToRespawn(const int clientIndex)
{
    // Validate client
    if(!IsPlayerExist(clientIndex, false))
    {
        return false;
    }
    
    // Verify that the client is dead
    if(IsPlayerAlive(clientIndex))
    {
        return false;
    }
    
    // Respawn as human ?
    int iDeathMatch = ModesGetMatch(gServerData.RoundMode);
    if(iDeathMatch == 1 || (iDeathMatch == 2 && GetRandomInt(0, 1)) || (iDeathMatch == 3 && fnGetHumans() < fnGetAlive() / 2))
    {
        gClientData[clientIndex].Respawn = TEAM_HUMAN;
    }
    // Respawn as zombie ?
    else
    {
        gClientData[clientIndex].Respawn = TEAM_ZOMBIE;
    }

    // Respawn a player
    CS_RespawnPlayer(clientIndex);
    
    // Return on success
    return true;
}

/**
 * @brief Gets or sets the velocity of a client.
 *
 * @param clientIndex       The client index.
 * @param vVelocity         The velocity output, or velocity to set on client.
 * @param bApply            True to get client velocity, false to set it.
 * @param bStack            If modifying velocity, then true will stack new velocity onto the client.
 *                          current velocity, false will reset it.
 **/
void ToolsClientVelocity(const int clientIndex, float vVelocity[3], const bool bApply = true, const bool bStack = true)
{
    // If retrieve if true, then get client velocity
    if(!bApply)
    {
        // i = vector component
        for(int i = 0; i < 3; i++)
        {
            vVelocity[i] = GetEntDataFloat(clientIndex, g_iOffset_PlayerVelocity + (i * 4));
        }
        
        // Stop here
        return;
    }
    
    // If stack is true, then add client velocity
    if(bStack)
    {
        // Gets client velocity
        static float vecClientVelocity[3];
        
        // i = vector component
        for(int i = 0; i < 3; i++)
        {
            vecClientVelocity[i] = GetEntDataFloat(clientIndex, g_iOffset_PlayerVelocity + (i * 4));
        }
        
        AddVectors(vecClientVelocity, vVelocity, vVelocity);
    }
    
    // Apply velocity on client
    TeleportEntity(clientIndex, NULL_VECTOR, NULL_VECTOR, vVelocity);
}

/**
 * @brief Gets the velocity of a client.
 *
 * @param clientIndex       The client index.
 * @param vVelocity         The velocity output.
 **/
void ToolsGetClientVelocity(const int clientIndex, float vVelocity[3])
{
    // i = vector component
    for(int i = 0; i < 3; i++)
    {
        vVelocity[i] = GetEntDataFloat(clientIndex, g_iOffset_PlayerVelocity + (i * 4));
    }
}

/**
 * @brief Sets the health of a client.
 *
 * @param clientIndex       The client index.
 * @param iValue            The health value.
 * @param bSet              True to set maximum value, false to modify health.  
 **/
void ToolsSetClientHealth(const int clientIndex, const int iValue, const bool bSet = false)
{
    // Sets health of client
    SetEntData(clientIndex, g_iOffset_PlayerHealth, iValue, _, true);
    
    // If set is true, then set max health
    if(bSet) 
    {
        // Find the datamap
        if(!g_iOffset_PlayerMaxHealth)
        {
            g_iOffset_PlayerMaxHealth = FindDataMapInfo(clientIndex, "m_iMaxHealth");
        }

        // Sets max health of client
        SetEntData(clientIndex, g_iOffset_PlayerMaxHealth, iValue, _, true);
    }
}

/**
 * @brief Sets the speed of a client.
 *
 * @param clientIndex       The client index.
 * @param flValue           The LMV value.
 **/
void ToolsSetClientLMV(const int clientIndex, const float flValue)
{
    // Sets lagged movement value of client
    SetEntDataFloat(clientIndex, g_iOffset_PlayerLMV, flValue, true);
}

/**
 * @brief Sets the armor of a client.
 *
 * @param clientIndex       The client index.
 * @param iValue            The armor value.
 **/
void ToolsSetClientArmor(const int clientIndex, const int iValue)
{
    // Sets armor of client
    SetEntData(clientIndex, g_iOffset_PlayerArmor, iValue, _, true);
}

/**
 * @brief Sets the team of a client.
 *
 * @param clientIndex       The client index.
 * @param iValue            The team index.
 **/
void ToolsSetClientTeam(const int clientIndex, const int iValue)
{
    // Validate team
    if(GetClientTeam(clientIndex) <= TEAM_SPECTATOR) /// Fix, thanks to inklesspen!
    {
        // Sets team of client
        ChangeClientTeam(clientIndex, iValue);
    }
    else
    {
        // Switch team of client
        CS_SwitchTeam(clientIndex, iValue); 
    }
}

/**
 * @brief Gets nightvision values on a client.
 *
 * @param clientIndex       The client index.
 * @param ownership         If true, function will return the value of the client ownership of nightvision.
 *                          If false, function will return the value of the client on/off state of the nightvision.
 * @return                  True if aspect of nightvision is enabled on the client, false if not.
 **/
bool ToolsGetClientNightVision(const int clientIndex, const bool bOwnership = false)
{
    // If ownership is true, then gets the ownership of nightvision on client
    return view_as<bool>(GetEntData(clientIndex, bOwnership ? g_iOffset_PlayerHasNightVision : g_iOffset_PlayerNightVisionOn, 1));
}

/**
 * @brief Controls nightvision values on a client.
 *
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of nightvision. (see ownership parameter)
 * @param bOwnership        If true, enable will toggle the client ownership of nightvision.
 *                          If false, enable will toggle the client on/off state of the nightvision.
 **/
void ToolsSetClientNightVision(const int clientIndex, const bool bEnable, const bool bOwnership = false)
{
    // If ownership is true, then toggle the ownership of nightvision on client
    SetEntData(clientIndex, bOwnership ? g_iOffset_PlayerHasNightVision : g_iOffset_PlayerNightVisionOn, bEnable, 1, true);
}

/**
 * @brief Gets defuser value on a client.
 *
 * @param clientIndex       The client index.
 * @return                  The aspect of the client defuser.
 **/
bool ToolsGetClientDefuser(const int clientIndex)
{
    // Gets value on the client
    return view_as<bool>(GetEntData(clientIndex, g_iOffset_PlayerHasDefuser, 1));
}

/**
 * @brief Controls defuser value on a client.
 *
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of defuser.
 **/
void ToolsSetClientDefuser(const int clientIndex, const bool bEnable)
{
    // Sets value on the client
    SetEntData(clientIndex, g_iOffset_PlayerHasDefuser, bEnable, 1, true);
}

/**
 * @brief Gets helmet value on a client.
 *
 * @param clientIndex       The client index.
 * @return                  The aspect of the client helmet.
 **/
bool ToolsGetClientHelmet(const int clientIndex)
{
    // Gets value on the client
    return view_as<bool>(GetEntData(clientIndex, g_iOffset_PlayerHasHelmet, 1));
}

/**
 * @brief Controls helmet value on a client.
 *
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of helmet.
 **/
void ToolsSetClientHelmet(const int clientIndex, const bool bEnable)
{
    // Sets value on the client
    SetEntData(clientIndex, g_iOffset_PlayerHasHelmet, bEnable, 1, true);
}

/**
 * @brief Gets suit value on a client.
 *
 * @param clientIndex       The client index.
 * @return                  The aspect of the client suit.
 **/
bool ToolsGetClientHeavySuit(const int clientIndex)
{
    // Gets value on the client
    return view_as<bool>(GetEntData(clientIndex, g_iOffset_PlayerHasHeavyArmor, 1));
}

/**
 * @brief Controls suit value on a client.
 *
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of suit.
 **/
void ToolsSetClientHeavySuit(const int clientIndex, const bool bEnable)
{
    // Sets value on the client
    SetEntData(clientIndex, g_iOffset_PlayerHasHeavyArmor, bEnable, 1, true);
}

/**
 * @brief Gets the active weapon index of a client.
 *
 * @param clientIndex       The client index.
 * @return                  The weapon index.
 **/
int ToolsGetClientActiveWeapon(const int clientIndex)
{
    // Gets value on the client    
    return GetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon);
}

/**
 * @brief Sets the active weapon index of a client.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
void ToolsSetClientActiveWeapon(const int clientIndex, const int weaponIndex)
{
    // Sets value on the client    
    SetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon, weaponIndex, true);
}

/**
 * @brief Gets the last weapon index of a client.
 *
 * @param clientIndex       The client index.
 * @return                  The weapon index.
 **/
int ToolsGetClientLastWeapon(const int clientIndex)
{
    // Gets value on the client    
    return GetEntDataEnt2(clientIndex, g_iOffset_PlayerLastWeapon);
}

/**
 * @brief Sets the last weapon index of a client.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
void ToolsSetClientLastWeapon(const int clientIndex, const int weaponIndex)
{
    // Sets value on the client    
    SetEntDataEnt2(clientIndex, g_iOffset_PlayerLastWeapon, weaponIndex, true);
}

/**
 * @brief Gets the addon bits of a client.
 *
 * @param clientIndex       The client index.
 * @return                  The addon bits.
 **/
int ToolsGetClientAddonBits(const int clientIndex)
{
    // Gets value on the client    
    return GetEntData(clientIndex, g_iOffset_PlayerAddonBits);
}

/**
 * @brief Sets the addon bits index of a client.
 *
 * @param clientIndex       The client index.
 * @param iValue            The addon bits.
 **/
void ToolsSetClientAddonBits(const int clientIndex, const int iValue)
{
    // Sets value on the client    
    SetEntData(clientIndex, g_iOffset_PlayerAddonBits, iValue, _, true);
}

/**
 * @brief Gets the observer mode of a client.
 *
 * @param clientIndex       The client index.
 * @return                  The mode index.
 **/
int ToolsGetClientObserverMode(const int clientIndex)
{
    // Gets value on the client    
    return GetEntData(clientIndex, g_iOffset_PlayerObserverMode);
}

/**
 * @brief Gets the observer target of a client.
 *
 * @param clientIndex       The client index.
 * @return                  The target index.
 **/
int ToolsGetClientObserverTarget(const int clientIndex)
{
    // Gets value on the client    
    return GetEntDataEnt2(clientIndex, g_iOffset_PlayerObserverTarget);
}

/**
 * @brief Gets or sets a client score or deaths.
 * 
 * @param clientIndex       The client index.
 * @param bScore            True to look at score, false to look at deaths.  
 * @return                  The score or death count of the client.
 **/
int ToolsGetClientScore(const int clientIndex, const bool bScore = true)
{
    // If score is true, then return client score, otherwise return client deaths
    return bScore ? GetClientFrags(clientIndex) : GetClientDeaths(clientIndex);
}

/**
 * @brief Gets hitgroup value on a client.
 *
 * @param clientIndex       The client index.
 * @return                  The hitgroup index.
 **/
int ToolsGetClientHitGroup(const int clientIndex)
{
    // Gets value on the client    
    return GetEntData(clientIndex, g_iOffset_PlayerHitGroup);
}

/**
 * @brief Sets a client score or deaths.
 * 
 * @param clientIndex       The client index.
 * @param bScore            True to look at score, false to look at deaths.  
 * @param iValue            The score/death amount.
 **/
void ToolsSetClientScore(const int clientIndex, const bool bScore = true, const int iValue = 0)
{
    // Find the datamap
    if(!g_iOffset_PlayerFrags || !g_iOffset_PlayerDeath)
    {
        g_iOffset_PlayerFrags = FindDataMapInfo(clientIndex, "m_iFrags");
        g_iOffset_PlayerDeath = FindDataMapInfo(clientIndex, "m_iDeaths");
    }
    
    // If score is true, then set client score, otherwise set client deaths
    SetEntData(clientIndex, bScore ? g_iOffset_PlayerFrags : g_iOffset_PlayerDeath, iValue, _, true);
}


/**
 * @brief Sets the gravity of a client.
 * 
 * @param clientIndex       The client index.
 * @param flValue           The gravity amount.
 **/
void ToolsSetClientGravity(const int clientIndex, const float flValue)
{
    // Find the datamap
    if(!g_iOffset_PlayerGravity)
    {
        g_iOffset_PlayerGravity = FindDataMapInfo(clientIndex, "m_flGravity");
    }
    
    // Sets value on the client
    SetEntDataFloat(clientIndex, g_iOffset_PlayerGravity, flValue, true);
}

/**
 * @brief Sets the spotting of a client.
 * 
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of spotting.
 **/
void ToolsSetClientSpot(const int clientIndex, const bool bEnable)
{
    // If retrieve if true, then reset variables
    if(!bEnable)
    {
        // Sets value on the client
        SetEntData(clientIndex, g_iOffset_PlayerSpotted, false, 1, true);
        SetEntData(clientIndex, g_iOffset_PlayerSpottedByMask, false, _, true);
        SetEntData(clientIndex, g_iOffset_PlayerSpottedByMask + 4, false, _, true);
        SetEntData(clientIndex, g_iOffset_PlayerSpotted - 4, 0, _, true);
    }
    else
    {
        // Sets value on the client
        SetEntData(clientIndex, g_iOffset_PlayerSpotted - 4, 9, _, true);
    }
}

/**
 * @brief Sets the detecting of a client.
 * 
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of detection.
 **/
void ToolsSetClientDetecting(const int clientIndex, const bool bEnable)
{
    // Sets value on the client
    SetEntDataFloat(clientIndex, g_iOffset_PlayerDetected, bEnable ? (GetGameTime() + 9999.0) : 0.0, true);
}

/**
 * @brief Sets the hud of a client.
 * 
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of hud.
 **/
void ToolsSetClientHud(const int clientIndex, const bool bEnable)
{   
    // Sets value on the client
    SetEntData(clientIndex, g_iOffset_PlayerHUD, bEnable ? (GetEntData(clientIndex, g_iOffset_PlayerHUD) & ~HIDEHUD_CROSSHAIR) : (GetEntData(clientIndex, g_iOffset_PlayerHUD) | HIDEHUD_CROSSHAIR), _, true);
}

/**
 * @brief Sets the arms of a client.
 * 
 * @param clientIndex       The client index.
 * @param sModel            The model path.
 * @param iMaxLen           The lenght of string. 
 **/
void ToolsSetClientArm(const int clientIndex, char[] sModel, const int iMaxLen)
{
    // Sets value on the client
    SetEntDataString(clientIndex, g_iOffset_PlayerArms, sModel, iMaxLen, true);
}

/**
 * @brief Sets the attack delay of a client.
 * 
 * @param clientIndex       The client index.
 * @param flValue           The speed amount.
 **/
void ToolsSetClientAttack(const int clientIndex, const float flValue)
{
    // Sets value on the client
    SetEntDataFloat(clientIndex, g_iOffset_PlayerAttack, flValue, true);
}

/**
 * @brief Sets the flashlight of a client.
 * 
 * @param clientIndex       The client index.
 * @param bEnable           Enable or disable an aspect of flashlight.
 **/
void ToolsSetClientFlashLight(const int clientIndex, const bool bEnable)
{
    // Sets value on the client
    ToolsSetEntityEffect(clientIndex, bEnable ? (ToolsGetEntityEffect(clientIndex) ^ EF_DIMLIGHT) : 0);
}

/**
 * @brief Sets the fov of a client.
 * 
 * @param clientIndex       The client index.
 * @param iValue            (Optional) The fov amount.
 **/
void ToolsSetClientFov(const int clientIndex, const int iValue = 90)
{
    // Sets value on the client
    SetEntData(clientIndex, g_iOffset_PlayerFov, iValue, _, true);
    SetEntData(clientIndex, g_iOffset_PlayerDefaultFOV, iValue, _, true);
}

/*_____________________________________________________________________________________________________*/

/**
 * @brief Gets the effect of an entity.
 * 
 * @param entityIndex       The entity index.
 * @return                  The effect value.
 **/
int ToolsGetEntityEffect(const int entityIndex)
{
    // Gets value on the client    
    return GetEntData(entityIndex, g_iOffset_EntityEffects);
}

/**
 * @brief Sets the effect of an entity.
 * 
 * @param entityIndex       The entity index.
 * @param iValue            The effect value.
 **/
void ToolsSetEntityEffect(const int entityIndex, const int iValue)
{
    // Sets value on the entity
    SetEntData(entityIndex, g_iOffset_EntityEffects, iValue, _, true);
}

/**
 * @brief Sets the model of an entity.
 * 
 * @param entityIndex       The entity index.
 * @param iModel            The model index.
 **/
void ToolsSetEntityModelIndex(const int entityIndex, const int iModel)
{
    // Sets value on the entity
    SetEntData(entityIndex, g_iOffset_EntityModelIndex, iModel, _, true);
}

/**
 * @brief Gets the owner of an entity.
 * 
 * @param entityIndex       The entity index.
 * @return                  The owner index.
 **/
int ToolsGetEntityOwner(const int entityIndex)
{
    // Gets value on the entity
    return GetEntDataEnt2(entityIndex, g_iOffset_EntityOwnerEntity);
}

/**
 * @brief Sets the owner of an entity.
 * 
 * @param entityIndex       The entity index.
 * @param ownerIndex        The owner index.
 **/
void ToolsSetEntityOwner(const int entityIndex, const int ownerIndex)
{
    SetEntDataEnt2(entityIndex, g_iOffset_EntityOwnerEntity, ownerIndex, true);
}

/**
 * @brief Sets the team of an entity.
 * 
 * @param entityIndex       The entity index.
 * @param iValue            The team index.
 **/
void ToolsSetEntityTeam(const int entityIndex, const int iValue)
{
    SetEntData(entityIndex, g_iOffset_EntityTeam, iValue);
}

/**
 * @brief Gets the origin of an entity.
 *
 * @param entityIndex       The entity index.
 * @param vOrigin           The origin output.
 **/
void ToolsGetEntityOrigin(const int entityIndex, float vOrigin[3])
{
    // i = vector component
    for(int i = 0; i < 3; i++)
    {
        vOrigin[i] = GetEntDataFloat(entityIndex, g_iOffset_EntityOrigin + (i * 4));
    }
}

/*_____________________________________________________________________________________________________*/

/**
 * @brief Update a entity transmit state.
 * 
 * @param entityIndex       The entity index.
 **/
void ToolsUpdateTransmitState(const int entityIndex)
{
    SDKCall(hSDKCallEntityUpdateTransmitState, entityIndex);
}

/**
 * @brief Validate the attachment on the entity.
 *
 * @param entityIndex       The entity index.
 * @param sAttach           The attachment name.
 * @return                  True or false.
 **/
bool ToolsLookupAttachment(const int entityIndex, const char[] sAttach)
{
    return (hasLength(sAttach) && SDKCall(hSDKCallLookupAttachment, entityIndex, sAttach));
}

/**
 * @brief Gets the attachment of the entity.
 *
 * @param entityIndex       The entity index.
 * @param sAttach           The attachment name.
 * @param vOrigin           The origin ouput.
 * @param vAngle            The angle ouput.
 **/
void ToolsGetAttachment(const int entityIndex, const char[] sAttach, float vOrigin[3], float vAngle[3])
{
    // Validate length
    if(!hasLength(sAttach))
    {
        return;
    }
    
    // Validate windows
    if(GameEngineGetPlatform(OS_Windows))
    {
        SDKCall(hSDKCallGetAttachmentW, entityIndex, sAttach, vOrigin, vAngle); 
    }
    else
    {
        int iAnimating = SDKCall(hSDKCallLookupAttachment, entityIndex, sAttach);
        if(iAnimating)
        {
            SDKCall(hSDKCallGetAttachmentL, entityIndex, iAnimating, vOrigin, vAngle); 
        }
    }
}