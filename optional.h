#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value) {
        // Сначала конструируем объект в выделенной под него памяти
        value_ptr_ = new (data_) T(value);
        // Затем ставим метку о том, что optional не пуст
        is_initialized_ = true;
    }

    Optional(T&& value) {
        // Конструируем пустой объект нужного типа в выделенной памяти
        value_ptr_ = new (data_) T(std::move(value));
        // Затем ставим метку о том, что optional не пуст
        is_initialized_ = true;
    }

    Optional(const Optional& other) {
        // Если копируемый объект не инициализирован, то нет смысла дальше что-то делать
        if (!other.is_initialized_) {
            return;
        }
        CopyFrom(other);
    }

    Optional(Optional&& other) {
        // Если копируемый объект не инициализирован, то нет смысла дальше что-то делать
        if (!other.is_initialized_) {
            return;
        }
        MoveFrom(other);
    }

    Optional& operator=(const T& value) {
        if (!is_initialized_) {
            // Сначала конструируем объект в выделенной под него памяти
            value_ptr_ = new (data_) T(value);
            // Затем ставим метку о том, что optional не пуст
            is_initialized_ = true;
        } else {
            *value_ptr_ = value;
        }
        return *this;
    }
    Optional& operator=(T&& value) {
        if (!is_initialized_) {
            // Сначала конструируем объект в выделенной под него памяти
            value_ptr_ = new (data_) T(std::move(value));
            // Затем ставим метку о том, что optional не пуст
            is_initialized_ = true;
        } else {
            *value_ptr_ = std::move(value);
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (!rhs.is_initialized_) {
            if (is_initialized_) {
                Reset();
            }
        } else {
            CopyFrom(rhs);
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (!rhs.is_initialized_) {
            if (is_initialized_) {
                Reset();
            }
        } else {
            MoveFrom(rhs);
        }
        return *this;
    }

    template<typename... Values>
    void Emplace(Values&&... values) {
        Reset();
        value_ptr_ = new (data_) T(std::forward<Values&&>(values)...);
        is_initialized_ = true;
    }

    ~Optional() {
        Reset();
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T&& operator*() && {
        return std::move(*value_ptr_);
    }
    T& operator*() & {
        return *value_ptr_;
    }
    const T& operator*() const & {
        return *value_ptr_;
    }
    T* operator->() {
        return value_ptr_;
    }
    const T* operator->() const {
        return value_ptr_;
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T&& Value() && {
        return const_cast<T&&>(
            const_cast<const Optional<T>*>(this)->Value()
        );
    }

    T& Value() & {
        return const_cast<T&>(
            const_cast<const Optional<T>*>(this)->Value()
        );
    }

    const T& Value() const & {
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return *value_ptr_;
    }

    void Reset() {
        if (!is_initialized_) {
            return;
        }
        value_ptr_->~T();
        value_ptr_ = nullptr;
        is_initialized_ = false;
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    T* value_ptr_ = nullptr;
    bool is_initialized_ = false;

    void CopyFrom(const Optional& other) {
        // Если этот объект не инициализирован, создаём и копируем
        // Иначе просто копируем
        // Вариант, что копируемый объект не инициализирован отсекается до входа в этот метод
        if (!is_initialized_) {
            // Конструируем объект, перемещая только что скопированный
            value_ptr_ = new (data_) T(*other.value_ptr_);
            // И теперь ставим метку
            is_initialized_ = true;
        } else {
            *value_ptr_ = *other.value_ptr_;
        }
    }

    void MoveFrom(Optional& other) {
        if (!is_initialized_) {
            // Конструируем объект, перемещая только что скопированный
            value_ptr_ = new (data_) T(std::move(*other.value_ptr_));
            // И теперь ставим метку
            is_initialized_ = true;
        } else {
            *value_ptr_ = std::move(*other.value_ptr_);
        }
    }

};
