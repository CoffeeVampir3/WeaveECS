module;
export module Entity;

import std;
import Component;

export namespace Ecs
{
    typedef unsigned long long EntityId;

    struct Entity {
        EntityId id;
        std::vector<ComponentId> components;
    };

    class EntityManager {
        ComponentId nextId = 0;
        std::vector<Entity> entities;
        std::unordered_map<EntityId, std::size_t> entityIdMap;
    public:
        EntityId addEntity(Entity&& newEntity)
        {
            EntityId newEntityId = ++nextId;
            newEntity.id = newEntityId;
            entities.emplace_back(std::move(newEntity));
            int lastIndex = entities.size() - 1;

            entityIdMap[newEntityId] = lastIndex;
            return newEntityId;
        }
    };
}


