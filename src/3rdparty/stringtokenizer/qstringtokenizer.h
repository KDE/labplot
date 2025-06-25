/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marc Mutz <marc.mutz@kdab.com>

  SPDX-License-Identifier: MIT
*/
#ifndef QSTRINGTOKENIZER_H
#define QSTRINGTOKENIZER_H

#include <QtCore/qnamespace.h>

QT_BEGIN_NAMESPACE

template<typename, typename>
class QStringBuilder;
template<typename>
class QVector;

QT_END_NAMESPACE

#if defined(Q_QDOC) || (defined(__cpp_range_based_for) && __cpp_range_based_for >= 201603)
#define Q_STRINGTOKENIZER_USE_SENTINEL
#endif

class QStringTokenizerBaseBase
{
protected:
    ~QStringTokenizerBaseBase() = default;
    Q_DECL_CONSTEXPR QStringTokenizerBaseBase(Qt::SplitBehavior sb, Qt::CaseSensitivity cs) noexcept
        : m_sb{sb}
        , m_cs{cs}
    {
    }

    struct tokenizer_state
    {
        qsizetype start, end, extra;
        friend constexpr bool operator==(tokenizer_state lhs,
                                         tokenizer_state rhs) noexcept // clazy:exclude=function-args-by-ref
        {
            return lhs.start == rhs.start && lhs.end == rhs.end && lhs.extra == rhs.extra;
        }
        friend constexpr bool operator!=(tokenizer_state lhs,
                                         tokenizer_state rhs) noexcept // clazy:exclude=function-args-by-ref
        {
            return !operator==(lhs, rhs);
        }
    };

    Qt::SplitBehavior m_sb;
    Qt::CaseSensitivity m_cs;
};

template<typename Haystack, typename Needle>
class QStringTokenizerBase : protected QStringTokenizerBaseBase
{
    struct next_result
    {
        Haystack value;
        bool ok;
        tokenizer_state state;
    };
    inline next_result next(tokenizer_state state) const noexcept;
    inline next_result toFront() const noexcept { return next({}); }

public:
    constexpr explicit QStringTokenizerBase(Haystack haystack, Needle needle, Qt::SplitBehavior sb,
                                            Qt::CaseSensitivity cs) noexcept
        : QStringTokenizerBaseBase{sb, cs}
        , m_haystack{haystack}
        , m_needle{needle}
    {
    }

    class iterator;
    friend class iterator;
#ifdef Q_STRINGTOKENIZER_USE_SENTINEL
    class sentinel
    {
        friend constexpr bool operator==(sentinel, sentinel) noexcept { return true; }
        friend constexpr bool operator!=(sentinel, sentinel) noexcept { return false; }
    };
#else
    using sentinel = iterator;
#endif
    class iterator
    {
        const QStringTokenizerBase *tokenizer;
        next_result current;
        friend class QStringTokenizerBase;
        explicit iterator(const QStringTokenizerBase &t) noexcept
            : tokenizer{&t}
            , current{t.toFront()}
        {
        }

    public:
        using difference_type = qsizetype;
        using value_type = Haystack;
        using pointer = const value_type *;
        using reference = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        iterator() noexcept = default;

        // violates std::forward_iterator (returns a reference into the iterator)
        Q_REQUIRED_RESULT constexpr const Haystack *operator->() const { return Q_ASSERT(current.ok), &current.value; }
        Q_REQUIRED_RESULT constexpr const Haystack &operator*() const { return *operator->(); }

        iterator &operator++()
        {
            advance();
            return *this;
        }
        iterator operator++(int)
        {
            auto tmp = *this;
            advance();
            return tmp;
        }

        friend constexpr bool operator==(const iterator &lhs, const iterator &rhs) noexcept
        {
            return lhs.current.ok == rhs.current.ok && (!lhs.current.ok || (Q_ASSERT(lhs.tokenizer == rhs.tokenizer),
                                                                            lhs.current.state == rhs.current.state));
        }
        friend constexpr bool operator!=(const iterator &lhs, const iterator &rhs) noexcept
        {
            return !operator==(lhs, rhs);
        }
#ifdef Q_STRINGTOKENIZER_USE_SENTINEL
        friend constexpr bool operator==(const iterator &lhs, sentinel) noexcept { return !lhs.current.ok; }
        friend constexpr bool operator!=(const iterator &lhs, sentinel) noexcept
        {
            return !operator==(lhs, sentinel{});
        }
        friend constexpr bool operator==(sentinel, const iterator &rhs) noexcept { return !rhs.current.ok; }
        friend constexpr bool operator!=(sentinel, const iterator &rhs) noexcept
        {
            return !operator==(sentinel{}, rhs);
        }
#endif
    private:
        void advance()
        {
            Q_ASSERT(current.ok);
            current = tokenizer->next(current.state);
        }
    };
    using const_iterator = iterator;

