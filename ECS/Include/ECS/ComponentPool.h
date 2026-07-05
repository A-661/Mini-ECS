#pragma once
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "ECS/Entity.h"

// compile-time switch, no effect on runtime
// false = use dense containers | true = use unique_ptr
inline constexpr bool UsePointerComponentStorage = false;

template<typename T>
struct ComponentStorageTraits
{
    using StoredType = std::conditional_t<UsePointerComponentStorage, std::unique_ptr<T>, T>;

    template<typename... Args>
    static StoredType Create(Args&&... args)
    {
        if constexpr (UsePointerComponentStorage)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
        else
        {
            return T(std::forward<Args>(args)...);
        }
    }

    // assigns new component value without changing external API.
    // in pointer-storage mode updates the existing object instead of replacing
    template<typename... Args>
    static T& Assign(StoredType& value, Args&&... args)
    {
        if constexpr (UsePointerComponentStorage)
        {
            if (!value)
            {
                value = Create(std::forward<Args>(args)...);
            }
            else
            {
                *value = T(std::forward<Args>(args)...);
            }

            return *value;
        }
        else
        {
            value = T(std::forward<Args>(args)...);
            return value;
        }
    }

    static T& Get(StoredType& value)
    {
        if constexpr (UsePointerComponentStorage)
        {
            return *value;
        }
        else
        {
            return value;
        }
    }

    static const T& Get(const StoredType& value)
    {
        if constexpr (UsePointerComponentStorage)
        {
            return *value;
        }
        else
        {
            return value;
        }
    }

    static bool IsValid(const StoredType& value)
    {
        if constexpr (UsePointerComponentStorage)
        {
            return value != nullptr;
        }
        else
        {
            return true;
        }
    }
};

// sparse array maps Entity id => dense index.
template<typename T>
class ComponentPool
{
public:
    using StorageTraits = ComponentStorageTraits<T>;
    using StoredType = typename StorageTraits::StoredType;

    void Clear()
    {
        _sparse.clear();
        _denseEntities.clear();
        _denseComponents.clear();
    }

    void Reserve(size_t capacity)
    {
        _denseEntities.reserve(capacity);
        _denseComponents.reserve(capacity);
    }

    bool Has(Entity entity) const
    {
        if (entity >= _sparse.size())
            return false;

        size_t denseIndex = _sparse[entity];
        if (denseIndex == InvalidIndex)
            return false;

        return denseIndex < _denseEntities.size() &&
            _denseEntities[denseIndex] == entity &&
            denseIndex < _denseComponents.size() &&
            StorageTraits::IsValid(_denseComponents[denseIndex]);
    }

    T& Add(Entity entity, const T& value = T{})
    {
        return Emplace(entity, value);
    }

    template<typename... Args>
    T& Emplace(Entity entity, Args&&... args)
    {
        if (entity >= _sparse.size())
            _sparse.resize(static_cast<size_t>(entity) + 1, InvalidIndex);

        if (Has(entity))
        {
            return StorageTraits::Assign(_denseComponents[_sparse[entity]], std::forward<Args>(args)...);
        }

        size_t newIndex = _denseComponents.size();
        _sparse[entity] = newIndex;
        _denseEntities.push_back(entity);
        _denseComponents.emplace_back(StorageTraits::Create(std::forward<Args>(args)...));
        return StorageTraits::Get(_denseComponents.back());
    }

    void Remove(Entity entity)
    {
        if (!Has(entity))
            return;

        size_t removeIndex = _sparse[entity];
        size_t lastIndex = _denseComponents.size() - 1;
        Entity lastEntity = _denseEntities[lastIndex];

        if (removeIndex != lastIndex)
        {
            _denseComponents[removeIndex] = std::move(_denseComponents[lastIndex]);
            _denseEntities[removeIndex] = lastEntity;
            _sparse[lastEntity] = removeIndex;
        }

        _denseComponents.pop_back();
        _denseEntities.pop_back();
        _sparse[entity] = InvalidIndex;
    }

    T& Get(Entity entity)
    {
        if (!Has(entity))
        {
            throw std::out_of_range("ComponentPool::Get failed: component does not exist for entity");
        }

        return StorageTraits::Get(_denseComponents[_sparse[entity]]);
    }

    const T& Get(Entity entity) const
    {
        if (!Has(entity))
        {
            throw std::out_of_range("ComponentPool::Get failed: component does not exist for entity");
        }

        return StorageTraits::Get(_denseComponents[_sparse[entity]]);
    }

    T& GetByDenseIndex(size_t index)
    {
        return StorageTraits::Get(_denseComponents[index]);
    }

    const T& GetByDenseIndex(size_t index) const
    {
        return StorageTraits::Get(_denseComponents[index]);
    }

    size_t Size() const
    {
        return _denseComponents.size();
    }

    const std::vector<Entity>& GetEntities() const
    {
        return _denseEntities;
    }

    std::vector<StoredType>& GetDense()
    {
        return _denseComponents;
    }

    const std::vector<StoredType>& GetDense() const
    {
        return _denseComponents;
    }

private:
    std::vector<size_t> _sparse;
    std::vector<Entity> _denseEntities;
    std::vector<StoredType> _denseComponents;
};
