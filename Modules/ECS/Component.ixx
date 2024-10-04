module;
export module Component;
import std;

export namespace Ecs {
    typedef unsigned long long ComponentId;
    struct Component;
    template<typename T>
    concept ComponentType = std::is_base_of_v<Component, T>;
    struct Component
    {
        ComponentId id;
    };

    template <ComponentType compType>
    std::vector<compType> components;
    class ComponentManager
    {
        ComponentId nextId = 0;
        std::unordered_map<ComponentId, std::size_t> componentIdMap;

    public:
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

        template <ComponentType compType>
        compType* getComponent(ComponentId id)
        {
            auto it = componentIdMap.find(id);
            if (it == componentIdMap.end()) {
                return nullptr;
            }
            return &components<compType>[it->second];
        }

        //Currently this has a chance of invalidating references if we are creating and destroying components.
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

            if (destroyedIndex != lastIndex)
            {
                std::swap(components<compType>[destroyedIndex], components<compType>[lastIndex]);
                componentIdMap[lastCompId] = destroyedIndex;
            }
            components<compType>.pop_back();
            componentIdMap.erase(idToDestroy);
        }
    };
}
