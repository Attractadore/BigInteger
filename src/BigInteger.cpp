#include "BigInteger.hpp"

#include <cassert>
#include <iomanip>

BigInteger::BigInteger() {
    this->zero();
}

BigInteger& BigInteger::operator+=(BigInteger const& other) {
    assert(this->data.size() and other.data.size());

    const bool thisNegative = this->bNegative();
    const bool otherNegative = other.bNegative();

    const dataType thisFillByte = this->getFillByte();
    const dataType otherFillByte = other.getFillByte();

    if (this->data.size() < other.data.size()) {
        this->data.resize(other.data.size(), thisFillByte);
    }

    dataType carryOver = 0;
    std::size_t maxSize = std::max(this->data.size(), other.data.size());
    for (std::size_t i = 0; i < maxSize; i++) {
        assert(i < this->data.size());

        dataType lv = this->data[i];
        dataType rv = (i < other.data.size()) ? (other.data[i]) : (otherFillByte);
        this->data[i] += rv + carryOver;

        dataType lTopBit = lv >> (dataTypeBits - 1);
        dataType rTopBit = rv >> (dataTypeBits - 1);
        dataType bitMask = ~(1ull << (dataTypeBits - 1));
        lv &= bitMask;
        rv &= bitMask;
        dataType resTopBit = (lv + rv + carryOver) >> (dataTypeBits - 1);
        carryOver = (lTopBit + rTopBit + resTopBit) >> 1;
    }

    const bool resNegative = this->bNegative();
    if ((thisNegative or otherNegative) != resNegative) {
        this->data.push_back(thisFillByte);
    }

    return *this;
}

BigInteger& BigInteger::operator-=(BigInteger const& other) {
    return *this += (-other);
}

BigInteger& BigInteger::operator*=(BigInteger const& other) {
    if (other.bNegative()) {
        this->negate();
        return *this *= -other;
    }

    BigInteger accumulator = *this;
    this->zero();
    std::size_t bitsShift = 0;
    for (std::size_t i = 0; i < other.numBits(); i++, bitsShift++) {
        if (other[i]) {
            accumulator <<= bitsShift;
            *this += accumulator;
            bitsShift = 0;
        }
    }

    return *this;
}

std::tuple<BigInteger, BigInteger> quotRem(BigInteger leftBI, BigInteger rightBI) {
    const bool resNegative = leftBI.bNegative() != rightBI.bNegative();
    if (leftBI.bNegative()) {
        leftBI.negate();
    }
    if (rightBI.bNegative()) {
        rightBI.negate();
    }

    BigInteger partQuotBI = 1;
    while (rightBI <= leftBI) {
        rightBI <<= 1;
        partQuotBI <<= 1;
    }

    BigInteger quotBI = 0;
    while (partQuotBI > 0) {
        while (rightBI > leftBI) {
            rightBI >>= 1;
            partQuotBI >>= 1;
        }
        if (partQuotBI == 0) {
            break;
        }
        leftBI -= rightBI;
        quotBI += partQuotBI;
    }

    if (resNegative) {
        quotBI.negate();
        leftBI.negate();
    }

    return {quotBI, leftBI};
}

std::tuple<BigInteger&, BigInteger> BigInteger::divmod(BigInteger const& other) {
    BigInteger mod;
    return (std::tie(*this, mod) = quotRem(*this, other));
}

std::tuple<BigInteger, BigInteger&> BigInteger::moddiv(BigInteger const& other) {
    BigInteger quot;
    return (std::tie(quot, *this) = quotRem(*this, other));
}

BigInteger& BigInteger::operator/=(BigInteger const& other) {
    this->divmod(other);
    return *this;
}

BigInteger& BigInteger::operator%=(BigInteger const& other) {
    this->moddiv(other);
    return *this;
}

BigInteger operator~(BigInteger bgIntgr) {
    return bgIntgr.flip();
}

