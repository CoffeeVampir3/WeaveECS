module;
#include "ankerl/unordered_dense.h"
#include <immintrin.h>
export module Entity;

import std;
import Component;
import Logging;

template <>
struct ankerl::unordered_dense::hash<std::type_index> {
    using is_avalanching = void;

    [[nodiscard]] auto operator()(std::type_index const& type_idx) const noexcept -> uint64_t {
        return hash<size_t>{}(type_idx.hash_code());
    }
};

export namespace Ecs
{
    typedef unsigned long long EntityId;
    typedef unsigned long long BitsetUint;

    //TODO:: @Z: Proper reflection whenever it comes to C++, in 2049 or whatever.
    //Lots of usages of std::type_index, should be replaced with smarter thing.
    //To extend this to a real idea, using a fixed size array like array<bits, N> will work.
    struct ArchetypeMap
    {
        static constexpr unsigned maximumRegisterableTypes = 64u;
        ankerl::unordered_dense::map<std::type_index, BitsetUint> typeMap;
        unsigned long long nextValue = 1;
        unsigned numRegisteredTypes = 0;

        inline static ArchetypeMap* Instance()
        {
            static ArchetypeMap* instance;
            [[unlikely]]
            if (instance == nullptr)
            {
                instance = new ArchetypeMap();
            }
            return instance;
        }

        template<typename T>
        BitsetUint registerType() {
            if (numRegisteredTypes >= maximumRegisterableTypes) {
                Logging::failure("Attempted to register more components than bitset could deal with.");
                std::abort();
            }
            std::type_index typeIdx(typeid(T));
            auto it = typeMap.find(typeIdx);
            if (it != typeMap.end())
            {
                return it->second;
            }

            auto bitValue = nextValue;
            typeMap[typeIdx] = bitValue;
            nextValue <<= 1;
            numRegisteredTypes++;
            return bitValue;
        }

        template<typename T>
        BitsetUint lookupType()
        {
            return typeMap[std::type_index(typeid(T))];
        }

        template<typename... ComponentTypes>
        BitsetUint makeArchetype() {
            return (... | lookupType<ComponentTypes>());
        }
    };

    using ComponentMap = ankerl::unordered_dense::map<std::type_index, ComponentId>;

    struct Entity;
    class EntityManager {
        ComponentId nextId = 0;
        ArchetypeMap archetypeMap;
        std::vector<Entity> entities;
        ankerl::unordered_dense::map<EntityId, std::size_t> entityIdMap;

        EntityManager() = default;
        EntityManager(const EntityManager&) = delete;
        EntityManager& operator=(const EntityManager&) = delete;

    public:
        static EntityManager* Instance();

        Entity& newEntity();
        std::vector<Entity>& getEntities();

        template<ComponentType... compTypes>
        std::vector<Entity*> entitiesWith();

        void deleteEntity(EntityId idToDestroy);
    };

    struct Entity {
        friend EntityManager;

        template <ComponentType compType>
        Entity& addComponent(compType&& newComponent)
        {
            const auto componentManager = ComponentManager::Instance();
            const auto archetypeMap = ArchetypeMap::Instance();
            auto newCompId = componentManager->addComponent(id, std::forward<compType>(newComponent));
            auto compTypeIndex = std::type_index(typeid(compType));

            componentTypeMap.emplace(compTypeIndex, newCompId);
            archetypeBits |= archetypeMap->registerType<compType>();

            return *this;
        }

        template <ComponentType compType>
        compType* component()
        {
            auto it = componentTypeMap.find(std::type_index(typeid(compType)));
            if (it == componentTypeMap.end()) {
                return nullptr;
            }
            return ComponentManager::Instance()->getComponent<compType>(it->second);
        }

        template<ComponentType... compTypes>
        std::tuple<compTypes*...> components()
        {
            return std::make_tuple(component<compTypes>()...);
        }

        constexpr EntityId Id() const
        {
            return id;
        }

        constexpr BitsetUint ArchetypeBits() const
        {
            return archetypeBits;
        }

        explicit Entity(const EntityId id) :id(id) {};
    private:
        EntityId id {0};
        BitsetUint archetypeBits {0};
        ComponentMap componentTypeMap;
    };

    EntityManager* EntityManager::Instance()
    {
        static EntityManager* instance;
        [[unlikely]]
        if (instance == nullptr)
        {
            instance = new EntityManager();
            instance->entityIdMap.reserve(1024*8);
            instance->entities.reserve(1024*8);
        }
        return instance;
    }

    Entity& EntityManager::newEntity() {
        const EntityId newEntityId = ++nextId;
        Entity ent(newEntityId);
        entities.push_back(std::move(ent));

        const int lastIndex = entities.size() - 1;
        entityIdMap.emplace(newEntityId, lastIndex);
        return entities.back();
    }

    std::vector<Entity>& EntityManager::getEntities() {
        return entities;
    }

    template<ComponentType... compTypes>
    std::vector<Entity*> EntityManager::entitiesWith() {
        std::vector<Entity*> matchingEntities;
        const auto archetypeMap = ArchetypeMap::Instance();
        const auto archetypeBits = archetypeMap->makeArchetype<compTypes...>();

        for (auto& entity : entities)
        {
            if ((entity.archetypeBits & archetypeBits) == archetypeBits)
            {
                matchingEntities.push_back(&entity);
            }
        }

        return matchingEntities;
    }

    void EntityManager::deleteEntity(const EntityId idToDestroy) {
        const auto it = entityIdMap.find(idToDestroy);
        //If component doesn't exist, do nothing
        if (it == entityIdMap.end()) {
            return;
        }

        const int lastIndex = entities.size() - 1;
        const EntityId lastCompId = entities[lastIndex].id;
        const int destroyedIndex = it->second;

        //In the event that we're destroying the last element, both of these operations are equivalent to a no-op, which
        //means we do not need any conditional logic here.
        std::swap(entities[destroyedIndex], entities[lastIndex]);
        entityIdMap.emplace(lastCompId, destroyedIndex);

        entities.pop_back();
        entityIdMap.erase(idToDestroy);
    }
}


