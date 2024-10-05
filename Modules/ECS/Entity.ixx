module;
export module Entity;

import std;
import Component;
import Logging;

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
        std::unordered_map<std::type_index, BitsetUint> typeMap;
        unsigned long long nextValue = 1;
        unsigned numRegisteredTypes = 0;

        static ArchetypeMap* Instance()
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
            if (numRegisteredTypes++ >= maximumRegisterableTypes) {
                Logging::failure("Attempted to register more components than bitset could deal with.");
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

    using ComponentMap = std::unordered_map<std::type_index, ComponentId>;

    class EntityManager;
    struct Entity {
        friend EntityManager;

        template <ComponentType compType>
        Entity& addComponent(compType&& newComponent)
        {
            const auto componentManager = ComponentManager::Instance();
            const auto archetypeMap = ArchetypeMap::Instance();
            auto newCompId = componentManager->addComponent(id, std::forward<compType>(newComponent));
            auto compTypeIndex = std::type_index(typeid(compType));

            (*componentTypeMap)[compTypeIndex] = newCompId;
            archetypeBits |= archetypeMap->registerType<compType>();

            return *this;
        }

        template <ComponentType compType>
        compType* component()
        {
            auto it = componentTypeMap->find(std::type_index(typeid(compType)));
            if (it == componentTypeMap->end()) {
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

    private:
        EntityId id {0};
        BitsetUint archetypeBits {0};
        ComponentMap* componentTypeMap {nullptr};
    };

    class EntityManager {
        friend Entity;
        ComponentId nextId = 0;
        ArchetypeMap archetypeMap;
        std::vector<Entity> entities;
        std::unordered_map<EntityId, std::size_t> entityIdMap;
    public:
        Entity& newEntity()
        {
            EntityId newEntityId = ++nextId;
            Entity newEntity;
            newEntity.id = newEntityId;
            newEntity.componentTypeMap = new ComponentMap();
            entities.emplace_back(newEntity);

            int lastIndex = entities.size() - 1;
            entityIdMap[newEntityId] = lastIndex;
            return entities.back();
        }

        std::vector<Entity>& getEntities()
        {
            return entities;
        }

        template<ComponentType... compTypes>
        std::vector<Entity*> entitiesWith()
        {
            std::vector<Entity*> matchingEntities;
            const auto archetypeMap = ArchetypeMap::Instance();
            const auto archetypeBits = archetypeMap->makeArchetype<compTypes...>();

            for (auto& entity : entities)
            {
                if ((entity.archetypeBits & archetypeBits) == archetypeBits)
                {
                    matchingEntities.emplace_back(&entity);
                }
            }

            return matchingEntities;
        }

        void deleteEntity(EntityId idToDestroy)
        {
            auto it = entityIdMap.find(idToDestroy);
            //If component doesn't exist, do nothing
            if (it == entityIdMap.end()) {
                return;
            }

            int lastIndex = entities.size() - 1;
            EntityId lastCompId = entities[lastIndex].id;
            int destroyedIndex = it->second;

            delete entities[destroyedIndex].componentTypeMap;

            //In the event that we're destroying the last element, both of these operations are equivalent to a no-op, which
            //means we do not need any conditional logic here.
            std::swap(entities[destroyedIndex], entities[lastIndex]);
            entityIdMap[lastCompId] = destroyedIndex;

            entities.pop_back();
            entityIdMap.erase(idToDestroy);
        }
    };
}


