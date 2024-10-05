import std;
import Component;
import Entity;

struct Whale : Ecs::Component
{
    int test = 5;
};

struct Snail : Ecs::Component
{
    std::string name = "Smail";
    Ecs::CompRef<Whale> whaleRef;
};

struct Cat : Ecs::Component
{
    std::string name = "Smail";
    Ecs::CompRef<Whale> whaleRef;
};

int main() {
    auto EntityManager = Ecs::EntityManager();

    auto one = EntityManager
        .newEntity()
        .addComponent(Whale{.test = 3})
        .addComponent(Snail{.name = "Jim", .whaleRef = 1})
        .addComponent(Cat{.name = "cat", .whaleRef = 3});

    std::print("one {}\n", one.ArchetypeBits());

    auto two = EntityManager
        .newEntity()
        .addComponent(Whale{.test = 15})
        .addComponent(Snail{.name = "Swag", .whaleRef = 3})
        .addComponent(Cat{.name = "cat", .whaleRef = 3});

    std::print("two {}\n", two.ArchetypeBits());

    auto three = EntityManager
        .newEntity()
        .addComponent(Whale{.test = 25})
        .addComponent(Snail{.name = "f", .whaleRef = 5});

    std::print("three {}\n", three.ArchetypeBits());

    std::print("\n\n");

    for (auto entity : EntityManager.getEntitiesOf<Whale, Snail>())
    {
        auto [whale, snail] = entity->getComponents<Whale, Snail>();
        std::print("{} {}", whale->test, snail->name);
        std::print("{}\n\n", entity->ArchetypeBits());
    }
    return -1;
}