template <typename F>
    requires std::invocable<F, BigInteger::dataType, BigInteger::dataType>
BigInteger& BigInteger::binaryOperationCommon(BigInteger const& other, F const& op) {
    const BigInteger::dataType thisFillByte = this->getFillByte();
    const BigInteger::dataType otherFillByte = other.getFillByte();

    if (this->data.size() < other.data.size()) {
        this->data.resize(other.data.size(), thisFillByte);
    }

    auto tit = this->data.begin();
    auto oit = other.data.cbegin();
    for (; oit < other.data.cend(); ++tit, ++oit) {
        assert(tit < this->data.end());

        *tit = op(*tit, *oit);
    }
    std::transform(tit, this->data.end(), tit, [&op, &otherFillByte](const dataType val) { return op(val, otherFillByte); });

    return *this;
}

BigInteger& BigInteger::operator&=(BigInteger const& other) {
    return this->binaryOperationCommon(other, std::bit_and<BigInteger::dataType>());
}

BigInteger& BigInteger::operator|=(BigInteger const& other) {
    return this->binaryOperationCommon(other, std::bit_or<BigInteger::dataType>());
}

BigInteger& BigInteger::operator^=(BigInteger const& other) {
    return this->binaryOperationCommon(other, std::bit_xor<BigInteger::dataType>());
}

BigInteger& BigInteger::operator>>=(const std::size_t bits) {
    const bool thisNegative = this->bNegative();

    const dataType thisFillByte = this->getFillByte();

    const std::size_t bytesShift = bits / dataTypeBits;
    if (bytesShift) {
        if (bytesShift < this->data.size()) {
            std::move(this->data.begin() + bytesShift, this->data.end(), this->data.begin());
            this->data.resize(this->data.size() - bytesShift);
        } else {
            return this->zero();
        }
    }

    dataType carryOver = 0;
    const std::size_t bitsShift = bits % dataTypeBits;
    if (bitsShift) {
        std::transform(this->data.rbegin(), this->data.rend(), this->data.rbegin(),
                       [&carryOver, &bitsShift](dataType val) {
                           const dataType tCarryOver = val << (dataTypeBits - bitsShift);
                           val = (val >> bitsShift) | carryOver;
                           carryOver = tCarryOver;
                           return val;
                       });
        this->data.back() |= thisFillByte << (dataTypeBits - bits);
    }

    return *this;
}

BigInteger& BigInteger::operator<<=(const std::size_t bits) {
    const bool thisNegative = this->bNegative();
    const dataType thisFillByte = this->getFillByte();

    const std::size_t bytesShift = bits / dataTypeBits;
    if (bytesShift) {
        this->data.resize(this->data.size() + bytesShift);
        std::move(this->data.rbegin() + bytesShift, this->data.rend(), this->data.rbegin());
        std::fill(this->data.begin(), this->data.begin() + bytesShift, 0);
    }

    dataType carryOver = 0;
    const std::size_t bitsShift = bits % dataTypeBits;
    if (bitsShift) {
        std::transform(this->data.begin(), this->data.end(), this->data.begin(),
                       [&carryOver, &bitsShift](dataType val) {
                           const dataType tCarryOver = val >> (dataTypeBits - bitsShift);
                           val = (val << bitsShift) | carryOver;
                           carryOver = tCarryOver;
                           return val;
                       });
        const bool resNegative = this->bNegative();
        if (thisNegative != resNegative or carryOver != (thisFillByte >> (dataTypeBits - bitsShift))) {
            this->data.push_back(carryOver | (thisFillByte << bitsShift));
        }
    }

    return *this;
}

BigInteger operator-(BigInteger bgIntgr) {
    return bgIntgr.negate();
}

