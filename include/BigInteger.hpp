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

    BigInteger& negate();
    BigInteger& flip();
    BigInteger& zero();

private:
    using dataType = unsigned long long;
    static const std::size_t dataTypeSize = sizeof(dataType);
    static const std::size_t dataTypeBits = dataTypeSize * CHAR_BIT;

    std::vector<dataType> data;

    void copy(std::integral auto const& intgr);
    void copy(std::unsigned_integral auto const& intgr);
    BigInteger::dataType getFillByte() const noexcept;

    template<typename F>
    requires std::invocable<F, dataType, dataType>
    BigInteger& binaryOperationCommon(BigInteger const& other, F const& op);

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
    BigInteger& operator>>=(const std::size_t bits);
    BigInteger& operator<<=(const std::size_t bits);
    BigInteger& operator&=(BigInteger const& other);
    BigInteger& operator|=(BigInteger const& other);
    BigInteger& operator^=(BigInteger const& other);

    friend std::ostream& operator<<(std::ostream& os, BigInteger const& bgIntgr);

    friend std::weak_ordering operator<=>(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
    friend bool operator==(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
    friend bool operator!=(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept;
};

BigInteger operator+(BigInteger bgIntgr);
BigInteger operator-(BigInteger bgIntgr);
BigInteger operator--(BigInteger& bgIntgr, int);
BigInteger operator++(BigInteger& bgIntgr, int);
BigInteger operator~(BigInteger bgIntgr);

BigInteger operator+(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator-(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator*(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator/(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator%(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator>>(BigInteger bgIntgr, const std::size_t bits);
BigInteger operator<<(BigInteger bgIntgr, const std::size_t bits);
BigInteger operator&(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator|(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);
BigInteger operator^(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr);

BigInteger::BigInteger(std::integral auto const& intgr) {
    this->copy(intgr);   
}

BigInteger& BigInteger::operator=(std::integral auto const& intgr) {
    this->copy(intgr);
    return *this;
}

void BigInteger::copy(std::integral auto const& intgr) {
    this->data = {intgr};
}

void BigInteger::copy(std::unsigned_integral auto const& intgr) {
    this->data = {intgr, 0};
}

