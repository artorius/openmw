#ifndef GAME_MWCLASS_ARMOR_H
#define GAME_MWCLASS_ARMOR_H

#include "../mwworld/class.hpp"

namespace MWClass
{
    class Armor : public MWWorld::Class
    {
        public:

            virtual void insertObjectRendering (const MWWorld::Ptr& ptr, MWRender::RenderingInterface& renderingInterface) const;
            ///< Add reference into a cell for rendering

            virtual void insertObject(const MWWorld::Ptr& ptr, MWWorld::PhysicsSystem& physics, MWWorld::Environment& environment) const;

            virtual std::string getName (const MWWorld::Ptr& ptr) const;
            ///< \return name (the one that is to be presented to the user; not the internal one);
            /// can return an empty string.

            virtual boost::shared_ptr<MWWorld::Action> activate (const MWWorld::Ptr& ptr,
                const MWWorld::Ptr& actor, const MWWorld::Environment& environment) const;
            ///< Generate action for activation

            virtual bool hasItemHealth (const MWWorld::Ptr& ptr) const;
            ///< \return Item health data available?

            virtual int getItemMaxHealth (const MWWorld::Ptr& ptr) const;
            ///< Return item max health or throw an exception, if class does not have item health

            virtual std::string getScript (const MWWorld::Ptr& ptr) const;
            ///< Return name of the script attached to ptr

            virtual std::pair<std::vector<int>, bool> getEquipmentSlots (const MWWorld::Ptr& ptr) const;
            ///< \return first: Return IDs of the slot this object can be equipped in; second: can object
            /// stay stacked when equipped?

            virtual int getEquipmentSkill (const MWWorld::Ptr& ptr,
                const MWWorld::Environment& environment) const;
            /// Return the index of the skill this item corresponds to when equiopped or -1, if there is
            /// no such skill.

            static void registerSelf();

            virtual std::string getUpSoundId (const MWWorld::Ptr& ptr, const MWWorld::Environment& environment) const;
            ///< Return the pick up sound Id

            virtual std::string getDownSoundId (const MWWorld::Ptr& ptr, const MWWorld::Environment& environment) const;
            ///< Return the put down sound Id
    };
}

#endif
