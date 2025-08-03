#pragma once

#include <memory>
#include <string>

class ASMOperand {
public:
    virtual std::string ToString() const = 0;
};

///////////////////////////////////////////////

class Register : public ASMOperand {
public:
    explicit Register(std::string name);
    std::string ToString() const override;

private:
    std::string name_;
};

///////////////////////////////////////////////

class Immediate : public ASMOperand {
public:
    explicit Immediate(int value);
    std::string ToString() const override;

private:
    int value_;
};

///////////////////////////////////////////////

class Pseudo : public ASMOperand {
public:
    explicit Pseudo(const std::string& name);
    std::string ToString() const override;

    const std::string& GetName() const;

private:
    std::string name_;
};

///////////////////////////////////////////////

class MemoryOperand : public ASMOperand {
public:
    enum class Mode { Offset, PreIndexed, PostIndexed };

    MemoryOperand(std::shared_ptr<Register> base, int offset, Mode mode = Mode::Offset);

    std::string ToString() const override;

private:
    std::shared_ptr<Register> base_;
    int offset_;
    Mode mode_;
};
