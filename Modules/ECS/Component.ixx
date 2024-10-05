module;
#include "ankerl/unordered_dense.h"
export module Component;
import std;

export namespace Ecs {
    typedef unsigned long long ComponentId;
    typedef unsigned long long EntityId;
    constexpr ComponentId invalidId = 0u;

    class ComponentManager;
    struct Component
    {
        friend ComponentManager;
        inline ComponentId Id() const { return id; }
        inline EntityId Entity() const { return owningEntityId; }

    private:
        ComponentId id = invalidId;
        EntityId owningEntityId = invalidId;
    };

    template<typename T>
    concept ComponentType = std::is_base_of_v<Component, T>;

    //Global because this is illegal within the class scope for some reason unknown to me.
    template <ComponentType compType>
    std::vector<compType> components;

    class ComponentManager
    {
        ComponentId nextId{invalidId};
        ankerl::unordered_dense::map<ComponentId, std::size_t, ankerl::unordered_dense::hash<ComponentId>> componentIdMap;
    public:
        inline static ComponentManager* Instance()
        {
            static ComponentManager* instance;
            [[unlikely]]
            if (instance == nullptr)
            {
                instance = new ComponentManager();
            }
            return instance;
        }
        template <ComponentType compType>
        ComponentId addComponent(EntityId ownerId, compType&& newComp)
        {
            ComponentId newComponentId = ++nextId;
            newComp.id = newComponentId;
            newComp.owningEntityId = ownerId;
            components<std::remove_reference_t<compType>>.push_back(std::move(newComp));
            const int lastIndex = components<compType>.size() - 1;

            componentIdMap[newComponentId] = lastIndex;
            return newComponentId;
        }

        ///A function for tests to ensure properties of the ID map.
        std::size_t testing_getComponentIdMapCount()
        {
            return componentIdMap.size();
        }

        template <ComponentType compType>
        std::size_t count()
        {
            return components<compType>.size();
        }

        template <ComponentType compType>
        void clear()
        {
            for (auto& component : components<compType>)
            {
                componentIdMap.erase(component.id);
            }
            components<compType>.clear();
        }

        template <ComponentType compType>
        std::vector<compType>& getComponents()
        {
            return components<compType>;
        }

        ///Returns the component with the given if it exists or nullptr.
        ///Notes:
        ///1: The lifetime of this pointer is guaranteed only until destroyComponent is next called
        ///2: Using the wrong component type for the id is unchecked and undefined behaviour.
        template <ComponentType compType>
        compType* getComponent(const ComponentId id)
        {
            const auto it = componentIdMap.find(id);
            if (it == componentIdMap.end()) {
                return nullptr;
            }
            return &components<compType>[it->second];
        }

        ///INVALIDATING! Calling destroy component immediately invalidates all pointers from getComponent.
        //Technical details of invalidation:
        //The call to std::swap memswaps the last element with the deleted one, potentially invalidating references.
        //To be safe it should be assumed that any call to destroyComponent invalidates ALL pointers from getComponent.
        template <ComponentType compType>
        void destroyComponent(const ComponentId idToDestroy)
        {
            const auto it = componentIdMap.find(idToDestroy);
            //If component doesn't exist, do nothing
            if (it == componentIdMap.end()) {
                return;
            }

            int lastIndex = components<compType>.size() - 1;
            const ComponentId lastCompId = components<compType>[lastIndex].id;
            int destroyedIndex = it->second;

            //In the event that we're destroying the last element, both of these operations are equivalent to a no-op, which
            //means we do not need any conditional logic here.
            std::swap(components<compType>[destroyedIndex], components<compType>[lastIndex]);
            componentIdMap[lastCompId] = destroyedIndex;

            components<compType>.pop_back();
            componentIdMap.erase(idToDestroy);
        }
    };

    ///A convenient stable reference wrapper for holding long-term checked references to components.
    template <ComponentType compType>
    struct CompRef
    {
        inline ComponentId Id() const { return id; }

        constexpr CompRef() {};
        constexpr CompRef(const ComponentId componentId) {id = componentId;}

        constexpr CompRef& operator=(const ComponentId componentId) noexcept {
            id = componentId;
            return *this;
        }

        constexpr operator ComponentId() const noexcept {
            return id;
        }

        ///Returns the referenced component if it exists or nullptr.
        compType* get()
        {
            return ComponentManager::Instance()->getComponent<compType>(id);
        }

        constexpr void reset()
        {
            id = invalidId;
        }

    private:
        ComponentId id = invalidId;
    };
}
