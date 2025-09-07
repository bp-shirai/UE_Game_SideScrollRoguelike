#pragma once

#include "CoreMinimal.h"

#include "RogueGameTypes.generated.h"

// Define category "LogGame"
SIDESCROLLROGUELIKE_API DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);

// Perception System simple Team Numbers
namespace TeamID
{
    static const uint8 Bots    = 1;
    static const uint8 Players = 2;
} // namespace TeamID

namespace CollisionProfile
{
    static const FName Ragdoll           = FName("Ragdoll");
    static const FName OverlapAllPLayers = FName("OverlapAllPlayers");
    static const FName Projectile        = FName("Projectile");
    static const FName PickupItem        = FName("PickupItem");
    static const FName Player_Capsule    = FName("Player_Capsule");
    static const FName Player_Mesh       = FName("Player_Mesh");
    static const FName Player_Hurt       = FName("Player_Hurt");
    static const FName Enemy_Capsule     = FName("Enemy_Capsule");
    static const FName Enemy_Mesh        = FName("Enemy_Mesh");
    static const FName Enemy_Hurt        = FName("Enemy_Hurt");

} // namespace CollisionProfile

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
namespace CollisionChannel
{
    static const ECollisionChannel Player     = ECC_GameTraceChannel1;
    static const ECollisionChannel Projectile = ECC_GameTraceChannel2;
    static const ECollisionChannel Enemy      = ECC_GameTraceChannel3;

} // namespace CollisionChannel

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/
namespace PhysicalSurface
{
    static const EPhysicalSurface Default  = SurfaceType_Default;
    static const EPhysicalSurface Concrete = SurfaceType1;
    static const EPhysicalSurface Dirt     = SurfaceType2;
    static const EPhysicalSurface Water    = SurfaceType3;
    static const EPhysicalSurface Metal    = SurfaceType4;
    static const EPhysicalSurface Wood     = SurfaceType5;
    static const EPhysicalSurface Grass    = SurfaceType6;
    static const EPhysicalSurface Glass    = SurfaceType7;
    static const EPhysicalSurface Flesh    = SurfaceType8;

    static const EPhysicalSurface ZombieBody = SurfaceType9;
    static const EPhysicalSurface ZombieHead = SurfaceType10;
    static const EPhysicalSurface ZombieLimb = SurfaceType11;
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