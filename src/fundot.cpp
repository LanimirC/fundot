#include "../include/fundot.h"

namespace fundot
{
    Fundot::Fundot()
        : _obj(map<Identifier, Object>()), _input_stream(cin), _output_stream(cout)
    {
        _scopeTraceForward(&_obj.value<map<Identifier, Object>>());
        _init();
    }

    Fundot::Fundot(istream &is)
        : _obj(map<Identifier, Object>()), _input_stream(is), _output_stream(cout)
    {
        _scopeTraceForward(&_obj.value<map<Identifier, Object>>());
        _init();
    }

    Fundot::Fundot(istream &is, ostream &os)
        : _obj(map<Identifier, Object>()), _input_stream(is), _output_stream(os)
    {
        _scopeTraceForward(&_obj.value<map<Identifier, Object>>());
        _init();
    }

    Object Fundot::eval(Object &obj)
    {
        if (obj.holds<list<Object>>())
        {
            return _evalList(obj);
        }
        else if (obj.holds<Identifier>())
        {
            return _evalIdentifier(obj);
        }
        else if (obj.holds<pair<Identifier, Object>>())
        {
            return _evalPair(obj);
        }
        else if (obj.holds<vector<Object>>())
        {
            return _evalVector(obj);
        }
        return obj;
    }

    void Fundot::repl()
    {
        Object obj;
        for (size_t i = 0;; ++i)
        {
            _output_stream << "Fundot> ";
            _input_stream >> obj;
            obj = eval(obj);
            if (obj.type() != typeid(nullptr))
            {
                _output_stream << obj << endl;
            }
        }
    }

    vector<map<Identifier, Object> *>::reverse_iterator Fundot::_inWhichScope(const Identifier &id)
    {
        vector<map<Identifier, Object> *>::reverse_iterator it = _scopes.rbegin();
        while (it != _scopes.rend())
        {
            if ((*it)->count(id) > 0)
            {
                return it;
            }
            ++it;
        }
        return _scopes.rend();
    }

    void Fundot::_scopeTraceForward(map<Identifier, Object> *current_scope)
    {
        _local_scope = current_scope;
        _scopes.push_back(_local_scope);
    }

    void Fundot::_scopeTraceBackward(map<Identifier, Object> *previous_scope)
    {
        _local_scope = previous_scope;
        vector<map<Identifier, Object> *>::reverse_iterator it = _scopes.rbegin();
        while (it != _scopes.rend())
        {
            if (*it == previous_scope)
            {
                break;
            }
            _scopes.pop_back();
            ++it;
        }
    }

    Object Fundot::_evalList(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        if (obj_lst.front().holds<Identifier>())
        {
            Identifier &id = obj_lst.front().value<Identifier>();
            if (_inWhichScope(id) != _scopes.rend())
            {
                if (id == "quote")
                {
                    return *(++obj_lst.begin());
                }
                else if (id == "if")
                {
                    return _if(obj);
                }
                else if (id == "while")
                {
                    return _while(obj);
                }
                for (list<Object>::iterator it = ++obj_lst.begin(); it != obj_lst.end(); ++it)
                {
                    *it = eval(*it);
                }
                if (id == "exit")
                {
                    exit(0);
                }
                else if (id == "return")
                {
                    return *(++obj_lst.begin());
                }
                else if (id == "global")
                {
                    return _obj;
                }
                else if (id == "print")
                {
                    return _print(obj);
                }
                else if (id == "+")
                {
                    return _add(obj);
                }
                else if (id == "-")
                {
                    return _sub(obj);
                }
                else if (id == "*")
                {
                    return _mul(obj);
                }
                else if (id == "/")
                {
                    return _div(obj);
                }
                else if (id == "%")
                {
                    return _mod(obj);
                }
                else if (id == "<")
                {
                    return _lessThan(obj);
                }
                else if (id == "==")
                {
                    return _equalTo(obj);
                }
                else if (id == "&&")
                {
                    return _and(obj);
                }
                else if ((*_local_scope)[id].holds<map<Identifier, Object>>())
                {
                    map<Identifier, Object> *previous_scope = _local_scope;
                    _scopeTraceForward(&(*_local_scope)[id].value<map<Identifier, Object>>());
                    Object to_return;
                    if (_local_scope->count(Identifier("type")) > 0 && (*_local_scope)[Identifier("type")].holds<Identifier>())
                    {
                        if ((*_local_scope)[Identifier("type")].value<Identifier>() == "function")
                        {
                            if ((_local_scope->count(Identifier("params")) > 0 && (*_local_scope)[Identifier("params")].holds<vector<Object>>()))
                            {
                                to_return = _evalFunction(obj);
                            }
                        }
                    }
                    _scopeTraceBackward(previous_scope);
                    return to_return;
                }
            }
        }
        return obj;
    }

