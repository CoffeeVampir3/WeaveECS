import std;
import Component;

struct Whale : Ecs::Component
{
    int test = 5;
};

struct Snail : Ecs::Component
{
    std::string name = "Smail";
    Ecs::CompRef<Whale> whaleRef;
};

int main() {
    auto componentManager = Ecs::ComponentManager::Instance();
    componentManager->addComponent(Whale{.test = 31});
    componentManager->addComponent(Whale{.test = 5});
    componentManager->addComponent(Snail{.whaleRef = 2});
    componentManager->addComponent(Snail{.name = "mail"});

    auto& snails = componentManager->getComponents<Snail>();

    auto krale = Snail{.whaleRef = 2};

    for (auto& snail : snails)
    {
        std::print("\nSnail found with name: {} {}", snail.name, snail.Id());
        std::print("\nSnail ref id is: {}", snail.whaleRef.Id());

        if (snail.whaleRef == krale.whaleRef)
        {
            std::print("Equal");
        }

        if (snail.whaleRef != krale.whaleRef)
        {
            std::print("Not equal");
        }

        if (auto theWhale = snail.whaleRef.get())
        {
            std::print("Had whale, id: {} Whale test: {}", theWhale->Id(), theWhale->test);
        } else
        {
            std::print("Snail did not have whale");
        }
    }

    std::print("\n\n");

    componentManager->destroyComponent<Snail>(3);
    componentManager->addComponent(Snail{.name = "pail"});
    componentManager->destroyComponent<Snail>(4);
    componentManager->addComponent(Snail{.name = "Quale"});
    componentManager->addComponent(Snail{.name = "meeal"});
    componentManager->addComponent(Whale{.test = 8});
    for (auto& snail : snails)
    {
        std::print("\nSnail found with name: {}", snail.name);
    }

    for (auto& whales = componentManager->getComponents<Whale>(); auto& whale : whales)
    {
        std::print("\nWhale found with id: {}", whale.Id());
    }

    return -1;
}