    using size_type = std::size_t;
    using difference_type = typename iterator::difference_type;
    using value_type = typename iterator::value_type;
    using pointer = typename iterator::pointer;
    using const_pointer = pointer;
    using reference = typename iterator::reference;
    using const_reference = reference;

    Q_REQUIRED_RESULT iterator begin() const noexcept { return iterator{*this}; }
    Q_REQUIRED_RESULT iterator cbegin() const noexcept { return begin(); }
    template<bool = std::is_same<iterator, sentinel>::value> // ODR protection
    Q_REQUIRED_RESULT constexpr sentinel end() const noexcept
    {
        return {};
    }
    template<bool = std::is_same<iterator, sentinel>::value> // ODR protection
    Q_REQUIRED_RESULT constexpr sentinel cend() const noexcept
    {
        return {};
    }

private:
    Haystack m_haystack;
    Needle m_needle;
};

#include <QtCore/qstringview.h>

namespace KDToolBoxPrivate
{
namespace Tok
{

Q_DECL_CONSTEXPR qsizetype size(QChar) noexcept
{
    return 1;
}
template<typename String>
constexpr qsizetype size(const String &s) noexcept
{
    return static_cast<qsizetype>(s.size());
}

template<typename String>
struct ViewForImpl
{
};
template<>
struct ViewForImpl<QStringView>
{
    using type = QStringView;
};
template<>
struct ViewForImpl<QLatin1String>
{
    using type = QLatin1String;
};
template<>
struct ViewForImpl<QChar>
{
    using type = QChar;
};
template<>
struct ViewForImpl<QString> : ViewForImpl<QStringView>
{
};
template<>
struct ViewForImpl<QStringRef> : ViewForImpl<QStringView>
{
};
template<>
struct ViewForImpl<QLatin1Char> : ViewForImpl<QChar>
{
};
template<>
struct ViewForImpl<char16_t> : ViewForImpl<QChar>
{
};
template<>
struct ViewForImpl<char16_t *> : ViewForImpl<QStringView>
{
};
template<>
struct ViewForImpl<const char16_t *> : ViewForImpl<QStringView>
{
};
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
template<typename LHS, typename RHS>
struct ViewForImpl<QStringBuilder<LHS, RHS>> : ViewForImpl<typename QStringBuilder<LHS, RHS>::ConvertTo>
{
};
#endif
template<typename Char, typename... Args>
struct ViewForImpl<std::basic_string<Char, Args...>> : ViewForImpl<Char *>
{
};
#ifdef __cpp_lib_string_view
template<typename Char, typename... Args>
struct ViewForImpl<std::basic_string_view<Char, Args...>> : ViewForImpl<Char *>
{
};
#endif

// This metafunction maps a StringLike to a View (currently, QChar,
// QStringView, QLatin1String). This is what QStringTokenizerBase
// operates on. QStringTokenizer adds pinning to keep rvalues alive
// for the duration of the algorithm.
template<typename String>
using ViewFor = typename ViewForImpl<typename std::decay<String>::type>::type;

// Pinning:
// rvalues of owning string types need to be moved into QStringTokenizer
// to keep them alive for the lifetime of the tokenizer. For lvalues, we
// assume the user takes care of that.

// default: don't pin anything (characters are pinned implicitly)
template<typename String>
struct PinForImpl
{
    using type = ViewFor<String>;
};

// rvalue QString -> QString
template<>
struct PinForImpl<QString>
{
    using type = QString;
};

// rvalue std::basic_string -> basic_string
template<typename Char, typename... Args>
struct PinForImpl<std::basic_string<Char, Args...>>
{
    using type = std::basic_string<Char, Args...>;
};

// rvalue QStringBuilder -> pin as the nested ConvertTo type
template<typename LHS, typename RHS>
struct PinForImpl<QStringBuilder<LHS, RHS>> : PinForImpl<typename QStringBuilder<LHS, RHS>::ConvertTo>
{
};

template<typename StringLike>
using PinFor = typename PinForImpl<typename std::remove_cv<StringLike>::type>::type;

template<typename T>
struct is_owning_string_type : std::false_type
{
};
template<>
struct is_owning_string_type<QString> : std::true_type
{
};
template<typename... Args>
struct is_owning_string_type<std::basic_string<Args...>> : std::true_type
{
};

// unpinned
template<typename T, bool pinned = is_owning_string_type<T>::value>
struct Pinning
{
    // this is the storage for non-pinned types - no storage
    constexpr Pinning(const T &) noexcept {}
    // Since we don't store something, the view() method needs to be
    // given something it can return.
    constexpr T view(T t) const noexcept { return t; }
};

// pinned
template<typename T>
struct Pinning<T, true>
{
    T m_string;
    // specialisation for owning string types (QString, std::u16string):
    // stores the string:
    constexpr Pinning(T &&s) noexcept
        : m_string{std::move(s)}
    {
    }
    // ... and thus view() uses that instead of the argument passed in:
    constexpr QStringView view(const T &) const noexcept { return m_string; }
};

// NeedlePinning and HaystackPinning are there to distinguish them as
// base classes of QStringTokenizer. We use inheritance to reap the
// empty base class optimization.
template<typename T>
struct NeedlePinning : Pinning<T>
{
    using Pinning<T>::Pinning;
    template<typename Arg>
    constexpr auto needleView(Arg &&a) const noexcept -> decltype(this->view(std::forward<Arg>(a)))
    {
        return this->view(std::forward<Arg>(a));
    }
};

template<typename T>
struct HaystackPinning : Pinning<T>
{
    using Pinning<T>::Pinning;
    template<typename Arg>
    constexpr auto haystackView(Arg &&a) const noexcept -> decltype(this->view(std::forward<Arg>(a)))
    {
        return this->view(std::forward<Arg>(a));
    }
};

// The Base of a QStringTokenizer is QStringTokenizerBase for the views
// corresponding to the Haystack and Needle template arguments
//
// ie. QStringTokenizer<QString, QString>
//       : QStringTokenizerBase<QStringView, QStringView> (+ pinning)
template<typename Haystack, typename Needle>
using TokenizerBase = QStringTokenizerBase<ViewFor<Haystack>, ViewFor<Needle>>;
} // namespace Tok
} // namespace KDToolBoxPrivate

