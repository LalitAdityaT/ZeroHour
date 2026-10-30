Add To DefaultEngine.ini
=========================

;SteamOnlineSubsystem Implementation Start
[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SteamSockets.SteamSocketsNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480

; If Using Sessions
bInitServerOnClient=true

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"

;SteamOnlineSubsystem Implementation End

=========================

Add To DefaultGame.ini
=========================

[/Script/Engine.GameSession]
MaxPlayers=100

=========================