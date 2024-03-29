#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include "array_ptr.h"

struct ReserveProxyObj {
    size_t capacity_;
public:
    ReserveProxyObj(size_t capacity) : capacity_(capacity) {}
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
            : SimpleVector(size,
                           Type{})  // Делегируем инициализацию конструктору, принимающему size и value
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
            : items_(size)  // может бросить исключение
            , size_(size)
            , capacity_(size)  //
    {
        std::fill_n(items_.Get() , size, value);  // Может бросить исключение
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : SimpleVector(init.size()) {
        if (std::empty(init)) {
            return;
        }
        std::copy(init.begin(), init.end(), items_.Get());
    }

    SimpleVector(const SimpleVector& other) : SimpleVector(other.GetSize()) {
        if (other.IsEmpty()) {
            return;
        }
        size_t i = 0;
        for (auto it = std::cbegin(other); it != std::cend(other); ++it) {
            items_[i++] = *it;
        }
    }

    SimpleVector(ReserveProxyObj reserv_struct) : capacity_(reserv_struct.capacity_) {
        this ->Reserve(capacity_);
    }


    //move constructor
    SimpleVector(SimpleVector&& other)
        : items_(std::move(other.items_)) {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (*this == rhs) {
            return *this;
        }
        SimpleVector<Type> tmp(rhs);
        this->swap(tmp);

        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index of element is out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index of element is out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > size_ && new_size <= capacity_) {
            for (auto it = begin()+size_; it != begin()+new_size; ++it) {
                *it = Type{};
            }
        }
        if (new_size > capacity_) {
            const size_t new_capacity = std::max(capacity_ * 2, new_size);
            ArrayPtr<Type> new_vector(new_capacity);
            // Копируем существующие элементы вектора на новое место
            std::move(this->begin(), this->end(), new_vector.Get());
            items_.swap(new_vector);
            // заполняем добавленные элементы значением по умолчанию
            //не использую fill так как должно работать с типами с запрещенным
            //копирующим приравниванием
            for (auto it = begin()+size_; it != begin()+new_size; ++it) {
                *it = Type{};
            }
            capacity_ = new_capacity;
        }
        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return begin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cbegin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return cbegin() + size_;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        this->Insert(end(), item);
    }

    //push for rvalue types
    void PushBack(Type&& item) {
        this->Insert(end(), std::move(item));
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());

        auto index = pos - this->cbegin();
        this->Resize(size_+1);
        std::copy_backward(this->cbegin()+index, this->cend()-1, this->end());
        items_[index] = value;

        return &items_[index];
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());

        auto index = pos - this->begin();
        this->Resize(size_+1);
        std::move_backward(this->begin()+index, this->end()-1, this->end());
        items_[index] = std::move(value);

        return &items_[index];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(!IsEmpty());
        assert(pos >= cbegin() && pos < cend());

        std::copy(std::next(pos), this->cend(), const_cast<Iterator>(pos));
        --size_;

        return const_cast<Iterator>(pos);
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(Iterator pos) {
        assert(!IsEmpty());
        assert(pos >= begin() && pos < end());

        std::move(std::next(pos), this->end(), pos);
        --size_;

        return const_cast<Iterator>(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        ArrayPtr<Type> new_vector(new_capacity);
        std::move(this->begin(), this->end(), new_vector.Get());
        items_.swap(new_vector);
        capacity_ = new_capacity;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}