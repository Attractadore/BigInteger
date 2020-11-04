#pragma once

#include <concepts>
#include <vector>
#include <climits>
#include <iostream>

class BigInteger {
public:
    BigInteger(std::integral auto const& intgr);
    BigInteger();

    std::size_t numBits() const noexcept;
    bool bNegative() const noexcept;

private:
    using dataType = unsigned char;
    using operationType = unsigned long long;
    static_assert(sizeof(operationType) > sizeof(dataType));

    std::vector<dataType> data;

    void copy(std::integral auto const& intgr);
    void setZero();

public:
    bool operator[](const std::size_t i) const noexcept;

    BigInteger& operator=(std::integral auto const& intgr);

    BigInteger& operator++();
    BigInteger& operator--();
    BigInteger& operator+=(BigInteger const& other);
    BigInteger& operator-=(BigInteger const& other);
    BigInteger& operator*=(BigInteger const& other);
    BigInteger& operator/=(BigInteger const& other);
    BigInteger& operator%=(BigInteger const& other);
    BigInteger& operator>>=(std::integral auto const& bits);
    BigInteger& operator<<=(std::integral auto const& bits);
    BigInteger& operator&=(BigInteger const& other);
    BigInteger& operator|=(BigInteger const& other);
    BigInteger& operator^=(BigInteger const& other);

    friend BigInteger operator+(BigInteger bgIntgr);
    friend BigInteger operator-(BigInteger bgIntgr);
    friend BigInteger operator--(BigInteger& bgIntgr, int);
    friend BigInteger operator++(BigInteger& bgIntgr, int);
    friend BigInteger operator~(BigInteger bgIntgr);

    friend std::ostream& operator<<(std::ostream& os, BigInteger const& bgIntgr);

    friend std::weak_ordering operator<=>(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
    friend std::weak_ordering operator<=>(BigInteger const& bgIntgr, std::integral auto const& intgr) noexcept;
    friend bool operator==(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
    friend bool operator==(BigInteger const& bgIntgr, std::integral auto const& intgr) noexcept;
    friend bool operator!=(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
    friend bool operator!=(BigInteger const& bgIntgr, std::integral auto const& intgr) noexcept;
};

inline BigInteger operator+(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr += rightBgIntgr;
}

inline BigInteger operator-(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr -= rightBgIntgr;
}

inline BigInteger operator*(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr *= rightBgIntgr;
}

inline BigInteger operator/(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr /= rightBgIntgr;
}

inline BigInteger operator%(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr %= rightBgIntgr;
}

inline BigInteger operator>>(BigInteger bgIntgr, std::integral auto const& bits) {
    return bgIntgr >>= bits;
}

inline BigInteger operator<<(BigInteger bgIntgr, std::integral auto const& bits) {
    return bgIntgr <<= bits;
}

inline BigInteger operator&(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr &= rightBgIntgr;
}

inline BigInteger operator|(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr |= rightBgIntgr;
}

inline BigInteger operator^(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr ^= rightBgIntgr;
}

BigInteger::BigInteger(std::integral auto const& intgr) {
    this->copy(intgr);   
}

BigInteger& BigInteger::operator=(std::integral auto const& intgr) {
    this->copy(intgr);
    return *this;
}

BigInteger& BigInteger::operator>>=(std::integral auto const& bits) {
    using bitsType = decltype(bits);
    const bool bNeg = this->bNegative();

    const bitsType bytesShift = bits / CHAR_BIT;
    if (bytesShift) {
        if (bytesShift < this->data.size()) {
            std::move(this->data.begin() + bytesShift, this->data.end(), this->data.begin());
            this->data.resize(this->data.size() - bytesShift);
        }
        else {
            this->setZero();
        }
    }

    operationType carryOver = 0;
    const bitsType bitsShift = bits % CHAR_BIT;
    if (bitsShift) {
        if (bNeg) {
            this->data.push_back(-1);
        }
        else {
            this->data.push_back(0);
        }
        std::for_each(this->data.rbegin(), this->data.rend(),
                      [&carryOver, &bitsShift](BigInteger::dataType& v) {
                          const operationType tCarryOver = v << (CHAR_BIT - bitsShift);
                          v = (v >> bitsShift) | carryOver;
                          carryOver = tCarryOver;
                      });
        if (bNeg) {
            this->data.back() |= 1ull << (sizeof(BigInteger::dataType) * CHAR_BIT - bits);
        }
    }

    return *this;
}

BigInteger& BigInteger::operator<<=(std::integral auto const& bits) {
    using bitsType = decltype(bits);
    const bool bNeg = this->bNegative();

    const bitsType bytesShift = bits / CHAR_BIT;
    if (bytesShift) {
        this->data.resize(this->data.size() + bytesShift);
        std::move(this->data.rbegin() + bytesShift, this->data.rend(), this->data.rbegin());
        std::fill(this->data.begin(), this->data.begin() + bytesShift, 0);
    }

    operationType carryOver = 0;
    const bitsType bitsShift = bits % CHAR_BIT;
    if (bitsShift) {
        for (auto& v: this->data) {
            const operationType tCarryOver = v >> (CHAR_BIT - bitsShift);
            v = (v << bitsShift) | carryOver;
            carryOver = tCarryOver;
        }
        this->data.push_back(carryOver);
        if (bNeg) {
            this->data.back() |= 1ull << (sizeof(BigInteger::dataType) * CHAR_BIT - bits);
        }
    }

    return *this;
}

bool operator==(BigInteger const& bgIntgr, std::integral auto const& intgr) noexcept {
    // TODO: replace i?
    for (std::size_t i = 0; i < bgIntgr.data.size(); i++) {
        const BigInteger::operationType lv = (i < bgIntgr.data.size()) ? (bgIntgr.data[i]) : 0;
        const BigInteger::operationType rv = BigInteger::dataType(intgr >> (i * CHAR_BIT));
        if (lv != rv) {
            return false;
        }
    }
    return true;
}

std::weak_ordering operator<=>(BigInteger const& bgIntgr, std::integral auto const& intgr) noexcept {
    // TODO: replace i?
    for (std::size_t i = 0; i < bgIntgr.data.size(); i++) {
        const BigInteger::operationType lv = (i < bgIntgr.data.size()) ? (bgIntgr.data[i]) : 0;
        const BigInteger::operationType rv = BigInteger::dataType(intgr >> (i * CHAR_BIT));
        if (lv < rv) {
            return std::weak_ordering::less;
        }
        if (lv > rv) {
            return std::weak_ordering::greater;
        }
    }
    return std::weak_ordering::equivalent;

}

void BigInteger::copy(std::integral auto const& intgr) {
    this->data.clear();
    for (std::size_t i = 0; i < sizeof(intgr); i++) {
        this->data.push_back(intgr >> (i * CHAR_BIT));
    }
    if (this->bNegative() and intgr > 0) {
        this->data.push_back(0);
    }
}

