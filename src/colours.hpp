#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <unordered_map>

namespace color {

    enum Material
    {
        Background          =   0x263238,
        Foreground          =   0xB0BEC5,
        Text                =   0x607D8B,
        SelectionBackground =   0x546E7A,
        SelectionForeground =   0xFFFFFF,
        Buttons             =   0x2E3C43,
        SecondBackground    =   0x32424A,
        Disabled            =   0x415967,
        Contrast            =   0x1E272C,
        Active              =   0x314549,
        Border              =   0x2A373E,
        Highlight           =   0x425B67,
        Notifications       =   0x1E272C,
        Accent              =   0x009688,
        Excluded            =   0x2E3C43,
        Green               =   0xc3e88d,
        Yellow              =   0xffcb6b,
        Blue                =   0x82aaff,
        Red                 =   0xf07178,
        Purple              =   0xc792ea,
        Orange              =   0xf78c6c,
        Cyan                =   0x89ddff,
        Gray                =   0x546e7a,
        Error               =   0xff5370,
        Comments            =   0x546e7a,
        Variables           =   0xeeffff,
        Links               =   0x80cbc4,
        Functions           =   0x82aaff,
        Keywords            =   0xc792ea,
        Tags                =   0xf07178,
        Strings             =   0xc3e88d,
        Operators           =   0x89ddff,
        Attributes          =   0xffcb6b,
        Numbers             =   0xf78c6c,
        Parameters          =   0xf78c6c,

    };
}

constexpr sf::Color hexColor(uint32_t hex, uint8_t alpha = 0xFF)
{
    return sf::Color
    {
        static_cast<uint8_t>((hex >> 16) & 0xFF),
        static_cast<uint8_t>((hex >> 8) & 0xFF), 
        static_cast<uint8_t>(hex & 0xFF),        
        alpha                                 
    };
}