    Object Fundot::_evalIdentifier(Object &obj)
    {
        Identifier &id = obj.value<Identifier>();
        size_t dot_pos = id.str().find('.');
        map<Identifier, Object> *previous_scope = _local_scope;
        Object to_return;
        if (dot_pos != string::npos)
        {
            if (_local_scope->count(id.str().substr(0, dot_pos)) > 0)
            {
                if ((*_local_scope)[id.str().substr(0, dot_pos)].holds<map<Identifier, Object>>())
                {
                    Object to_eval = Identifier(id.str().substr(dot_pos + 1, id.str().length() - dot_pos - 1));
                    _scopeTraceForward(&(*_local_scope)[id.str().substr(0, dot_pos)].value<map<Identifier, Object>>());
                    to_return = _evalIdentifier(to_eval);
                    _scopeTraceBackward(previous_scope);
                    return to_return;
                }
            }
        }
        if (_local_scope->count(id) > 0)
        {
            return eval((*_local_scope)[id]);
        }
        return obj;
    }

    Object Fundot::_evalPair(Object &obj)
    {
        pair<Identifier, Object> &obj_pair = obj.value<pair<Identifier, Object>>();
        obj_pair.second = eval(obj_pair.second);
        Identifier &id = obj_pair.first;
        size_t dot_pos = id.str().find('.');
        map<Identifier, Object> *previous_scope = _local_scope;
        while (dot_pos != string::npos)
        {
            if (_local_scope->count(id.str().substr(0, dot_pos)) > 0)
            {
                if ((*_local_scope)[id.str().substr(0, dot_pos)].holds<map<Identifier, Object>>())
                {
                    _scopeTraceForward(&(*_local_scope)[id.str().substr(0, dot_pos)].value<map<Identifier, Object>>());
                    id = id.str().substr(dot_pos + 1, id.str().length() - dot_pos - 1);
                    dot_pos = id.str().find('.');
                    if (dot_pos == string::npos)
                    {
                        break;
                    }
                }
            }
        }
        (*_local_scope)[obj_pair.first] = obj_pair.second;
        _scopeTraceBackward(previous_scope);
        return obj;
    }

    Object Fundot::_evalVector(Object &obj)
    {
        vector<Object> &obj_vct = obj.value<vector<Object>>();
        for (size_t i = 0; i < obj_vct.size(); ++i)
        {
            if (i == obj_vct.size() - 1)
            {
                if (obj_vct[i].holds<list<Object>>())
                {
                    Object front = obj_vct[i].value<list<Object>>().front();
                    if (front.holds<Identifier>() && front.value<Identifier>() == "return")
                    {
                        return eval(obj_vct[i]);
                    }
                }
            }
            obj_vct[i] = eval(obj_vct[i]);
        }
        return obj_vct;
    }

    Object Fundot::_evalFunction(Object &obj)
    {
        Object to_return;
        list<Object> &obj_lst = obj.value<list<Object>>();
        vector<Object> &params = (*_local_scope)[Identifier("params")].value<vector<Object>>();
        map<Identifier, Object> param_map;
        list<Object>::iterator it = ++obj_lst.begin();
        for (size_t i = 0; i < params.size(); ++i)
        {
            if (params[i].holds<Identifier>())
            {
                param_map[params[i].value<Identifier>()] = *it++;
            }
        }
        if (_local_scope->count(Identifier("body")) > 0 && (*_local_scope)[Identifier("body")].holds<list<Object>>())
        {
            list<Object> &body = (*_local_scope)[Identifier("body")].value<list<Object>>();
            for (list<Object>::iterator it = body.begin(); it != body.end(); ++it)
            {
                if (it->holds<Identifier>() && param_map.count(it->value<Identifier>()) > 0)
                {
                    *it = param_map[it->value<Identifier>()];
                }
            }
            Object to_eval = body;
            to_return = eval(to_eval);
        }
        return to_return;
    }

    void Fundot::_init()
    {
        static const map<Identifier, Object> fun_defs = {
            {Identifier("type"), Identifier("built-in-function")}};
        static const vector<string> fun_ids = {"quote", "if", "while", "exit", "return", "global", "print",
                                               "+", "-", "*", "/", "%", "<", "==", "&&"};
        for (size_t i = 0; i < fun_ids.size(); ++i)
        {
            _obj[fun_ids[i]] = fun_defs;
        }
    }

    Object Fundot::_if(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        while (it != obj_lst.end())
        {
            Object cond_eval = eval(*it++);
            if (cond_eval.holds<bool>() && cond_eval.value<bool>() == true)
            {
                return eval(*it);
            }
            ++it;
            if (it->holds<Identifier>() && it->value<Identifier>() == "else")
            {
                ++it;
                if ((it->holds<Identifier>() && it->value<Identifier>() == "if") == false)
                {
                    return eval(*it);
                }
            }
            ++it;
        }
        return obj;
    }

    Object Fundot::_while(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        Object condition = *it++;
        Object cond_copy = condition;
        Object cond_eval = eval(condition);
        Object expression = *it;
        Object expr_copy = expression;
        while (cond_eval.holds<bool>() && cond_eval.value<bool>() == true)
        {
            eval(expression);
            expression = expr_copy;
            condition = cond_copy;
            cond_eval = eval(condition);
        }
        return obj;
    }

