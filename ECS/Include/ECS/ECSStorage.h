#pragma once
#include <tuple>
#include <vector>
#include <limits>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "ECS/Entity.h"
#include "ECS/ComponentPool.h"

template<typename T, typename... Ts>
struct IsOneOf : std::false_type
{
};

template<typename T, typename First, typename... Rest>
struct IsOneOf<T, First, Rest...>
    : std::conditional<std::is_same<T, First>::value, std::true_type, IsOneOf<T, Rest...>>::type
{
};

// --------------------
// compile-time storage
// --------------------
template<typename... Components>
class ECSStorage
{
public:
    ECSStorage()
    {
        _aliveList.push_back(false);
    }

    void Clear()
    {
        _aliveList.clear();
        _aliveList.push_back(false);
        _freeList.clear();
        ClearPools<0>();
    }

    Entity CreateEntity()
    {
        if (!_freeList.empty())
        {
            Entity reused = _freeList.back();
            _freeList.pop_back();
            _aliveList[reused] = true;
            return reused;
        }

        Entity created = static_cast<Entity>(_aliveList.size());
        _aliveList.push_back(true);
        return created;
    }

    void DestroyEntity(Entity entity)
    {
        if (!IsAlive(entity))
            return;

        RemoveAllComponents<0>(entity);
        _aliveList[entity] = false;
        _freeList.push_back(entity);
    }

    bool IsAlive(Entity entity) const
    {
        return entity != InvalidEntity && entity < _aliveList.size() && _aliveList[entity];
    }

    size_t GetEntityCount() const
    {
        size_t count = 0;
        for (size_t i = 0; i < _aliveList.size(); ++i)
        {
            if (_aliveList[i])
                ++count;
        }
        return count;
    }

    std::vector<Entity> GetAliveEntities() const
    {
        std::vector<Entity> result;
        result.reserve(GetEntityCount());

        for (size_t i = 0; i < _aliveList.size(); ++i)
        {
            if (_aliveList[i])
            {
                result.push_back(static_cast<Entity>(i));
            }
        }

        return result;
    }

    template<typename T, typename... Args>
    T& AddComponent(Entity entity, Args&&... args)
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        if (!IsAlive(entity))
        {
            throw std::runtime_error("AddComponent failed: entity is not alive");
        }

        return GetPool<T>().Emplace(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(Entity entity)
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        if (!IsAlive(entity))
        {
            return;
        }

        GetPool<T>().Remove(entity);
    }

    template<typename T>
    bool HasComponent(Entity entity) const
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        if (!IsAlive(entity))
        {
            return false;
        }

        return GetPool<T>().Has(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity)
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        if (!IsAlive(entity))
        {
            throw std::runtime_error("GetComponent failed: entity is not alive");
        }

        ComponentPool<T>& pool = GetPool<T>();
        if (!pool.Has(entity))
        {
            throw std::runtime_error("GetComponent failed: entity does not have requested component");
        }

        return pool.Get(entity);
    }

    template<typename T>
    const T& GetComponent(Entity entity) const
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        if (!IsAlive(entity))
        {
            throw std::runtime_error("GetComponent failed: entity is not alive");
        }

        const ComponentPool<T>& pool = GetPool<T>();
        if (!pool.Has(entity))
        {
            throw std::runtime_error("GetComponent failed: entity does not have requested component");
        }

        return pool.Get(entity);
    }

    template<typename T>
    ComponentPool<T>& GetPool()
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        return std::get<ComponentPool<T>>(_pools);
    }

    template<typename T>
    const ComponentPool<T>& GetPool() const
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");
        return std::get<ComponentPool<T>>(_pools);
    }
    
    template<typename T, typename Func>
    void ForEach(Func&& func)
    {
        static_assert(IsOneOf<T, Components...>::value, "T is not registered in ECSStorage");

        ComponentPool<T>& pool = GetPool<T>();
        const std::vector<Entity>& entities = pool.GetEntities();

        for (size_t i = 0; i < pool.Size(); ++i)
        {
            func(entities[i], pool.GetByDenseIndex(i));
        }
    }

    template<typename First, typename Second, typename... Rest, typename Func>
    void ForEach(Func&& func)
    {
        static_assert(IsOneOf<First, Components...>::value, "First is not registered in ECSStorage");
        static_assert(IsOneOf<Second, Components...>::value, "Second is not registered in ECSStorage");
        static_assert((IsOneOf<Rest, Components...>::value && ...), "Rest are not registered in ECSStorage");

        ComponentPool<First>& firstPool = GetPool<First>();
        const std::vector<Entity>& entities = firstPool.GetEntities();

        for (size_t i = 0; i < firstPool.Size(); ++i)
        {
            Entity entity = entities[i];

            if (!HasComponent<Second>(entity))
                continue;

            if (!(HasComponent<Rest>(entity) && ...))
                continue;

            func(entity, firstPool.GetByDenseIndex(i), GetComponent<Second>(entity), GetComponent<Rest>(entity)...);
        }
    }

private:
    template<size_t I>
    typename std::enable_if<I == sizeof...(Components), void>::type ClearPools()
    {
    }

    template<size_t I>
    typename std::enable_if < I < sizeof...(Components), void>::type ClearPools()
    {
        std::get<I>(_pools).Clear();
        ClearPools<I + 1>();
    }

    template<size_t I>
    typename std::enable_if<I == sizeof...(Components), void>::type RemoveAllComponents(Entity)
    {
    }

    template<size_t I>
    typename std::enable_if < I < sizeof...(Components), void>::type RemoveAllComponents(Entity entity)
    {
        std::get<I>(_pools).Remove(entity);
        RemoveAllComponents<I + 1>(entity);
    }

private:
    std::tuple<ComponentPool<Components>...> _pools;
    std::vector<uint8_t> _aliveList;
    std::vector<Entity> _freeList;
};
