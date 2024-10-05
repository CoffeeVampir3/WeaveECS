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
    auto EntityManager = Ecs::EntityManager::Instance();

    auto one = EntityManager
        ->newEntity()
        .addComponent(Whale{.test = 3})
        .addComponent(Snail{.name = "Jim", .whaleRef = 1})
        .addComponent(Cat{.name = "cat", .whaleRef = 3});

    std::print("one {}\n", one.ArchetypeBits());

    auto two = EntityManager
        ->newEntity()
        .addComponent(Whale{.test = 15})
        .addComponent(Snail{.name = "Swag", .whaleRef = 3})
        .addComponent(Cat{.name = "mat", .whaleRef = 3});

    EntityManager->deleteEntity(two.Id());

    two = EntityManager
        ->newEntity()
        .addComponent(Whale{.test = 31})
        .addComponent(Snail{.name = "meeses", .whaleRef = 5})
        .addComponent(Cat{.name = "dead rat", .whaleRef = 5});

    std::print("two {}\n", two.ArchetypeBits());

    auto three = EntityManager
        ->newEntity()
        .addComponent(Whale{.test = 25})
        .addComponent(Snail{.name = "f", .whaleRef = 5});

    for (int i = 0; i < 1000000; i++)
    {
        EntityManager
            ->newEntity()
            .addComponent(Whale{.test = i})
            .addComponent(Snail{.name = "f", .whaleRef = 5});
    }

    std::print("three {}\n", three.ArchetypeBits());

    std::print("\n\n");

    auto start = std::chrono::high_resolution_clock::now();
    for (auto entity : EntityManager->entitiesWith<Whale, Snail>())
    {
        auto [whale, cat] = entity->components<Whale, Snail>();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::print("D: {}\n\n", duration);

    start = std::chrono::high_resolution_clock::now();
    for (auto entity : EntityManager->entitiesWith<Whale, Cat>())
    {
        auto [whale, cat] = entity->components<Whale, Cat>();
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::print("D: {}\n", duration);

    start = std::chrono::high_resolution_clock::now();
    for (auto entity : EntityManager->entitiesWith<Whale, Snail>())
    {
        auto [whale, cat] = entity->components<Whale, Snail>();
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::print("D: {}", duration);
    return -1;
}