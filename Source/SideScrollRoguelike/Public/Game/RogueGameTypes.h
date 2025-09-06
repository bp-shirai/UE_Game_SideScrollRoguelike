#pragma once

#include "CoreMinimal.h"

#include "RogueGameTypes.generated.h"

// Define category "LogGame"
SIDESCROLLROGUELIKE_API DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);

// Perception System simple Team Numbers
namespace TeamID
{
    static uint8 Bots    = 1;
    static uint8 Players = 2;
} // namespace TeamID

namespace CollisionProfile
{
    static FName Ragdoll           = FName("Ragdoll");
    static FName OverlapAllPLayers = FName("OverlapAllPlayers");
    static FName Projectile        = FName("Projectile");
    static FName PickupItem        = FName("PickupItem");
    static FName Player_Capsule    = FName("Player_Capsule");
    static FName Player_Mesh       = FName("Player_Mesh");
    static FName Player_Hurt       = FName("Player_Hurt");
    static FName Enemy_Capsule     = FName("Enemy_Capsule");
    static FName Enemy_Mesh        = FName("Enemy_Mesh");
    static FName Enemy_Hurt        = FName("Enemy_Hurt");

} // namespace CollisionProfile

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
namespace CollisionChannel
{
    ECollisionChannel Player     = ECC_GameTraceChannel1;
    ECollisionChannel Projectile = ECC_GameTraceChannel2;
    ECollisionChannel Enemy      = ECC_GameTraceChannel3;

} // namespace CollisionChannel

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/
namespace PhysicalSurface
{
    EPhysicalSurface Default  = SurfaceType_Default;
    EPhysicalSurface Concrete = SurfaceType1;
    EPhysicalSurface Dirt     = SurfaceType2;
    EPhysicalSurface Water    = SurfaceType3;
    EPhysicalSurface Metal    = SurfaceType4;
    EPhysicalSurface Wood     = SurfaceType5;
    EPhysicalSurface Grass    = SurfaceType6;
    EPhysicalSurface Glass    = SurfaceType7;
    EPhysicalSurface Flesh    = SurfaceType8;

    EPhysicalSurface ZombieBody = SurfaceType9;
    EPhysicalSurface ZombieHead = SurfaceType10;
    EPhysicalSurface ZombieLimb = SurfaceType11;
} // namespace PhysicalSurface

/** keep in sync with ImpactEffect */
UENUM()
enum class EPhysMaterialType : uint8
{
    Default,
    Concrete,
    Dirt,
    Water,
    Metal,
    Wood,
    Grass,
    Glass,
    Flesh,
};