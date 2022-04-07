#include <Weapon.h>

using namespace Entity;

const std::unordered_map<WeaponTypeID, WeaponType> Entity::WeaponMgr::weaponTypes = {
    {
        WeaponTypeID::SNIPER, 
        {
            WeaponTypeID::SNIPER, WeaponType::FiringMode::SEMI_AUTO, Util::Time::Seconds{0.5f}, Util::Time::Seconds{2.0f}, 100.0f, 
            300.0f, 0.0f, 0, 3.0f, 21, 12, AmmoType::AMMO, AmmoTypeData{ .magazineSize = 4}
        },
    },
    {
        WeaponTypeID::CHEAT_SMG, 
        {
            WeaponTypeID::CHEAT_SMG, WeaponType::FiringMode::BURST, Util::Time::Seconds{0.05f}, Util::Time::Seconds{2.0f}, 100.0f, 
            300.0f, 0.0f, 3, 1.5f, 21, 12, AmmoType::OVERHEAT, AmmoTypeData{ .overheatRate = 10.0f}
        }
    }
};

// WeaponType

Weapon WeaponType::CreateInstance() const
{
    Weapon weapon;
    weapon.weaponTypeId = id;
    weapon.state = Weapon::State::IDLE;
    weapon.ammoState = ResetAmmo(ammoData, ammoType);

    return weapon;
}

// Weapon

bool Entity::HasShot(Weapon::State s1, Weapon::State s2)
{
    return s1 != Weapon::State::SHOOTING && s2 == Weapon::State::SHOOTING;
}

bool Entity::HasReloaded(Weapon::State s1, Weapon::State s2)
{
    return s1 != Weapon::State::RELOADING && s2 == Weapon::State::RELOADING;
}

bool Entity::CanShoot(Weapon weapon)
{
    auto wepType = WeaponMgr::weaponTypes.at(weapon.weaponTypeId);
    bool isAuto = wepType.firingMode == WeaponType::FiringMode::AUTO;
    bool canShootSemi = (wepType.firingMode == WeaponType::FiringMode::SEMI_AUTO) && !weapon.triggerPressed;
    bool canShootBurst = wepType.firingMode == WeaponType::FiringMode::BURST && (weapon.burstCount > 0 || (!weapon.triggerPressed && weapon.burstCount == 0));

    bool canShoot = isAuto || canShootSemi || canShootBurst;

    return canShoot;
}

// Ammo

Weapon::AmmoState Entity::ResetAmmo(AmmoTypeData ammoData, AmmoType ammoType)
{
    Weapon::AmmoState state;
    if(ammoType == AmmoType::AMMO)
        state.magazine = ammoData.magazineSize;
    else if(ammoType == AmmoType::OVERHEAT)
        state.overheat = 0.0f;
    
    return state;
}

Weapon::AmmoState Entity::UseAmmo(Weapon::AmmoState ammoState, AmmoTypeData ammoData, AmmoType ammoType)
{
    switch (ammoType)
    {
    case AmmoType::AMMO:
        if(ammoState.magazine > 0)
            ammoState.magazine -= 1;
        break;
    
    case AmmoType::OVERHEAT:
        if(ammoState.overheat < MAX_OVERHEAT)
            ammoState.overheat = std::min(ammoState.overheat + ammoData.overheatRate, MAX_OVERHEAT);
        break;

    case AmmoType::INFINITE_AMMO:
    default:
        break;
    }

    return ammoState;
}

bool Entity::HasAmmo(Weapon::AmmoState ammoState, AmmoTypeData ammoData, AmmoType ammoType)
{
    bool hasAmmo = true;
    switch (ammoType)
    {
    case AmmoType::AMMO:
        if(ammoState.magazine <= 0)
            hasAmmo = false;
        break;
    
    case AmmoType::OVERHEAT:
        if(ammoState.overheat >= MAX_OVERHEAT)
            hasAmmo = false;
        break;

    case AmmoType::INFINITE_AMMO:
    default:
        break;
    }

    return hasAmmo;
}

bool Entity::IsMagFull(Weapon::AmmoState ammoState, AmmoTypeData ammoData, AmmoType ammoType)
{
    bool isFull = false;
    switch(ammoType)
    {
    case AmmoType::AMMO:
        isFull = ammoState.magazine == ammoData.magazineSize;
    break;
    
    case AmmoType::OVERHEAT:
        isFull = ammoState.overheat <= 1.0f;
    break;

    case AmmoType::INFINITE_AMMO:
        isFull = true;
    default:
        break;
    }

    return isFull;
}

// Dmg

float Entity::GetDistanceDmgMod(Entity::WeaponType wepType, float distance)
{
    auto range = wepType.maxRange;
    float mod = distance <= range ? 1.0f : range / distance;

    return mod;
}