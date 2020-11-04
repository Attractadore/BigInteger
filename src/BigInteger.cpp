#include "BigInteger.hpp"

#include <cassert>
#include <iomanip>

BigInteger::BigInteger() {
    this->setZero();
}

BigInteger& BigInteger::operator+=(BigInteger const& other) {
    assert(this->data.size() and other.data.size());

    const bool thisNegative = this->bNegative();
    const bool otherNegative = other.bNegative();

    const BigInteger::dataType thisFillByte = 0 - thisNegative;
    const BigInteger::dataType otherFillByte = 0 - otherNegative;

    if (this->data.size() < other.data.size()) {
        this->data.resize(other.data.size(), thisFillByte);
    }

    operationType carryOver = 0;
    auto lit = this->data.begin();
    auto rit = other.data.cbegin();
    for (;lit < this->data.end() or rit < other.data.cend(); ++lit, ++rit) {
        assert(lit >= this->data.begin() and lit < this->data.end());

        const operationType lv = *lit;
        const operationType rv = (rit < other.data.cend()) ? (*rit) : (otherFillByte);
        *lit += rv + carryOver;

        carryOver = (lv + rv + carryOver) >> CHAR_BIT;
    }


    const bool resNegative = this->bNegative();
    if ((thisNegative or otherNegative) != resNegative) {
        // Overflow has happened
        this->data.push_back(thisFillByte);
    }

    return *this;
}

BigInteger& BigInteger::operator*=(BigInteger const& other) {
    // TODO: maybe there's a better way?
    BigInteger accumulator = (other.bNegative()) ? (-(*this)) : (*this);
    const BigInteger negOther = (other.bNegative()) ? (-other) : 0;
    BigInteger const& check = (other.bNegative()) ? (negOther) : other;
    this->setZero();
    for (size_t i = 0; i < check.numBits(); i++) {
        if (check[i]) {
            *this += accumulator;
        }
        accumulator <<= 1;
    }
    return *this;
}

BigInteger operator~(BigInteger bgIntgr) {
    std::transform(bgIntgr.data.begin(), bgIntgr.data.end(), bgIntgr.data.begin(), [](BigInteger::dataType val) {return ~val;});
    return bgIntgr;
}

BigInteger& BigInteger::operator&=(BigInteger const& other) {
    if (this->data.size() > other.data.size()) {
        this->data.resize(other.data.size());
    }
    auto tit = this->data.begin();
    auto oit = other.data.cbegin();
    for (; tit < this->data.end() and oit < other.data.cend(); ++tit, ++oit) {
        *tit &= *oit;
    }
    return *this;
}

BigInteger& BigInteger::operator|=(BigInteger const& other) {
    auto tit = this->data.begin();
    auto oit = other.data.cbegin();
    for (; tit < this->data.end() and oit < other.data.cend(); ++tit, ++oit) {
        *tit |= *oit;
    }
    if (oit < this->data.cend()) {
        std::copy(oit, other.data.end(), std::back_inserter(this->data));
    }
    return *this;
}

BigInteger& BigInteger::operator^=(BigInteger const& other) {
    auto tit = this->data.begin();
    auto oit = other.data.cbegin();
    for (; tit < this->data.end() and oit < other.data.cend(); ++tit, ++oit) {
        *tit ^= *oit;
    }
    if (oit < this->data.cend()) {
        std::copy(oit, other.data.end(), std::back_inserter(this->data));
    }
    return *this;
}

BigInteger operator-(BigInteger bgIntgr) {
    return ~bgIntgr + 1;
}

bool operator==(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept {
    for (auto lit = leftBgIntgr.data.cbegin(), rit = rightBgIntgr.data.cend();
         lit < leftBgIntgr.data.cend() and rit < rightBgIntgr.data.cend();
         ++lit, ++rit) {
        const BigInteger::operationType lv = (lit < leftBgIntgr.data.cend()) ? (*lit) : 0;
        const BigInteger::operationType rv = (rit < rightBgIntgr.data.cend()) ? (*rit) : 0;
        if (lv != rv) {
            return false;
        }
    }
    return true;
}

std::weak_ordering operator<=>(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept 
{
    for (auto lit = leftBgIntgr.data.cbegin(), rit = rightBgIntgr.data.cend();
         lit < leftBgIntgr.data.cend() and rit < rightBgIntgr.data.cend();
         ++lit, ++rit) {
        const BigInteger::operationType lv = (lit < leftBgIntgr.data.cend()) ? (*lit) : 0;
        const BigInteger::operationType rv = (rit < rightBgIntgr.data.cend()) ? (*rit) : 0;
        if (lv < rv) {
            return std::weak_ordering::less;
        }
        if (lv > rv) {
            return std::weak_ordering::greater;
        }
    }
    return std::weak_ordering::equivalent;
}

std::ostream& operator<<(std::ostream& os, BigInteger const& bgIntgr) {
    auto flags = os.flags();
    os << std::setfill('0') << std::hex << "0x";

    std::for_each(bgIntgr.data.crbegin(), bgIntgr.data.crend(),
                  [&os](unsigned char v) {
                      os << std::internal << std::setw(CHAR_BIT / 4) << int(v);
                  });

    os.setf(flags);

    return os;
}

std::size_t BigInteger::numBits() const noexcept{
    return this->data.size() * CHAR_BIT;
}

bool BigInteger::bNegative() const noexcept {
    assert(this->data.size());
    return this->data.back() & (1ull << (sizeof(BigInteger::dataType) * CHAR_BIT - 1));
}

bool BigInteger::operator[](const std::size_t i) const noexcept{
    const std::size_t byteI = i / CHAR_BIT;
    const std::size_t bitI = i % CHAR_BIT;
    if (byteI < this->data.size()) {
        return this->data[byteI] & (1 << bitI);
    }
    return 0;
}

void BigInteger::setZero() {
    this->data.clear();
    this->data.push_back(0);
}