bool operator==(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept {
    const bool bLeftNegative = leftBgIntgr.bNegative();
    const bool bRightNegative = rightBgIntgr.bNegative();

    if (bLeftNegative != bRightNegative) {
        return false;
    }

    const BigInteger::dataType leftFillByte = leftBgIntgr.getFillByte();
    const BigInteger::dataType rightFillByte = rightBgIntgr.getFillByte();

    std::size_t maxSize = std::max(leftBgIntgr.data.size(), rightBgIntgr.data.size());
    for (std::size_t i = 0; i < maxSize; i++) {
        const BigInteger::dataType lv = (i < leftBgIntgr.data.size()) ? (leftBgIntgr.data[i]) : leftFillByte;
        const BigInteger::dataType rv = (i < rightBgIntgr.data.size()) ? (rightBgIntgr.data[i]) : rightFillByte;

        if (lv != rv) {
            return false;
        }
    }

    return true;
}

std::weak_ordering operator<=>(BigInteger const& leftBgIntgr, BigInteger const& rightBgIntgr) noexcept {
    const bool bLeftNegative = leftBgIntgr.bNegative();
    const bool bRightNegative = rightBgIntgr.bNegative();

    if (bLeftNegative and !bRightNegative) {
        return std::weak_ordering::less;
    }
    if (!bLeftNegative and bRightNegative) {
        return std::weak_ordering::greater;
    }

    const BigInteger::dataType leftFillByte = leftBgIntgr.getFillByte();
    const BigInteger::dataType rightFillByte = rightBgIntgr.getFillByte();

    std::size_t maxSize = std::max(leftBgIntgr.data.size(), rightBgIntgr.data.size());
    for (std::size_t i = 0; i < maxSize; i++) {
        const BigInteger::dataType lv = (i < leftBgIntgr.data.size()) ? (leftBgIntgr.data[i]) : leftFillByte;
        const BigInteger::dataType rv = (i < rightBgIntgr.data.size()) ? (rightBgIntgr.data[i]) : rightFillByte;

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
                  [&os](const BigInteger::dataType v) {
                      os << std::internal << std::setw(BigInteger::dataTypeBits / 4) << BigInteger::dataType(v);
                  });

    os.setf(flags);

    return os;
}

BigInteger operator+(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr += rightBgIntgr;
}

BigInteger operator-(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr -= rightBgIntgr;
}

BigInteger operator*(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr *= rightBgIntgr;
}

BigInteger operator/(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr /= rightBgIntgr;
}

BigInteger operator%(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr %= rightBgIntgr;
}

BigInteger operator>>(BigInteger bgIntgr, const std::size_t bits) {
    return bgIntgr >>= bits;
}

BigInteger operator<<(BigInteger bgIntgr, const std::size_t bits) {
    return bgIntgr <<= bits;
}

BigInteger operator&(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr &= rightBgIntgr;
}

BigInteger operator|(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr |= rightBgIntgr;
}

BigInteger operator^(BigInteger leftBgIntgr, BigInteger const& rightBgIntgr) {
    return leftBgIntgr ^= rightBgIntgr;
}

std::size_t BigInteger::numBits() const noexcept {
    return this->data.size() * dataTypeBits;
}

bool BigInteger::bNegative() const noexcept {
    assert(this->data.size());
    return this->data.back() & (1ull << (dataTypeBits - 1));
}

BigInteger& BigInteger::negate() {
    this->flip();
    return *this += 1;
}

BigInteger& BigInteger::flip() {
    std::transform(this->data.begin(), this->data.end(), this->data.begin(), [](auto val) { return ~val; });
    return *this;
}

bool BigInteger::operator[](const std::size_t i) const noexcept {
    const std::size_t byteI = i / dataTypeBits;
    const std::size_t bitI = i % dataTypeBits;
    return (byteI < this->data.size()) ? (this->data[byteI] & (1ull << bitI)) : (this->bNegative());
}

BigInteger::dataType BigInteger::getFillByte() const noexcept {
    return dataType(0 - this->bNegative());
}

BigInteger& BigInteger::zero() {
    this->data.clear();
    this->data.push_back(0);
    return *this;
}