template<typename Haystack, typename Needle>
class QStringTokenizer : private KDToolBoxPrivate::Tok::HaystackPinning<Haystack>,
                         private KDToolBoxPrivate::Tok::NeedlePinning<Needle>,
                         public KDToolBoxPrivate::Tok::TokenizerBase<Haystack, Needle>
{
    using HPin = KDToolBoxPrivate::Tok::HaystackPinning<Haystack>;
    using NPin = KDToolBoxPrivate::Tok::NeedlePinning<Needle>;
    using Base = KDToolBoxPrivate::Tok::TokenizerBase<Haystack, Needle>;
    template<typename Container, typename HPin>
    struct if_haystack_not_pinned_impl : std::enable_if<std::is_empty<HPin>::value, bool>
    {
    };
    template<typename Container>
    using if_haystack_not_pinned = typename if_haystack_not_pinned_impl<Container, HPin>::type;
    template<typename Container, typename Iterator = decltype(std::begin(std::declval<Container>()))>
    using if_compatible_container = typename std::enable_if<
        std::is_same<typename Base::value_type, typename std::iterator_traits<Iterator>::value_type>::value,
        bool>::type;

public:
    using value_type = typename Base::value_type;

    constexpr explicit QStringTokenizer(Haystack haystack, Needle needle, Qt::CaseSensitivity cs,
                                        Qt::SplitBehavior sb = Qt::KeepEmptyParts)
        // here, we present the haystack to Pinning<>, for optional storing.
        // If it did store, haystack is moved-from and mustn't be touched
        // any longer, which is why view() for these Pinning<>s ignores the
        // argument.
        : HPin{std::forward<Haystack>(haystack)}
        , NPin{std::forward<Needle>(needle)}
        ,
        // If Pinning<> didn't store, we pass the haystack (ditto needle)
        // to view() again, so it can be copied from there.
        Base{this->haystackView(haystack), this->needleView(needle), sb, cs}
    {
    }
    constexpr explicit QStringTokenizer(Haystack haystack, Needle needle, Qt::SplitBehavior sb = Qt::KeepEmptyParts,
                                        Qt::CaseSensitivity cs = Qt::CaseSensitive)
        : HPin{std::forward<Haystack>(haystack)}
        , NPin{std::forward<Needle>(needle)}
        , Base{this->haystackView(haystack), this->needleView(needle), sb, cs}
    {
    }

    template<typename Container = QVector<value_type>, if_compatible_container<Container> = true>
    Container toContainer(Container &&c = {}) const &
    {
        for (auto e : *this)
            c.push_back(e);
        return std::forward<Container>(c);
    }

    template<typename Container = QVector<value_type>, if_compatible_container<Container> = true,
             if_haystack_not_pinned<Container> = true>
    Container toContainer(Container &&c = {}) const &&
    {
        for (auto e : *this)
            c.push_back(e);
        return std::forward<Container>(c);
    }
};

