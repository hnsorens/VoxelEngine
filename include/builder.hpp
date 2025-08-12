#pragma once



template <typename T, typename B>
class Builder
{
public:
    virtual T* build(const B& b) = 0;
};