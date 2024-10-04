import std;
import Component;

struct Whale : Ecs::Component
{
    int test = 5;
};

struct Snail : Ecs::Component
{
    std::string name = "Smail";
};

int main() {
    Ecs::ComponentManager componentManager;
    componentManager.addComponent(std::move(Whale{.test = 2}));
    componentManager.addComponent(std::move(Whale{.test = 7}));
    componentManager.addComponent(std::move(Snail{}));
    componentManager.addComponent(std::move(Snail{.name = "mail"}));

    auto& snails = componentManager.getComponents<Snail>();

    for (auto& snail : snails)
    {
        std::print("\nSnail found with name: {}", snail.name);
        snail.name = "blames";
    }

    std::print("\n\n");

    //componentManager.destroyComponent<Snail>(2);
    //componentManager.destroyComponent<Snail>(2);
    for (auto& snail : snails)
    {
        std::print("\nSnail found with name: {}", snail.name);
    }

    for (auto& whales = componentManager.getComponents<Whale>(); auto& whale : whales)
    {
        std::print("\nWhale found with id: {}", whale.id);
    }

    return -1;
}