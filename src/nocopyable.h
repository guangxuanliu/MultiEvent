#ifndef NOCOPYABLE_H
#define NOCOPYABLE_H

class nocopyable
{
public:
    nocopyable() {}
    ~nocopyable() {}
    
private:
    nocopyable(const nocopyable&);
    nocopyable& operator=(const nocopyable&);
};

#endif
