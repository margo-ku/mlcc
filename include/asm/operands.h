#pragma once

#include <memory>
#include <string>

class ASMOperand {
public:
    enum class Size {
        Byte1 = 1,
        Byte2 = 2,
        Byte4 = 4,
        Byte8 = 8,
    };

    explicit ASMOperand(Size size);
    virtual ~ASMOperand() = default;

    virtual std::string ToString() const = 0;
    virtual Size GetSize() const;

protected:
    Size size_;
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
    explicit Immediate(long long value);
    std::string ToString() const override;
    long long GetValue() const;

private:
    long long value_;
};

///////////////////////////////////////////////

class Pseudo : public ASMOperand {
public:
    Pseudo(const std::string& name, Size size);
    std::string ToString() const override;

    const std::string& GetName() const;

private:
    std::string name_;
};

///////////////////////////////////////////////

class MemoryOperand : public ASMOperand {
public:
    enum class Mode { Offset, PreIndexed, PostIndexed };

    MemoryOperand(std::shared_ptr<Register> base, int offset, Size size,
                  Mode mode = Mode::Offset);

    std::string ToString() const override;

private:
    std::shared_ptr<Register> base_;
    int offset_;
    Mode mode_;
};

///////////////////////////////////////////////

class DataOperand : public ASMOperand {
public:
    DataOperand(const std::string& name, Size size);
    std::string ToString() const override;

    const std::string& GetName() const;

private:
    std::string name_;
};