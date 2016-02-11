#pragma once
template <typename Fn>
class Debouncer
{
public:
    Debouncer(Fn buttonFn) : _counter(0), _didPush(false), _buttonFn(buttonFn)
    {
        _counter = 0;
        _didPush = false;
    }

    bool operator()()
    {
        if(_buttonFn())
        {
            ++_counter;
            if(_counter >= upThresh)
            {
                _didPush = true;
                _counter = upThresh;
            }
        }
        else
        {
            if(_counter > 0)
            {
                _counter--;
                if(_counter <= downThresh)
                {
                    if(_didPush)
                    {
                        _didPush = false;
                        _counter = 0;
                        return true;
                    }
                }
            }
        }
        
        return false;
    }
    
private:
    int _counter = 0;
    bool _didPush = false;
    const int upThresh = 4;
    const int downThresh = 2;
    Fn _buttonFn;
};

template <typename Fn>
auto makeDebouncer(Fn fn) -> Debouncer<Fn>
{
    return Debouncer<Fn>(fn);
}