namespace KDToolBoxPrivate
{
namespace Tok
{
// This meta function just calculated the template arguments for the
// QStringTokenizer (not -Base), based on the actual arguments passed
// to qTokenize() (or the ctor, with CTAD). It basically detects rvalue
// QString and std::basic_string and otherwise decays the arguments to
// the respective view type.
//
// #define works around a C++ restriction: [temp.deduct.guide]/3 seems
// to ask for the simple-template-id following the `->` of a deduction
// guide to be identical to the class name for which we guide deduction.
// In particular, Clang rejects a template alias there, while GCC accepts
// it.
#define Q_TOK_RESULT                                                                                                   \
    QStringTokenizer<KDToolBoxPrivate::Tok::PinFor<Haystack>, KDToolBoxPrivate::Tok::PinFor<Needle>> /*end*/
template<typename Haystack, typename Needle>
using TokenizerResult = Q_TOK_RESULT;
template<typename Haystack, typename Needle>
using is_nothrow_constructible_from = std::is_nothrow_copy_constructible<TokenizerResult<Haystack, Needle>>;
}
} // namespace KDToolBoxPrivate

#ifdef __cpp_deduction_guides
// these tell the compiler how to determine the QStringTokenizer
// template arguments based on the constructor arguments (CTAD):
template<typename Haystack, typename Needle>
QStringTokenizer(Haystack &&, Needle &&)->Q_TOK_RESULT;
template<typename Haystack, typename Needle>
QStringTokenizer(Haystack &&, Needle &&, Qt::SplitBehavior)->Q_TOK_RESULT;
template<typename Haystack, typename Needle>
QStringTokenizer(Haystack &&, Needle &&, Qt::SplitBehavior, Qt::CaseSensitivity)->Q_TOK_RESULT;
template<typename Haystack, typename Needle>
QStringTokenizer(Haystack &&, Needle &&, Qt::CaseSensitivity)->Q_TOK_RESULT;
template<typename Haystack, typename Needle>
QStringTokenizer(Haystack &&, Needle &&, Qt::CaseSensitivity, Qt::SplitBehavior)->Q_TOK_RESULT;
#endif

#undef Q_TOK_RESULT

template<typename Haystack, typename Needle, typename... Flags>
Q_REQUIRED_RESULT constexpr auto qTokenize(Haystack &&h, Needle &&n, Flags... flags)
    -> decltype(KDToolBoxPrivate::Tok::TokenizerResult<Haystack, Needle>{std::forward<Haystack>(h),
                                                                         std::forward<Needle>(n), flags...})
{
    return KDToolBoxPrivate::Tok::TokenizerResult<Haystack, Needle>{std::forward<Haystack>(h), std::forward<Needle>(n),
                                                                    flags...};
}

template<typename Haystack, typename Needle>
auto QStringTokenizerBase<Haystack, Needle>::next(tokenizer_state state) const noexcept -> next_result
{
    while (true)
    {
        if (state.end < 0)
        {
            // already at end:
            return {{}, false, state};
        }
        state.end = m_haystack.indexOf(m_needle, state.start + state.extra, m_cs);
        Haystack result;
        if (state.end >= 0)
        {
            // token separator found => return intermediate element:
            result = m_haystack.mid(state.start, state.end - state.start);
            const auto ns = KDToolBoxPrivate::Tok::size(m_needle);
            state.start = state.end + ns;
            state.extra = (ns == 0 ? 1 : 0);
        }
        else
        {
            // token separator not found => return final element:
            result = m_haystack.mid(state.start);
        }
        if ((m_sb & Qt::SkipEmptyParts) && result.isEmpty())
            continue;
        return {result, true, state};
    }
}

#endif /* QSTRINGTOKENIZER_H */
