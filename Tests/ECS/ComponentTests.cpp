//
// Created by blackroot on 10/4/24.
//
import std;
import Component;
import Logging;

struct Whale : Ecs::Component
{
    int test = 5;
};

struct Snail : Ecs::Component
{
    std::string name = "Smail";
    Ecs::CompRef<Whale> whaleRef;
};

void componentsGeneralUsage() {
    auto componentManager = Ecs::ComponentManager::Instance();
    componentManager->addComponent(1, Whale{.test = 31});
    componentManager->addComponent(1, Whale{.test = 5});
    componentManager->addComponent(1, Snail{.whaleRef = 2});
    componentManager->addComponent(1, Snail{.name = "mail"});

    auto krale = Snail{.whaleRef = 2, .name = "banana"};
    componentManager->addComponent(1, std::move(krale));

    auto snailCount = componentManager->count<Snail>();
    auto whaleCount = componentManager->count<Whale>();

    Logging::assert(snailCount == 3, "Added 3 snails but componentManager snail count was not equal to 3");
    Logging::assert(whaleCount == 2, "Added 2 whales but componentManager whale count was not equal to 2");

    auto& snails = componentManager->getComponents<Snail>();
    for (auto& snail : snails)
    {
        if (snail.name == "banana")
        {
            Logging::assert(snail.whaleRef == 2, "Snail whale ref should be 2");
        }
    }
    krale.whaleRef = componentManager->getComponent<Whale>(1)->Id();
    Logging::assert(krale.whaleRef == 1, "Krale whale ref should be 1");
    krale.whaleRef = componentManager->getComponent<Whale>(3)->Id();
    Logging::assert(krale.whaleRef != 0, "Krale whale ref should be invalid (0) but was not");

    auto& whales = componentManager->getComponents<Whale>();
    for (auto& whale : whales)
    {
        whale.test = 13;
    }
    whales = componentManager->getComponents<Whale>();
    for (auto& whale : whales)
    {
        Logging::assert(whale.test == 13, "All whales should have have test value equal to 13");
    }

    componentManager->clear<Whale>();
    componentManager->clear<Snail>();

    snailCount = componentManager->count<Snail>();
    whaleCount = componentManager->count<Whale>();

    Logging::assert(snailCount == 0, "Snails were cleared but not count is not zero");
    Logging::assert(whaleCount == 0, "Whales were cleared but not count is not zero");

    Logging::assert(componentManager->testing_getComponentIdMapCount() == 0, "Component id map still has entries at the end of test.");
}

int main()
{
    componentsGeneralUsage();
    return 0;
}
