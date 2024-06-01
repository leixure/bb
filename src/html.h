/**
 * @file html.h
 * @brief IO manipulators for inserting HTML tags into an ostream.
 */

#pragma once

#include <functional>
#include <iostream>

/**
 * @brief The namespace for this project, a.k.a. [B]oring [B]ooking.
 */
namespace bb {

/**
 * @brief Make the lambda form of IO manipulators work with ostream using the "<<" operator.
 */
template<typename T>
std::ostream& operator<<(std::ostream& out, std::function<T> t)
{
    return t(out);
}

/**
 * @brief The namespace for HTML IO manipulators.
 */
namespace html {

using ostream = std::ostream;
using tagger = std::function<ostream&(ostream&)>;

/**
 * @internal
 * insert all values to the output stream `out`, like the echo command
 */
template<typename T>
void do_echo_(ostream& out, T t)
{
    out << t;
}

/**
 * @internal
 * insert all values to the output stream `out`, like the echo command
 */
template<typename T, typename... Tail>
void do_echo_(ostream& out, T t, Tail... tail)
{
    out << t;
    do_echo_(out, tail...);
}

/**
 * @brief an IO manipulator for echoing arbitrary arguments to an ostream.
 * @param t the first argument to append to the ostream
 * @param tail other arguments to append to the ostream
 * @return a lambda that appends all the arguments specified by @a t and @a tail
 */
template<typename T, typename... Tail>
tagger echo(T t, Tail... tail)
{
    return [=](ostream& out) -> ostream& {
        do_echo_(out, t, tail...);
        return out;
    };
}

/**
 * @brief an IO manipulator for echoing an HTML tag attribute to an ostream.
 * @param name name of the attribute
 * @param value value of the attribute
 * @return a lambda that appends the attribute to an ostream in the form of ' ${name}="${value}"'
 */
template<typename T, typename U>
tagger attr(T name, U value)
{
    return echo(' ', name, "=\"", value, '"');
}

/**
 * @brief an IO manipulator for echoing an HTML tag to an ostream.
 * @param name name of the HTML tag
 * @param attributes attributes of the tag
 * @param content content of the tag
 * @return a lambda that appends the HTML tag to an ostream in the form of
 *         '<${name}${attributes}>${content}</${name}>'
 */
template<typename T, typename U, typename V>
tagger tag(T name, U attributes, V content)
{
    return echo('<', name, attributes, '>', content, "</", name, '>');
}

/**
 * @brief an IO manipulator for echoing an HTML tag to an ostream, without attributes.
 * @param name name of the HTML tag
 * @param content content of the tag
 * @return a lambda that appends the HTML tag to an ostream in the form of
 *         '<${name}>${content}</${name}>'
 */
template<typename T, typename U>
tagger tag(T name, U content)
{
    return tag(name, "", content);
}

/**
 * @brief an IO manipulator for echoing an anchor tag to an ostream.
 * @param href value of the href attribute
 * @param content content of the tag
 * @return a lambda that appends the anchor tag to an ostream in the form of
 *         '<a href="${href}">${content}</a>'
 */
template<typename T, typename U>
tagger a(T href, U content)
{
    return tag("a", attr("href", href), content);
}

/**
 * @brief an IO manipulator for echoing an li tag to an ostream.
 * @param selected whether this item is selected
 * @param content content of the tag
 * @return a lambda that appends the li tag to an ostream in the form of
 *         '<li>${content}</li>' or '<li class="selected">${content} if selected is true
 */
template<typename T>
tagger li(bool selected, T content)
{
    return selected ?
        tag("li", attr("class", "selected"), content) :
        tag("li", content);
}

}   // namespace tag
}   // namespace bb