    Object Fundot::_print(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        Object to_print = *(++obj_lst.begin());
        if (to_print.holds<string>())
        {
            _output_stream << to_print.value<string>();
        }
        else
        {
            _output_stream << to_print;
        }
        _output_stream << endl;
        return obj;
    }

    Object Fundot::_add(Object &obj)
    {
        double result = 0;
        list<Object> &obj_lst = obj.value<list<Object>>();
        for (list<Object>::iterator it = ++obj_lst.begin(); it != obj_lst.end(); ++it)
        {
            if (it->holds<int>())
            {
                result += it->value<int>();
            }
            else if (it->holds<double>())
            {
                result += it->value<double>();
            }
            else
            {
                // error handling
            }
        }
        return Object(result);
    }

    Object Fundot::_sub(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        double result = 0;
        list<Object>::iterator it = ++obj_lst.begin();
        if (it->holds<int>())
        {
            result = it->value<int>();
            ++it;
        }
        else if (it->holds<double>())
        {
            result = it->value<double>();
            ++it;
        }
        for (; it != obj_lst.end(); ++it)
        {
            if (it->holds<int>())
            {
                result -= it->value<int>();
            }
            else if (it->holds<double>())
            {
                result -= it->value<double>();
            }
            else
            {
                // error handling
            }
        }
        return Object(result);
    }

    Object Fundot::_mul(Object &obj)
    {
        double result = 1;
        list<Object> &obj_lst = obj.value<list<Object>>();
        for (list<Object>::iterator it = ++obj_lst.begin(); it != obj_lst.end(); ++it)
        {
            if (it->holds<int>())
            {
                result *= it->value<int>();
            }
            else if (it->holds<double>())
            {
                result *= it->value<double>();
            }
            else
            {
                // error handling
            }
        }
        return Object(result);
    }

    Object Fundot::_div(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        double result = 0;
        list<Object>::iterator it = ++obj_lst.begin();
        if (it->holds<int>())
        {
            result = it->value<int>();
            ++it;
        }
        else if (it->holds<double>())
        {
            result = it->value<double>();
            ++it;
        }
        for (; it != obj_lst.end(); ++it)
        {
            if (it->holds<int>())
            {
                result /= it->value<int>();
            }
            else if (it->holds<double>())
            {
                result /= it->value<double>();
            }
            else
            {
                // error handling
            }
        }
        return Object(result);
    }

    Object Fundot::_mod(Object &obj)
    {
        int result = 0;
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        if (it->holds<int>())
        {
            result = it->value<int>();
            ++it;
        }
        if (it->holds<int>())
        {
            result %= it->value<int>();
        }
        return Object(result);
    }

    Object Fundot::_lessThan(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        double first = 0;
        if (it->holds<int>())
        {
            first = it->value<int>();
        }
        else if (it->holds<double>())
        {
            first = it->value<double>();
        }
        ++it;
        double second = 0;
        if (it->holds<int>())
        {
            second = it->value<int>();
        }
        else if (it->holds<double>())
        {
            second = it->value<double>();
        }
        bool result = first < second;
        if (obj_lst.size() == 3)
        {
            return result;
        }
        list<Object> rest(it, obj_lst.end());
        rest.push_front(Identifier("<"));
        Object to_recurse(rest);
        Object recurse_result = _lessThan(to_recurse);
        if (recurse_result.holds<bool>())
        {
            return result && recurse_result.value<bool>();
        }
        return false;
    }

    Object Fundot::_equalTo(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        double first = 0;
        if (it->holds<int>())
        {
            first = it->value<int>();
        }
        else if (it->holds<double>())
        {
            first = it->value<double>();
        }
        ++it;
        double second = 0;
        if (it->holds<int>())
        {
            second = it->value<int>();
        }
        else if (it->holds<double>())
        {
            second = it->value<double>();
        }
        bool result = first == second;
        if (obj_lst.size() == 3)
        {
            return result;
        }
        list<Object> rest(it, obj_lst.end());
        rest.push_front(Identifier("=="));
        Object to_recurse(rest);
        Object recurse_result = _equalTo(to_recurse);
        if (recurse_result.holds<bool>())
        {
            return result && recurse_result.value<bool>();
        }
        return false;
    }

    Object Fundot::_and(Object &obj)
    {
        list<Object> &obj_lst = obj.value<list<Object>>();
        list<Object>::iterator it = ++obj_lst.begin();
        bool first = false;
        if (it->holds<bool>())
        {
            first = it->value<bool>();
        }
        ++it;
        bool second = false;
        if (it->holds<bool>())
        {
            second = it->value<bool>();
        }
        bool result = first && second;
        if (obj_lst.size() == 3)
        {
            return result;
        }
        list<Object> rest(it, obj_lst.end());
        rest.push_front(Identifier("&&"));
        Object to_recurse(rest);
        Object recurse_result = _and(to_recurse);
        if (recurse_result.holds<bool>())
        {
            return result && recurse_result.value<bool>();
        }
        return false;
    }
} // namespace fundot