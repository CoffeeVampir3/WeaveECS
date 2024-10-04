module;
export module Component;
import std;

export namespace Ecs {
    typedef unsigned long long ComponentId;

    class ComponentManager;
    struct Component
    {
        friend ComponentManager;
        inline ComponentId Id() const { return id; }
    private:
        ComponentId id = 0;
    };

    template<typename T>
    concept ComponentType = std::is_base_of_v<Component, T>;

    //Global because this is illegal within the class scope for some reason unknown to me.
    template <ComponentType compType>
    std::vector<compType> components;

    class ComponentManager
    {
        ComponentId nextId{0};
        std::unordered_map<ComponentId, std::size_t> componentIdMap;
    public:
        static ComponentManager* Instance()
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
        ComponentId addComponent(compType&& newComp)
        {
            ComponentId newComponentId = ++nextId;
            newComp.id = newComponentId;
            components<std::remove_reference_t<compType>>.emplace_back(std::forward<compType>(newComp));
            int lastIndex = components<compType>.size() - 1;

            componentIdMap[newComponentId] = lastIndex;
            return newComponentId;
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
        compType* getComponent(ComponentId id)
        {
            auto it = componentIdMap.find(id);
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
        void destroyComponent(ComponentId idToDestroy)
        {
            auto it = componentIdMap.find(idToDestroy);
            //If component doesn't exist, do nothing
            if (it == componentIdMap.end()) {
                return;
            }

            int lastIndex = components<compType>.size() - 1;
            ComponentId lastCompId = components<compType>[lastIndex].id;
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

        void reset()
        {
            id = 0;
        }

    private:
        ComponentId id = 0;
    };
}
